const RingBuffer = @import("../ring_buffer.zig").RingBuffer;
const common = @import("common.zig");
const helpers = @import("../helpers.zig");
const std = @import("std");
const c = @cImport({
    @cInclude("soundio/soundio.h");
});

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const AudError = common.AudError;
const AudioFormat = common.AudioFormat;
const ConnectContext = common.ConnectContext;
const Device = common.Device;
const Direction = common.Direction;
const PrioritizedSampleRates = common.PrioritizedSampleRates;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const mem = std.mem;
const print = std.debug.print;
const talloc = std.testing.allocator;
const time = std.time;

// Use the SoundIo enum for formats because these enum values
// will not be transferrable across different backends.
pub const PrioritizedFormats = [_]c_int{
    c.SoundIoFormatS8,
    c.SoundIoFormatU8,
    c.SoundIoFormatS16LE,
    c.SoundIoFormatS16BE,
    c.SoundIoFormatU16LE,
    c.SoundIoFormatU16BE,
    c.SoundIoFormatS24LE,
    c.SoundIoFormatS24BE,
    c.SoundIoFormatU24LE,
    c.SoundIoFormatU24BE,
    c.SoundIoFormatS32LE,
    c.SoundIoFormatS32BE,
    c.SoundIoFormatU32LE,
    c.SoundIoFormatU32BE,
    c.SoundIoFormatFloat32LE,
    c.SoundIoFormatFloat32BE,
};

pub const DefaultLatencyMs: c_int = 0;
pub const DefaultBufferMs: c_int = 4000;

fn failed(status: c_int) bool {
    return status != 0;
}

fn index_failed(status: c_int) bool {
    return status < 0;
}

// TODO(lbayes): Work out backend_disconnect scenario outlined here:
// http://libsound.io/doc-2.0.0/backend_disconnect_recover_8c-example.html
//
// TODO(lbayes): Decide if we want to work with 'raw' devices.
//
// According to SoundIO docs found here:
//
// http://libsound.io/doc-2.0.0/structSoundIoDevice.html#afb73b9bd13e98a14a187567f508acebe
//
// And example here:
//
// http://libsound.io/doc-2.0.0/sio_record_8c-example.html
//
// Device.is_raw indicates that, "you are directly opening the
// hardware device and not going through a proxy such as dmix, PulseAudio, or
// JACK.
//
// When you open a raw device, other applications on the computer are not able
// to simultaneously access the device. Raw devices do not perform automatic
// resampling and thus tend to have fewer formats available."

const NativeGetDevice = fn ([*c]c.struct_SoundIo, c_int) callconv(.C) [*c]c.struct_SoundIoDevice;

pub const Devices = struct {
    allocator: *Allocator,
    soundio: *c.SoundIo,
    aud_devices: ArrayList(*Device) = undefined,
    sio_devices: ArrayList(*c.SoundIoDevice) = undefined,
    last_id: u8 = 0,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);

        const version = c.soundio_version_string();
        print("soundio version: {s}\n", .{version});

        const soundio = c.soundio_create();
        if (soundio == null) {
            print("soundio failed to allocate\n", .{});
            return error.Fail;
        }
        print("soundio successfully created context\n", .{});

        {
            // const value = @intToEnum(c.SoundIoBackend, c.SoundIoBackendPulseAudio);
            // const status = c.soundio_connect_backend(soundio, value);
            const status = c.soundio_connect(soundio);

            if (failed(status)) {
                print("------------------------------------\n", .{});
                print("soundio failed to connect to backend\n", .{});
                // Free the soundio object before exiting
                c.soundio_destroy(soundio);
                return error.Fail;
            }
            print("soundio connected with backend: {}\n", .{soundio.*.current_backend});
        }

        c.soundio_flush_events(soundio);
        print("soundio flushed events\n", .{});

        instance.* = Devices{
            .allocator = a,
            .soundio = soundio,
            .sio_devices = ArrayList(*c.SoundIoDevice).init(a),
            .aud_devices = ArrayList(*Device).init(a),
        };

        return instance;
    }

    pub fn deinit(self: *Devices) void {
        self.sio_devices.deinit();
        // Free each device that was allocated for the devices list.
        for (self.aud_devices.items) |dev| {
            self.allocator.destroy(dev);
        }
        self.aud_devices.deinit();
        c.soundio_destroy(self.soundio);
        self.allocator.destroy(self);
    }

    fn getNativeDevices(self: *Devices, buffer: []Device, count: c_int, get_by_index: NativeGetDevice) ![]Device {
        var index: usize = 0;

        while (index < count) : (index += 1) {
            const sio_device = get_by_index(self.soundio, @intCast(c_int, index));
            const device = try self.sioDeviceToAudDevice(sio_device, index);
            buffer[index] = device.*;
        }
        return buffer[0..index];
    }

    fn sioDeviceToAudDevice(self: *Devices, sio_device: *c.SoundIoDevice, c_index: usize) !*Device {
        const id = mem.sliceTo(sio_device.id, 0);
        const native_name = mem.sliceTo(sio_device.name, 0);

        const as_int = @ptrToInt(sio_device);
        const after = @intToPtr(*c.struct_SoundIoDevice, as_int);

        print("-------------------\n", .{});
        print("soundio device id: {s}\n", .{id});
        print("soundio device ptr: {d}\n", .{as_int});
        // print("SIO DEV BEFORE {*}\n", .{sio_device});
        // print("SIO PTR INT: {} {d}\n", .{ @TypeOf(as_int), as_int });
        // print("SIO PTR AFTER: {*}\n", .{after});
        print("-------------------\n", .{});

        var device = try self.allocator.create(Device);

        device.* = Device{
            .id = id,
            .name = native_name,
            .platform_device = as_int,
        };

        // Default Device.direction is Render, update if we have a Capture device.
        if (sio_device.aim == @intToEnum(c.SoundIoDeviceAim, c.SoundIoDeviceAimInput)) {
            device.direction = Direction.Capture;
        }

        // print("ID: {s}\n", .{device.id});
        // print("NAME: {d}: {s}\n", .{ mem.len(device.name), device.name });
        try self.aud_devices.append(device);

        return device;
    }

    pub fn info(self: *Devices) []const u8 {
        return "soundio";
    }

    pub fn getDefaultDevice(self: *Devices, direction: Direction) !*Device {
        if (direction == Direction.Capture) {
            return try self.getDefaultCaptureDevice();
        } else {
            return try self.getDefaultRenderDevice();
        }
    }

    pub fn getCaptureDeviceAt(self: *Devices, index: usize) !*Device {
        const sio_device = try c.soundio_get_input_device(self.soundio, @intCast(c_int, index)) orelse error.Fail;
        return try self.sioDeviceToAudDevice(sio_device, index);
    }

    pub fn getDefaultCaptureDevice(self: *Devices) !*Device {
        const index = c.soundio_default_input_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_input_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        return try self.getCaptureDeviceAt(@intCast(usize, index));
    }

    pub fn getRenderDeviceAt(self: *Devices, index: usize) !*Device {
        const sio_device = try c.soundio_get_output_device(self.soundio, @intCast(c_int, index)) orelse error.Fail;
        return try self.sioDeviceToAudDevice(sio_device, index);
    }

    pub fn getDefaultRenderDevice(self: *Devices) !*Device {
        const index = c.soundio_default_output_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_output_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        return try self.getRenderDeviceAt(@intCast(usize, index));
    }

    pub fn getDevices(self: *Devices, buffer: []Device, direction: Direction) ![]Device {
        if (direction == Direction.Capture) return self.getCaptureDevices(buffer) else return self.getRenderDevices(buffer);
    }

    pub fn getCaptureDevices(self: *Devices, buffer: []Device) ![]Device {
        print("CAPTURE DEVICES\n", .{});
        const count = c.soundio_input_device_count(self.soundio);
        return self.getNativeDevices(buffer, count, c.soundio_get_input_device);
    }

    pub fn getRenderDevices(self: *Devices, buffer: []Device) ![]Device {
        print("RENDER DEVICES\n", .{});
        const count = c.soundio_output_device_count(self.soundio);
        return self.getNativeDevices(buffer, count, c.soundio_get_output_device);
    }

    pub fn startCapture(self: *Devices, config: *ConnectContext) !void {
        ////////////////////////////////////////////////////////////////
        // Initialize the render and capture devices
        const capture = config.capture_device;
        const sio_capture = @intToPtr(*c.struct_SoundIoDevice, capture.platform_device);
        defer c.soundio_device_unref(sio_capture);
        print("Soundio capture name: {s}\n", .{sio_capture.name});

        const render = config.render_device;
        const sio_render = @intToPtr(*c.struct_SoundIoDevice, render.platform_device);
        defer c.soundio_device_unref(sio_render);
        print("Soundio render name: {s}\n", .{sio_render.name});

        const latency = DefaultLatencyMs;
        const buffer_latency = DefaultBufferMs;

        ////////////////////////////////////////////////////////////////
        // Configure Channel Layout
        // Find the first supported layout between the capture and render devices.

        var layout = sio_capture.current_layout;

        // c.soundio_device_sort_channel_layouts(sio_capture);
        // c.soundio_device_sort_channel_layouts(sio_render);
        // var layout = c.soundio_best_matching_channel_layout(sio_render.layouts, sio_render.layout_count, sio_capture.layouts, sio_capture.layout_count);
        // if (layout == null) {
        //     return AudError.StreamLayoutFailure;
        // }
        // TODO(lbayes): Figure out how to trace an identifier for layout
        print("Soundio using layout channel_count: {}\n", .{layout.channel_count});

        ////////////////////////////////////////////////////////////////
        // Configure Sample Rate
        // Find the first supported sample rate between the capture and render devices.
        var sample_rate: c_int = undefined;
        for (PrioritizedSampleRates[0..]) |rate| {
            sample_rate = @intCast(c_int, rate);
            if (c.soundio_device_supports_sample_rate(sio_capture, sample_rate) and
                c.soundio_device_supports_sample_rate(sio_render, sample_rate))
            {
                break;
            }
        }
        print("Soundio using sample_rate: {}\n", .{sample_rate});

        ////////////////////////////////////////////////////////////////
        // Configure Format
        var format: c.SoundIoFormat = c.enum_SoundIoFormat.Invalid;
        for (PrioritizedFormats[0..]) |p_format| {
            format = @intToEnum(c.SoundIoFormat, p_format);
            if (format != c.enum_SoundIoFormat.Invalid and
                c.soundio_device_supports_format(sio_capture, format) and
                c.soundio_device_supports_format(sio_render, format))
            {
                break;
            }
        }

        if (format == c.enum_SoundIoFormat.Invalid) {
            return AudError.FormatFailure;
        }
        print("Soundio using format: {}\n", .{format});

        ////////////////////////////////////////////////////////////////
        // Configure In Stream
        var in_stream: *c.struct_SoundIoInStream = c.soundio_instream_create(sio_capture);
        // TODO(lbayes): Figure out if we've got a null in_stream?
        defer c.soundio_instream_destroy(in_stream);

        in_stream.software_latency = latency;
        in_stream.format = format;
        in_stream.sample_rate = sample_rate;
        in_stream.layout = layout;

        in_stream.read_callback = sioCaptureCallback;
        in_stream.error_callback = sioCaptureErrorCallback;
        in_stream.overflow_callback = sioCaptureOverflowCallback;

        in_stream.userdata = config;

        var err = c.soundio_instream_open(in_stream);
        if (err != 0) {
            return AudError.StreamOpenFailure;
        }
        print("Soundio opened instream\n", .{});

        ////////////////////////////////////////////////////////////////
        // Configure Out Stream
        var out_stream: *c.struct_SoundIoOutStream = c.soundio_outstream_create(sio_render);
        // TODO(lbayes): Figure out if we've got a null out_stream?
        // defer c.soundio_outstream_destroy(out_stream);

        out_stream.software_latency = latency;
        out_stream.format = format;
        out_stream.sample_rate = sample_rate;
        out_stream.layout = layout;

        out_stream.write_callback = sioRenderCallback;
        out_stream.error_callback = sioRenderErrorCallback;
        out_stream.underflow_callback = sioRenderUnderflowCallback;

        out_stream.userdata = config;

        err = c.soundio_outstream_open(out_stream);
        if (err != 0) {
            return AudError.StreamOpenFailure;
        }
        print("Soundio opened outstream\n", .{});

        ////////////////////////////////////////////////////////////////
        // Create RingBuffer
        // const capacity: c_int = in_stream.bytes_per_frame * buffer_latency * sample_rate * 2;
        // const c_capacity: c_int = in_stream.bytes_per_frame * buffer_latency * sample_rate * 2;
        // const capacity: usize = @intCast(usize, c_capacity);

        // TODO(lbayes): Fix below. Got an LLVM IR error when attempting to intCast this at runtime.
        const capacity: usize = 716800000;
        print("Soundio ring_buffer capacity bytes: {}\n", .{capacity});
        var buffer = try RingBuffer(u8).init(self.allocator, capacity);
        config.buffer = buffer;

        // var ring_buffer = c.soundio_ring_buffer_create(self.soundio, capacity);
        // Store the ring_buffer pointer on the context
        // config.platform_buffer = @ptrToInt(ring_buffer);

        // var render_buffer = c.soundio_ring_buffer_write_ptr(ring_buffer);
        // TODO(lbayes): Figure if we need to advance the write ptr at start
        // TODO(lbayes): Make the ring_buffer accessible to callbacks
        print("soundio ring_buffer created\n", .{});

        ////////////////////////////////////////////////////////////////
        // Cross the streams!
        c.soundio_flush_events(self.soundio);
        print(">>>>>>>>>>>>>> Soundio render name: {s}\n", .{sio_render.name});

        // TODO(lbayes): THE FOLLOWING LINE SEGFAULTS!
        // err = c.soundio_outstream_start(out_stream);
        // if (err != 0) {
        //     return AudError.StreamStartFailure;
        // }
        // print("Soundio started outstream\n", .{});

        err = c.soundio_instream_start(in_stream);
        if (err != 0) {
            return AudError.StreamStartFailure;
        }

        print("Waiting for stream to close\n", .{});

        while (config.is_active) {
            c.soundio_wait_events(self.soundio);
            time.sleep(40 * time.ns_per_ms);
        }
    }

    fn sioRenderCallback(c_outstream: [*c]c.struct_SoundIoOutStream, frame_count_min: c_int, frame_count_max: c_int) callconv(.C) void {
        print("SIO RENDER CB\n", .{});
        // const outstream: *c.struct_SoundIoOutStream = @as(*c.struct_SoundIoOutStream, c_outstream);
        // const context: *ConnectContext = @ptrCast(*ConnectContext, outstream.userdata);

        // print("soundio render: {} {} {}\n", .{ context.is_active, frame_count_min, frame_count_max });
    }

    fn sioRenderErrorCallback(instream: [*c]c.struct_SoundIoOutStream, err: c_int) callconv(.C) void {
        print("ERROR: soundio render: {}\n", .{err});
    }

    fn sioRenderUnderflowCallback(outstream: [*c]c.struct_SoundIoOutStream) callconv(.C) void {
        print("UNDERFLOW: soundio render\n", .{});
    }

    fn sioCaptureCallback(c_instream: [*c]c.struct_SoundIoInStream, frame_count_min: c_int, frame_count_max: c_int) callconv(.C) void {
        const instream: *c.struct_SoundIoInStream = @as(*c.struct_SoundIoInStream, c_instream);
        const context: *ConnectContext = @ptrCast(*ConnectContext, instream.userdata);
        // const ring_buffer = @intToPtr(*c.struct_SoundIoRingBuffer, context.platform_buffer);

        print("soundio capture: {} {} {}\n", .{ context.is_active, frame_count_min, frame_count_max });
        print("soundio bytes_per_frame: {}\n", .{instream.bytes_per_frame});

        // var frame: c_int = 0;
        // while (frame < frame_count_max) {
        //     print(
        //     frame += 1;
        // }
        // _ = std.c.printf("HELLO WORLD\n");
        // print("SIO READ CB\n", .{});
    }

    fn sioCaptureErrorCallback(instream: [*c]c.struct_SoundIoInStream, err: c_int) callconv(.C) void {
        print("ERROR: soundio capture: {}\n", .{err});
    }

    fn sioCaptureOverflowCallback(instream: [*c]c.struct_SoundIoInStream) callconv(.C) void {
        print("OVERFLOW: soundio capture\n", .{});
    }
};

test "Soundio Devices is instantiable" {
    var api = try Devices.init(talloc);
    defer api.deinit();

    try expectEqualStrings("soundio", api.info());
}

// NOTE(lbayes): This test will only pass on my workstation
// with particular devices connected. It is only useful for development purposes
// test "Soundio getDevices returns list" {
//     var api = try Devices.init(talloc);
//     defer api.deinit();
//
//     var buff: [common.MAX_DEVICE_COUNT]Device = undefined;
//     const devices = try api.getDevices(&buff, Direction.Render);
//
//     for (devices) |dev| {
//         print("DEVI: {s}\n", .{dev.name});
//     }
//     try expectEqual(devices.len, 5);
// }


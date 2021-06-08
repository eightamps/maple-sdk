const std = @import("std");
const common = @import("./common.zig");
const helpers = @import("../helpers.zig");

const c = @cImport({
    @cInclude("soundio/soundio.h");
});

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const Device = common.Device;
const Direction = common.Direction;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const mem = std.mem;
const print = std.debug.print;
const talloc = std.testing.allocator;

fn failed(status: c_int) bool {
    return status != 0;
}

fn index_failed(status: c_int) bool {
    return status < 0;
}

const NativeGetDevice = fn ([*c]c.struct_SoundIo, c_int) callconv(.C) [*c]c.struct_SoundIoDevice;

pub const Devices = struct {
    allocator: *Allocator,
    soundio: *c.SoundIo,
    ring_buffer: *c.SoundIoRingBuffer = undefined,
    aud_devices: ArrayList(*Device) = undefined,
    sio_devices: ArrayList(*c.SoundIoDevice) = undefined,
    last_id: u8 = 0,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);

        const soundio = c.soundio_create();
        if (soundio == null) {
            print("soundio failed to allocate\n", .{});
            return error.Fail;
        }
        print("soundio successfully created context\n", .{});

        {
            const value = @intToEnum(c.SoundIoBackend, c.SoundIoBackendPulseAudio);
            const status = c.soundio_connect_backend(soundio, value);
            if (failed(status)) {
                print("soundio failed to connect to backend\n", .{});
                // Free the soundio object before exiting
                c.soundio_destroy(soundio);
                return error.Fail;
            }
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
            const device = try self.sioDeviceToAudDevice(sio_device);
            buffer[index] = device;
        }
        return buffer[0..index];
    }

    fn sioDeviceToAudDevice(self: *Devices, sio_device: *c.SoundIoDevice) !Device {
        const id = mem.sliceTo(sio_device.id, 0);
        const native_name = mem.sliceTo(sio_device.name, 0);

        var device = try self.allocator.create(Device);

        device.* = Device{
            .id = id,
            .name = native_name,
        };

        // print("ID: {s}\n", .{device.id});
        // print("NAME: {d}: {s}\n", .{ mem.len(device.name), device.name });
        try self.aud_devices.append(device);

        return device.*;
    }

    pub fn info(self: *Devices) []const u8 {
        return "soundio";
    }

    pub fn getDefaultDevice(self: *Devices, direction: Direction) !Device {
        if (direction == Direction.Capture) {
            return self.getDefaultCaptureDevice();
        } else {
            return self.getDefaultRenderDevice();
        }
    }

    pub fn getCaptureDeviceAt(self: *Devices, index: c_int) !Device {
        const sio_device = try c.soundio_get_input_device(self.soundio, index) orelse error.Fail;
        return self.sioDeviceToAudDevice(sio_device);
    }

    pub fn getDefaultCaptureDevice(self: *Devices) !Device {
        const index = c.soundio_default_input_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_input_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        return try self.getCaptureDeviceAt(index);
    }

    pub fn getRenderDeviceAt(self: *Devices, index: c_int) !Device {
        const sio_device = try c.soundio_get_output_device(self.soundio, index) orelse error.Fail;
        return self.sioDeviceToAudDevice(sio_device);
    }

    pub fn getDefaultRenderDevice(self: *Devices) !Device {
        const index = c.soundio_default_output_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_output_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        return try self.getRenderDeviceAt(index);
    }

    pub fn getDevices(self: *Devices, buffer: []Device, direction: Direction) ![]Device {
        if (direction == Direction.Capture) return self.getCaptureDevices(buffer) else return self.getRenderDevices(buffer);
    }

    pub fn getCaptureDevices(self: *Devices, buffer: []Device) ![]Device {
        const count = c.soundio_input_device_count(self.soundio);
        return self.getNativeDevices(buffer, count, c.soundio_get_input_device);
    }

    pub fn getRenderDevices(self: *Devices, buffer: []Device) ![]Device {
        const count = c.soundio_output_device_count(self.soundio);
        return self.getNativeDevices(buffer, count, c.soundio_get_output_device);
    }
};

test "Soundio Devices is instantiable" {
    var api = try Devices.init(talloc);
    defer api.deinit();

    try expectEqualStrings("soundio", api.info());
}

// NOTE(lbayes): This test will only pass on my workstation
// with particular devices connected. It is only useful for development purposes
test "Soundio getDevices returns list" {
    var api = try Devices.init(talloc);
    defer api.deinit();

    var buff: [common.MAX_DEVICE_COUNT]Device = undefined;
    const devices = try api.getDevices(&buff, Direction.Render);

    for (devices) |dev| {
        print("DEVI: {s}\n", .{dev.name});
    }
    try expectEqual(devices.len, 5);
}

test "Soundio getDeviceById" {}

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

    fn sioDeviceToAudDevice(self: *Devices, sio_device: *c.SoundIoDevice) !Device {
        const id = mem.sliceTo(sio_device.id, 0);
        const native_name = mem.sliceTo(sio_device.name, 0);

        var device = try self.allocator.create(Device);

        device.* = Device{
            .id = id,
            .name = native_name,
        };

        print("ID: {s}\n", .{device.id});
        print("NAME: {d}: {s}\n", .{ mem.len(device.name), device.name });
        try self.aud_devices.append(device);

        return device.*;
    }

    pub fn getDefaultCaptureDevice(self: *Devices) !Device {
        return error.Fail;
    }

    pub fn getDefaultRenderDevice(self: *Devices) !Device {
        const index = c.soundio_default_output_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_output_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        print("soundio_default_output_device_index index: {}\n", .{index});
        const sio_device = try self.getRenderDeviceByIndex(index);

        // Store the device for later lookups?
        try self.sio_devices.append(sio_device);

        return self.sioDeviceToAudDevice(sio_device);
    }

    fn getRenderDeviceByIndex(self: *Devices, index: c_int) !*c.SoundIoDevice {
        const sio_device = c.soundio_get_output_device(self.soundio, index);
        if (sio_device == null) {
            print("soundio_get_output_device failed\n", .{});
            return error.Fail;
        }
        print("soundio_get_output_device success\n", .{});
        return sio_device;
    }

    fn getDefaultCaptureDeviceIndex(self: *Devices) !c_int {
        const index = c.soundio_default_input_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_input_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        print("soundio_default_input_device_index: {}\n", .{index});
        return index;
    }

    pub fn getDevices(self: *Devices, buffer: []Device, direction: Direction) ![]Device {
        // const filter = if (direction == Direction.Capture) isCaptureDevice else isRenderDevice;
        // var filters = [_]DeviceFilter{filter};
        // return helpers.filterItems(Device, self.devices, buffer, &filters);
        return error.Fail;
    }

    pub fn getCaptureDevices(self: *Devices, buffer: []Device) ![]Device {
        return self.getDevices(buffer, Direction.Capture);
    }

    pub fn getRenderDevices(self: *Devices, buffer: []Device) ![]Device {
        return self.getDevices(buffer, Direction.Render);
    }

    pub fn getCaptureDeviceAt(self: *Devices, index: u16) *Device {
        var buffer: [MAX_DEVICE_COUNT]Device = undefined;
        var result = try self.getCaptureDevices(&buffer);
        return &result[index];
    }

    pub fn getRenderDeviceAt(self: *Devices, index: u16) *Device {
        var buffer: [MAX_DEVICE_COUNT]Device = undefined;
        var result = try self.getRenderDevices(&buffer);
        return &result[index];
    }
};

test "Soundio Devices is instantiable" {
    var api = try Devices.init(talloc);
    defer api.deinit();

    try expectEqualStrings("soundio", api.info());
}

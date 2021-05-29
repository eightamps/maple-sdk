const std = @import("std");
const common = @import("../aud_common.zig");

const Allocator = std.mem.Allocator;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const heap = std.heap;
const mem = std.mem;
const print = std.debug.print;

const DEFAULT_DEVICE_NAME = "[unknown]";

pub fn info() []const u8 {
    return "TEST";
}

pub const Device = struct {
    direction: common.Direction,
    allocator: *Allocator,
    name: []const u8 = DEFAULT_DEVICE_NAME,

    pub fn deinit(self: *Device) void {
        self.allocator.destroy(self);
    }
};

pub const Devices = struct {
    allocator: *Allocator,
    fake_devices: []const Device,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);
        return instance;
    }

    pub fn deinit(self: *Devices) void {
        print("AudibleApi.deinit called\n", .{});
        soundio_destroy(self.soundio);
        self.allocator.destroy(self);
    }

    pub fn createDevice(self: *Devices, direction: common.Direction) !*Device {
        const device = try self.allocator.create(Device);
        device.* = Device{
            .allocator = self.allocator,
            .direction = direction,
        };
        return device;
    }

    pub fn getCaptureDevice(self: *Devices, matcher: common.Matcher) !*Device {
        return try self.createDevice(common.Direction.Capture);
    }

    pub fn getRenderDeviceByIndex(self: *Devices, index: c_int) !*SoundIoDevice {
        const sio_device = soundio_get_output_device(self.soundio, index);
        if (sio_device == null) {
            print("soundio_get_output_device failed\n", .{});
            return error.Fail;
        }
        print("soundio_get_output_device success\n", .{});
        return sio_device;
    }

    pub fn getDefaultRenderDeviceIndex(self: *Devices) !c_int {
        const index = soundio_default_output_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_output_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        print("soundio_default_output_device_index index: {}\n", .{index});
        return index;
    }

    fn getRenderDeviceCount(self: *Devices) !c_int {
        const count = soundio_output_device_count(self.soundio);
        if (index_failed(count)) {
            print("soundio_output_device_count failed with: {}\n", .{count});
            return error.Fail;
        }
        print("soundio_output_device_count: {}\n", .{count});
        return count;
    }

    fn getDefaultCaptureDeviceIndex(self: *Devices) !c_int {
        const index = soundio_default_input_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_input_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        print("soundio_default_input_device_index: {}\n", .{index});
        return index;
    }

    fn getCaptureDeviceCount(self: *Devices) !c_int {
        const count = soundio_input_device_count(self.soundio);
        if (index_failed(count)) {
            print("soundio_input_device_count failed with: {}\n", .{count});
            return error.Fail;
        }
        print("soundio_input_device_count: {}\n", .{count});
        return count;
    }
};

test "Nix Native.Devices is instantiable" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();
}

test "Nix Native.Devices info" {
    const name = info();
    try expectEqual(name, "LINUX");
}

test "Nix Native.Default device" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();

    const index = try devices.getDefaultRenderDeviceIndex();
    const device = try devices.getRenderDeviceByIndex(index);
}

const std = @import("std");
const common = @import("../aud_common.zig");

const Allocator = std.mem.Allocator;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const heap = std.heap;
const mem = std.mem;
const print = std.debug.print;

const DEFAULT_DEVICE_NAME = "[unknown]";

usingnamespace common;

pub fn info() []const u8 {
    return "TEST";
}

pub const Devices = struct {
    allocator: *Allocator,
    fake_devices: []const Device = undefined,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);
        instance.* = Devices{
            .allocator = a,
        };
        return instance;
    }

    pub fn deinit(self: *Devices) void {
        print("AudibleApi.deinit called\n", .{});
        self.allocator.destroy(self);
    }

    pub fn createDevice(self: *Devices, direction: Direction) !*Device {
        const device = try self.allocator.create(Device);
        device.* = Device{
            .allocator = self.allocator,
            .direction = direction,
        };
        return device;
    }

    pub fn getRenderDeviceByIndex(self: *Devices, index: c_int) !*Device {
        const d = Device{
            .name = "Default Fake Render Device",
            .allocator = self.allocator,
            .direction = Direction.Render,
        };

        // self.fake_devices.append(d);

        return d;
    }
};

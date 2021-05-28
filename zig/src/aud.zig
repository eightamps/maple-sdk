const std = @import("std");
const common = @import("os/aud_common.zig");

const target_file = switch (std.Target.current.os.tag) {
    .windows => "os/win/aud_native.zig",
    .linux => "os/nix/aud_native.zig",
    else => "os/nix/aud_native.zig",
};

const Allocator = std.mem.Allocator;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const heap = std.heap;
const print = std.debug.print;

const native = @import(target_file);
pub usingnamespace common;

pub fn info() []const u8 {
    return native.info();
}

pub const Devices = struct {
    allocator: *Allocator,
    devices: *native.Devices,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);
        const devices = try native.Devices.init(a);

        instance.* = Devices{
            .allocator = a,
            .devices = devices,
        };
        return instance;
    }

    pub fn deinit(self: *Devices) void {
        print("AudibleApi.deinit called\n", .{});
        self.devices.deinit();
        self.allocator.destroy(self);
    }

    pub fn getDevice(self: *Devices, matcher: Matcher) !*native.Device {
        std.debug.print("Linux getDefaultDevice()\n", .{});
        return self.devices.getDevice(matcher);
    }
};

test "Aud.Devices is instantiable" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();
}

test "Aud.Devices info" {
    const name = info();
    try expectEqual(name, "LINUX");
}

test "Aud.Default device" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();

    const device = try devices.getDevice(DefaultCapture);
    defer device.deinit();
}

const std = @import("std");
const common = @import("os/aud_common.zig");

const target_file = switch (std.Target.current.os.tag) {
    .windows => "os/win/aud_native.zig",
    .linux => "os/nix/aud_native.zig",
    else => "fakes/aud_native.zig",
};

const Allocator = std.mem.Allocator;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const heap = std.heap;
const mem = std.mem;
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

    pub fn getDevice(self: *Devices, matcher: common.Matcher) !*native.Device {
        std.debug.print("Linux getDevice()\n", .{});
        switch (matcher.direction) {
            common.Direction.Render => {
                return self.getRenderDevice(matcher);
            },
            common.Direction.Capture => {
                return self.getCaptureDevice(matcher);
            },
        }
    }

    pub fn getRenderDevice(self: *Devices, matcher: common.Matcher) !*native.Device {
        if (matcher.is_default) {
            const index = try self.devices.getDefaultRenderDeviceIndex();
            const sio_device = try self.devices.getRenderDeviceByIndex(index);
            const device_name_cs = @ptrCast([*:0]const u8, sio_device.name);
            const device_name = device_name_cs[0..mem.len(device_name_cs)];
            print(">>>>>>>>>>>>> name: {s}\n", .{device_name});
            var itr = mem.split(matcher.not_matches, "|");
            {
                print("-----------\n", .{});
                var name = itr.next();
                print("device_name: {s}\n", .{device_name});
                while (name != null) : (name = itr.next()) {
                    print("device_name: \"{s}\" vs name: \"{s}\"\n", .{ device_name, name });
                    const unwrapped = name orelse continue;
                    if (mem.indexOf(u8, device_name, unwrapped) != null) {
                        print("DEVICE NAME IS NOT VALID!\n", .{});
                        return error.Fail;
                    }
                }
                print("-----------\n", .{});
            }
            // const count = try self.getRenderDeviceCount();
        }
        return try self.devices.createDevice(common.Direction.Render);
    }

    pub fn getCaptureDevice(self: *Devices, matcher: common.Matcher) !*native.Device {
        return try self.devices.createDevice(common.Direction.Capture);
    }
};

test "aud.Devices is instantiable" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();
}

test "aud.info" {
    const name = info();
    try expectEqual(name, "LINUX");
}

test "aud.Devices.getDevice()" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();

    const device = try devices.getDevice(DefaultCapture);
    defer device.deinit();
}

const std = @import("std");
const common = @import("os/aud_common.zig");
const fake_native = @import("os/fake/aud_native.zig");

const target_file = switch (std.Target.current.os.tag) {
    .windows => "os/win/aud_native.zig",
    .linux => "os/nix/aud_native.zig",
    else => "os/fake/aud_native.zig",
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

pub fn Devices2(comptime T: type, comptime R: type) type {
    return struct {
        allocator: *Allocator,
        impl: *T,

        pub fn init(a: *Allocator) !*Devices2(T, R) {
            const instance = try a.create(Devices2(T, R));
            const impl = try T.init(a);

            instance.* = Devices2(T, R){
                .allocator = a,
                .impl = impl,
            };
            print("Devices2.init called!\n", .{});
            return instance;
        }

        pub fn deinit(self: *Devices2(T, R)) void {
            self.impl.deinit();
            self.allocator.destroy(self);
        }
    };
}

test "Devices2 with current platform" {
    print("---------------\n", .{});
    const api = try Devices2(native.Devices, native.Device).init(std.testing.allocator);
    defer api.deinit();
    print("---------------\n", .{});
}

test "Devices2 with fake implementation" {
    const api = try Devices2(fake_native.Devices, fake_native.Device).init(std.testing.allocator);
    defer api.deinit();
    print("---------------\n", .{});
}

test "Devices2 with fake finds valid device" {
    const api = try Devices2(fake_native.Devices, fake_native.Device).init(std.testing.allocator);
    defer api.deinit();
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

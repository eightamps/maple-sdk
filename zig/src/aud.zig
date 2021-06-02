const std = @import("std");
const common = @import("os/aud_common.zig");
const fake = @import("os/fake/aud_native.zig");

const target_file = switch (std.Target.current.os.tag) {
    .windows => "os/win/aud_native.zig",
    .linux => "os/nix/aud_native.zig",
    else => "os/fake/aud_native.zig",
};

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const expect = std.testing.expect;
const expectError = std.testing.expectError;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const fs = std.fs;
const heap = std.heap;
const json = std.json;
const mem = std.mem;
const print = std.debug.print;

const native = @import(target_file);
pub usingnamespace common;

pub fn info() []const u8 {
    return native.info();
}

pub fn Devices2(comptime T: type) type {
    return struct {
        allocator: *Allocator,
        delegate: *T,

        pub fn init(a: *Allocator, delegate: *T) !*Devices2(T) {
            const instance = try a.create(Devices2(T));

            instance.* = Devices2(T){
                .allocator = a,
                .delegate = delegate,
            };
            print("Devices2.init called!\n", .{});
            return instance;
        }

        pub fn deinit(self: *Devices2(T)) void {
            self.delegate.deinit();
            self.allocator.destroy(self);
        }

        pub fn getDefaultCaptureDevice(self: *Devices2(T)) !*Device {
            return self.delegate.getDefaultCaptureDevice();
        }

        pub fn getDefaultRenderDevice(self: *Devices2(T)) !*Device {
            return self.delegate.getDefaultRenderDevice();
        }

        pub fn getCaptureDevices(self: *Devices2(T), result: *ArrayList(*Device)) !void {
            return self.delegate.getCaptureDevices(result);
        }

        pub fn getRenderDevices(self: *Devices2(T), result: *ArrayList(*Device)) !void {
            return self.delegate.getRenderDevices(result);
        }

        pub fn getCaptureDeviceAt(self: *Devices2(T), index: u16) *Device {
            return self.delegate.getCaptureDeviceAt(index);
        }

        pub fn getRenderDeviceAt(self: *Devices2(T), index: u16) *Device {
            return self.delegate.getRenderDeviceAt(index);
        }
    };
}

const Devices2Type = Devices2(native.Devices);
const FakeDevices = Devices2(fake.Devices);

const fake_devices_path = "src/fakes/devices.json";
const fake_devices_w2c_defaults = "src/fakes/devices_w2c_defaults.json";
const fake_devices_asi_defaults = "src/fakes/devices_asi_defaults.json";
const fake_devices_only_bad = "src/fakes/devices_only_bad.json";

// Create and return a configured API client for tests.
fn createFakeApi(path: []const u8) !*FakeDevices {
    const alloc = std.testing.allocator;
    // Configure and create the API surface
    const delegate = try fake.Devices.initWithDevicesPath(alloc, path);
    return try FakeDevices.init(alloc, delegate);
}

test "Devices2.getDefaultCaptureDevice returns expected entry" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();

    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices2.getDefaultCaptureDevice cannot be Way2Call" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();

    // Return the zeroth entry b/c W2C is invalid
    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices2.getDefaultCaptureDevice cannot be ASI Telephone" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_asi_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();

    // Return the zeroth entry b/c W2C is invalid
    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices2.getDefaultCaptureDevice fails if no good devices" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_only_bad);
    defer api.deinit(); // Will deinit delegate and self

    var device = api.getDefaultCaptureDevice();

    try expectError(error.Fail, device);
}

test "Devices2.getDefaultRenderDevice returns expected entry" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultRenderDevice();

    try expectEqual(device.id, 1);
    try expectEqualStrings("Built-in Speakers", device.name);
}

test "Devices2.getCaptureDeviceAt" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device_at = api.getCaptureDeviceAt(3);
    try expectEqual(device_at.id, 3);
    try expectEqualStrings("Way2Call (Microphone)", device_at.name);
}

test "Devices2.getRenderDeviceAt" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device_at = api.getRenderDeviceAt(4);
    try expectEqual(device_at.id, 4);
    try expectEqualStrings("Way2Call (Speakers)", device_at.name);
}

test "Devices2.getCaptureDevices" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var result = ArrayList(*Device).init(std.testing.allocator);
    defer result.deinit();

    try api.getCaptureDevices(&result);
    const items = result.items;

    try expectEqual(@as(usize, 3), items.len);

    try expectEqualStrings("Array Microphone", items[0].name);
    try expectEqualStrings("Way2Call (Microphone)", items[1].name);
    try expectEqualStrings("ASI Telephone (Microphone)", items[2].name);
}

test "Devices2.getRenderDevices" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var result = ArrayList(*Device).init(std.testing.allocator);
    defer result.deinit();

    try api.getRenderDevices(&result);
    const items = result.items;

    try expectEqual(@as(usize, 4), items.len);

    try expectEqualStrings("Built-in Speakers", items[0].name);
    try expectEqualStrings("Headphones", items[1].name);
    try expectEqualStrings("Way2Call (Speakers)", items[2].name);
    try expectEqualStrings("ASI Telephone (Speakers)", items[3].name);
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

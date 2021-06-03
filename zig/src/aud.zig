const std = @import("std");
const common = @import("os/aud_common.zig");
const fake = @import("os/fake/aud_native.zig");
const helpers = @import("./helpers.zig");

const target_file = switch (std.Target.current.os.tag) {
    .windows => "os/win/aud_native.zig",
    .linux => "os/nix/aud_native.zig",
    else => "os/fake/aud_native.zig",
};

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const ascii = std.ascii;
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

const WAY2CALL_SUBSTR = "Way2Call";
const ASI_TEL_SUBSTR = "ASI Telephone";
const MAX_DEVICE_COUNT: usize = 128;

pub fn Devices2(comptime T: type) type {
    return struct {
        allocator: *Allocator,
        delegate: *T,

        // Filter device names to ensure they do not contain (case-insensitive) either
        // "Way2Call" or "ASI Telephone". This is used by the default device requests
        // to guarantee we never attempt to send or receive host-side user audio through
        // these known-bad devices, which Microsoft insists on forcing into the default
        // position(s).
        fn isValidDefaultDeviceName(d: Device) bool {
            if (ascii.indexOfIgnoreCasePos(d.name, 0, WAY2CALL_SUBSTR) != null) return false;
            if (ascii.indexOfIgnoreCasePos(d.name, 0, ASI_TEL_SUBSTR) != null) return false;
            return true;
        }

        pub fn init(a: *Allocator, delegate: *T) !*Devices2(T) {
            const instance = try a.create(Devices2(T));

            instance.* = Devices2(T){
                .allocator = a,
                .delegate = delegate,
            };
            return instance;
        }

        pub fn deinit(self: *Devices2(T)) void {
            self.delegate.deinit();
            self.allocator.destroy(self);
        }

        pub fn getDefaultCaptureDevice(self: *Devices2(T)) !?Device {
            // Ask the native implementation for it's default device
            const opt_device = try self.delegate.getDefaultCaptureDevice();
            if (opt_device != null) {
                // TODO(lbayes): Figure out how to make ?* into * without a second variable.
                var device = opt_device orelse &NullCaptureDevice;
                if (isValidDefaultDeviceName(device.*)) {
                    // If the device is a valid default device, return it
                    return device.*;
                }
            }

            // The native default device was not a valid device, get the next candidate.
            var buffer: [MAX_DEVICE_COUNT]Device = undefined;
            var devices = try self.getCaptureDevices(&buffer);
            var filters = [_]DeviceFilter{
                isValidDefaultDeviceName,
            };

            return helpers.firstItemMatching(Device, devices, &filters);
        }

        pub fn getDefaultRenderDevice(self: *Devices2(T)) !?Device {
            // Ask the native implementation for it's default device
            const opt_device = try self.delegate.getDefaultRenderDevice();
            if (opt_device != null) {
                // TODO(lbayes): Figure out how to make ?* into * without a second variable.
                var device = opt_device orelse &NullRenderDevice;
                if (isValidDefaultDeviceName(device.*)) {
                    // If the device is a valid default device, return it
                    return device.*;
                }
            }

            // The native default device was not a valid device, get the next candidate.
            var buffer: [MAX_DEVICE_COUNT]Device = undefined;
            var devices = try self.getRenderDevices(&buffer);
            var filters = [_]DeviceFilter{
                isValidDefaultDeviceName,
            };

            return helpers.firstItemMatching(Device, devices, &filters);
        }

        // Get the collection capture devices.
        pub fn getCaptureDevices(self: *Devices2(T), buffer: []Device) ![]Device {
            return self.delegate.getCaptureDevices(buffer);
        }

        // Get the collection of render devices.
        pub fn getRenderDevices(self: *Devices2(T), buffer: []Device) ![]Device {
            return self.delegate.getRenderDevices(buffer);
        }

        // Get the capture device found at the provided index.
        // The list of all devices is pre-filtered to only include Capture
        // devices.
        pub fn getCaptureDeviceAt(self: *Devices2(T), index: u16) *Device {
            return self.delegate.getCaptureDeviceAt(index);
        }

        // Get the render device found at the provided index.
        // The list of all devices is pre-filtered to only include Capture
        // devices.
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

test "Devices2 Fake is instantiable" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();
}

test "Devices2.getCaptureDevices" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var buffer: [10]Device = undefined;

    const results = try api.getCaptureDevices(&buffer);

    try expectEqual(@as(usize, 3), results.len);
    try expectEqualStrings("Array Microphone", results[0].name);
    try expectEqualStrings("Way2Call (Microphone)", results[1].name);
    try expectEqualStrings("ASI Telephone (Microphone)", results[2].name);
}

test "Devices2.getRenderDevices" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var buffer: [10]Device = undefined;

    const results = try api.getRenderDevices(&buffer);

    try expectEqual(@as(usize, 4), results.len);

    try expectEqualStrings("Built-in Speakers", results[0].name);
    try expectEqualStrings("Headphones", results[1].name);
    try expectEqualStrings("Way2Call (Speakers)", results[2].name);
    try expectEqualStrings("ASI Telephone (Speakers)", results[3].name);
}

test "Devices2.getCaptureDeviceAt" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device_at = api.getCaptureDeviceAt(1);
    try expectEqual(device_at.id, 3);
    try expectEqualStrings("Way2Call (Microphone)", device_at.name);
}

test "Devices2.getRenderDeviceAt" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device_at = api.getRenderDeviceAt(3);
    try expectEqual(device_at.id, 6);
    try expectEqualStrings("ASI Telephone (Speakers)", device_at.name);
}

test "Devices2.getDefaultCaptureDevice returns expected entry" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var opt_device = try api.getDefaultCaptureDevice();
    var device = opt_device orelse NullCaptureDevice;

    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices2.getDefaultCaptureDevice cannot be Way2Call" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var opt_device = try api.getDefaultCaptureDevice();
    var device = opt_device orelse NullCaptureDevice;

    // Return the first non-W2C entry.
    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices2.getDefaultCaptureDevice cannot be ASI Telephone" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_asi_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var opt_device = try api.getDefaultCaptureDevice();
    var device = opt_device orelse NullCaptureDevice;

    // Return the zeroth entry b/c W2C is invalid
    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices2.getDefaultCaptureDevice fails if no good devices" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_only_bad);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();
    try expect(device == null);
}

test "Devices2.getDefaultRenderDevice returns expected entry" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit(); // Will deinit delegate and self

    var opt_device = try api.getDefaultRenderDevice();
    var device = opt_device orelse NullRenderDevice;

    try expectEqual(device.id, 1);
    try expectEqualStrings("Built-in Speakers", device.name);
}

test "Devices2.getDefaultRenderDevice cannot be Way2Call" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var opt_device = try api.getDefaultRenderDevice();
    var device = opt_device orelse NullRenderDevice;

    // Return the first non-W2C entry.
    try expectEqual(device.id, 1);
    try expectEqualStrings("Built-in Speakers", device.name);
}

pub const Devices = struct {
    allocator: *Allocator,
    delegate: *native.Devices,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);
        const delegate = try native.Devices.init(a);

        instance.* = Devices{
            .allocator = a,
            .delegate = delegate,
        };
        return instance;
    }

    pub fn deinit(self: *Devices) void {
        print("AudibleApi.deinit called\n", .{});
        self.delegate.deinit();
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
            const index = try self.delegate.getDefaultRenderDeviceIndex();
            const sio_device = try self.delegate.getRenderDeviceByIndex(index);
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
        return try self.delegate.createDevice(common.Direction.Render);
    }

    pub fn getCaptureDevice(self: *Devices, matcher: common.Matcher) !*native.Device {
        return try self.delegate.createDevice(common.Direction.Capture);
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

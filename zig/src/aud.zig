const std = @import("std");
const common = @import("os/aud_common.zig");
const fake = @import("os/fake/aud_native.zig");
const helpers = @import("./helpers.zig");

const target_file = switch (std.Target.current.os.tag) {
    .windows => "os/win/aud_native.zig",
    .linux => "os/nix/aud_native.zig",
    else => "os/fake/aud_native.zig",
};

const native = @import(target_file);

pub usingnamespace common;

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

const WAY2CALL_SUBSTR = "Way2Call";
const ASI_TEL_SUBSTR = "ASI Telephone";
const MAX_DEVICE_COUNT: usize = 128;

pub fn Devices(comptime T: type) type {
    return struct {
        allocator: *Allocator,
        delegate: *T,

        pub fn init(a: *Allocator) !*Devices(T) {
            var delegate = try native.Devices.init(a);
            return init_with_delegate(a, delegate);
        }

        pub fn init_with_delegate(a: *Allocator, delegate: *T) !*Devices(T) {
            var instance = try a.create(Devices(T));

            instance.* = Devices(T){
                .allocator = a,
                .delegate = delegate,
            };
            return instance;
        }

        pub fn deinit(self: *Devices(T)) void {
            self.delegate.deinit();
            self.allocator.destroy(self);
        }

        pub fn info(self: *Devices(T)) []const u8 {
            return self.delegate.info();
        }

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

        pub fn getDefaultDevice(self: *Devices(T), direction: Direction) !Device {
            // Ask the native implementation for it's default device
            const device = try self.delegate.getDefaultDevice(direction);
            if (isValidDefaultDeviceName(device)) {
                // If the device is a valid default device, return it
                return device;
            }

            // The native default device was not a valid device, get the next candidate.
            var buffer: [MAX_DEVICE_COUNT]Device = undefined;
            var devices = try self.getDevices(&buffer, direction);
            var filters = [_]DeviceFilter{
                isValidDefaultDeviceName,
            };

            return helpers.firstItemMatching(Device, devices, &filters) orelse error.Fail;
        }

        // Get the default capture device (i.e., Microphone) that is not presented with a
        // blocked name.
        pub fn getDefaultCaptureDevice(self: *Devices(T)) !Device {
            return self.getDefaultDevice(Direction.Capture);
        }

        // Get the default render device (i.e., Speakers) that is not presented with a
        // blocked name.
        pub fn getDefaultRenderDevice(self: *Devices(T)) !Device {
            return self.getDefaultDevice(Direction.Render);
        }

        pub fn getDevices(self: *Devices(T), buffer: []Device, direction: Direction) ![]Device {
            return self.delegate.getDevices(buffer, direction);
        }

        // Get the collection capture devices.
        pub fn getCaptureDevices(self: *Devices(T), buffer: []Device) ![]Device {
            return self.delegate.getCaptureDevices(buffer);
        }

        // Get the collection of render devices.
        pub fn getRenderDevices(self: *Devices(T), buffer: []Device) ![]Device {
            return self.delegate.getRenderDevices(buffer);
        }

        // Get the capture device found at the provided index.
        // The list of all devices is pre-filtered to only include Capture
        // devices.
        pub fn getCaptureDeviceAt(self: *Devices(T), index: u16) *Device {
            return self.delegate.getCaptureDeviceAt(index);
        }

        // Get the render device found at the provided index.
        // The list of all devices is pre-filtered to only include Capture
        // devices.
        pub fn getRenderDeviceAt(self: *Devices(T), index: u16) *Device {
            return self.delegate.getRenderDeviceAt(index);
        }
    };
}

pub const NativeDevices = Devices(native.Devices);

const FakeDevices = Devices(fake.Devices);

const fake_devices_path = "src/fakes/devices.json";
const fake_devices_w2c_defaults = "src/fakes/devices_w2c_defaults.json";
const fake_devices_asi_defaults = "src/fakes/devices_asi_defaults.json";
const fake_devices_only_bad = "src/fakes/devices_only_bad.json";

// Create and return a configured API client for tests.
fn createFakeApi(path: []const u8) !*FakeDevices {
    const alloc = std.testing.allocator;
    // Configure and create the API surface
    const delegate = try fake.Devices.initWithDevicesPath(alloc, path);
    return try FakeDevices.init_with_delegate(alloc, delegate);
}

test "Devices Fake is instantiable" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();
}

test "Devices.getCaptureDevices" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var buffer: [10]Device = undefined;

    const results = try api.getCaptureDevices(&buffer);

    try expectEqual(@as(usize, 3), results.len);
    try expectEqualStrings("Array Microphone", results[0].name);
    try expectEqualStrings("Way2Call (Microphone)", results[1].name);
    try expectEqualStrings("ASI Telephone (Microphone)", results[2].name);
}

test "Devices.getRenderDevices" {
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

test "Devices.getCaptureDeviceAt" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device_at = api.getCaptureDeviceAt(1);
    try expectEqual(device_at.id, 3);
    try expectEqualStrings("Way2Call (Microphone)", device_at.name);
}

test "Devices.getRenderDeviceAt" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device_at = api.getRenderDeviceAt(3);
    try expectEqual(device_at.id, 6);
    try expectEqualStrings("ASI Telephone (Speakers)", device_at.name);
}

test "Devices.getDefaultCaptureDevice returns expected entry" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device = try api.getDefaultCaptureDevice();

    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices.getDefaultCaptureDevice cannot be Way2Call" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();

    // Return the first non-W2C entry.
    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices.getDefaultCaptureDevice cannot be ASI Telephone" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_asi_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();

    // Return the zeroth entry b/c W2C is invalid
    try expectEqual(device.id, 0);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices.getDefaultCaptureDevice fails if no good devices" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_only_bad);
    defer api.deinit(); // Will deinit delegate and self

    try expectError(error.Fail, api.getDefaultCaptureDevice());
}

test "Devices.getDefaultRenderDevice returns expected entry" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultRenderDevice();

    try expectEqual(device.id, 1);
    try expectEqualStrings("Built-in Speakers", device.name);
}

test "Devices.getDefaultRenderDevice cannot be Way2Call" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultRenderDevice();

    // Return the first non-W2C entry.
    try expectEqual(device.id, 1);
    try expectEqualStrings("Built-in Speakers", device.name);
}

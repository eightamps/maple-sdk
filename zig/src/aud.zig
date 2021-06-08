const std = @import("std");
const common = @import("./aud/common.zig");
const fake = @import("./aud/fake.zig");
const helpers = @import("./helpers.zig");

// Get the native audible implementation
const target_file = switch (std.Target.current.os.tag) {
    .windows => "./aud/soundio.zig",
    .linux => "./aud/soundio.zig",
    else => "./aud_fake.zig", // TODO(lbayes): Should be error
};
const native = @import(target_file);

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const ascii = std.ascii;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const expectError = std.testing.expectError;
const mem = std.mem;
const print = std.debug.print;
const talloc = std.testing.allocator;

pub const MAX_DEVICE_COUNT = common.MAX_DEVICE_COUNT;
pub const Device = common.Device;
pub const Direction = common.Direction;
pub const DeviceFilter = common.DeviceFilter;

const PreferredIds = ArrayList([]const u8);

pub fn Devices(comptime T: type) type {
    return struct {
        allocator: *Allocator,
        delegate: *T,
        preferred: PreferredIds,

        pub fn init(a: *Allocator) !*Devices(T) {
            var delegate = try native.Devices.init(a);
            return init_with_delegate(a, delegate);
        }

        pub fn init_with_delegate(a: *Allocator, delegate: *T) !*Devices(T) {
            var instance = try a.create(Devices(T));

            instance.* = Devices(T){
                .allocator = a,
                .delegate = delegate,
                .preferred = PreferredIds.init(a),
            };
            return instance;
        }

        pub fn deinit(self: *Devices(T)) void {
            self.preferred.deinit();
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
        fn isValidDefaultCaptureDevice(d: Device) bool {
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.WAY2CALL) != null) return false;
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.ASI_TELEPHONE) != null) return false;
            return true;
        }

        fn isValidDefaultRenderDevice(d: Device) bool {
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.WAY2CALL) != null) return false;
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.ASI_TELEPHONE) != null) return false;
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.ASI_MICROPHONE) != null) return false;
            return true;
        }

        fn getDeviceFilterFor(self: *Devices(T), direction: Direction) DeviceFilter {
            if (direction == Direction.Capture) {
                return isValidDefaultCaptureDevice;
            } else {
                return isValidDefaultRenderDevice;
            }
        }

        pub fn getDefaultDevice(self: *Devices(T), direction: Direction) !Device {
            // Ask the native implementation for it's default device
            const device = try self.delegate.getDefaultDevice(direction);
            const directionFilter = self.getDeviceFilterFor(direction);
            if (directionFilter(device)) {
                // If the device is a valid default device, return it
                return device;
            }

            // The native default device was not a valid device, get the next candidate.
            var buf_a: [common.MAX_DEVICE_COUNT]Device = undefined;
            var all_devices = try self.getDevices(&buf_a, direction);
            var filters = [_]DeviceFilter{
                directionFilter,
            };

            var buf_b: [common.MAX_DEVICE_COUNT]Device = undefined;
            // Get a subset of all_devices that only includes valid device names.
            var valid_devices = helpers.filterItems(Device, all_devices, &buf_b, &filters);

            if (valid_devices.len == 0) {
                return error.Fail;
            }

            for (self.preferred.items) |pref| {
                for (valid_devices) |dev| {
                    if (mem.eql(u8, pref, dev.id)) {
                        return dev;
                    }
                }
            }

            return helpers.firstItemMatching(Device, valid_devices, &filters) orelse error.Fail;
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

        // Push a new device native id to the top of the list of preferred devices.
        // This list will be used to select a default device whenever one of the prohibited device
        // names is returned from the platform as if it were a default device.
        //
        // Each call to this method will push the provided id to the top of the list and the
        // entire list will be scanned until the first expected device is found.
        pub fn pushPreferredNativeId(self: *Devices(T), id: []const u8, direction: Direction) !void {
            try self.preferred.insert(0, id);
        }

        // Clear the accumulated list of preferred device ids.
        pub fn clearPreferredIds(self: *Devices(T)) void {
            self.preferred.clearRetainingCapacity();
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
    // Configure and create the API surface
    const delegate = try fake.Devices.initWithDevicesPath(talloc, path);
    return try FakeDevices.init_with_delegate(talloc, delegate);
}

test "Devices Fake is instantiable" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();
}

test "Devices.getCaptureDevices" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var buffer: [common.MAX_DEVICE_COUNT]Device = undefined;

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
    try expectEqualStrings("3", device_at.id);
    try expectEqualStrings("Way2Call (Microphone)", device_at.name);
}

test "Devices.getRenderDeviceAt" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device_at = api.getRenderDeviceAt(3);
    try expectEqualStrings("6", device_at.id);
    try expectEqualStrings("ASI Telephone (Speakers)", device_at.name);
}

test "Devices.getDefaultCaptureDevice returns expected entry" {
    var api = try createFakeApi(fake_devices_path);
    defer api.deinit();

    var device = try api.getDefaultCaptureDevice();

    try expectEqualStrings("0", device.id);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices.getDefaultCaptureDevice cannot be Way2Call" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();

    // Return the first non-W2C entry.
    try expectEqualStrings("0", device.id);
    try expectEqualStrings("Array Microphone", device.name);
}

test "Devices.getDefaultCaptureDevice cannot be ASI Telephone" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_asi_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultCaptureDevice();

    // Return the zeroth entry b/c W2C is invalid
    try expectEqualStrings("0", device.id);
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

    try expectEqualStrings("1", device.id);
    try expectEqualStrings("Built-in Speakers", device.name);
}

test "Devices.getDefaultRenderDevice cannot be Way2Call" {
    // Get the dataset that has W2C as default mic
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    var device = try api.getDefaultRenderDevice();

    // Return the first non-W2C entry.
    try expectEqualStrings("1", device.id);
    try expectEqualStrings("Built-in Speakers", device.name);
}

test "Devices saves preferred device ids" {
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit(); // Will deinit delegate and self

    try api.pushPreferredNativeId("headset-spkr", Direction.Render); // second priority
    try api.pushPreferredNativeId("bt-spkr", Direction.Render); // first priority

    var speaker = try api.getDefaultRenderDevice();
    try expectEqualStrings("bt-spkr", speaker.id);
    try expectEqualStrings("Bluetooth Speaker", speaker.name);

    api.clearPreferredIds();
    try api.pushPreferredNativeId("headset-spkr", Direction.Render); // only priority

    var headset = try api.getDefaultRenderDevice();
    try expectEqualStrings("headset-spkr", headset.id);
    try expectEqualStrings("Bluetooth Headset (Speakers)", headset.name);
}

// NOTE(lbayes): This will load whichever implementation is appropriate
// for the environment where these tests are being COMPILED. Probably
// not exactly what we want, but slightly better than nothing?
test "Devices native implementation gets tested" {
    var api = try NativeDevices.init(talloc);
    defer api.deinit();
}

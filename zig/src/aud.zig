const common = @import("aud/common.zig");
const fake = @import("aud/fake.zig");
const helpers = @import("helpers.zig");
const ring_buffer = @import("ring_buffer.zig");
const std = @import("std");

// Get the native audible implementation
const native = @import(switch (std.Target.current.os.tag) {
    .windows => "aud/soundio.zig",
    .linux => "aud/soundio.zig",
    else => "aud_fake.zig", // TODO(lbayes): Should be error
});

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const AudioSampleRingBuffer = ring_buffer.AudioSampleRingBuffer;
const StreamConfig = common.StreamConfig;
const ConnectContext = common.ConnectContext;
const Thread = std.Thread;
const ascii = std.ascii;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const expectError = std.testing.expectError;
const fifo = std.fifo;
const mem = std.mem;
const print = std.debug.print;
const talloc = std.testing.allocator;
const time = std.time;

// App constants
pub const Device = common.Device;
pub const DeviceFilter = common.DeviceFilter;
pub const Direction = common.Direction;
pub const MAX_DEVICE_COUNT = common.MAX_DEVICE_COUNT; // TODO(lbayes): Fix case
pub const NativeDevices = Devices(native.Devices);
pub const TelephoneSampleDurationSecs = 5;
pub const TelephoneSampleRate = 16000;

const PreferredIds = ArrayList([]const u8);
const OuterThreadSleepMs = 10 * time.ns_per_ms;
// Create a RingBuffer type that will hold a 16kHz sample rate for 5 seconds
// (before dropping samples).
const TelephoneBuffer = AudioSampleRingBuffer(TelephoneSampleRate, TelephoneSampleDurationSecs);

// Test constants
const FakeDevices = Devices(fake.Devices);
const fake_devices_path = "src/fakes/devices.json";
const fake_devices_w2c_defaults = "src/fakes/devices_w2c_defaults.json";
const fake_devices_asi_defaults = "src/fakes/devices_asi_defaults.json";
const fake_devices_only_bad = "src/fakes/devices_only_bad.json";

pub const AudError = error{
    ArgumentError,
    DeviceNotFound,
};

pub fn Devices(comptime T: type) type {
    return struct {
        const Self = @This();

        allocator: *Allocator,
        delegate: *T,
        preferred: PreferredIds,
        connect_context: *ConnectContext = undefined,

        fn captureCallback(context: *ConnectContext, frame_count_min: u32, frame_count_max: u32) void {
            print("captureCallback with: {d} {d}\n", .{ frame_count_min, frame_count_max });
        }

        pub fn init(a: *Allocator) !*Self {
            var delegate = try native.Devices.init(a);
            return init_with_delegate(a, delegate);
        }

        pub fn init_with_delegate(a: *Allocator, delegate: *T) !*Self {
            var instance = try a.create(Self);

            instance.* = Self{
                .allocator = a,
                .delegate = delegate,
                .preferred = PreferredIds.init(a),
            };
            return instance;
        }

        pub fn deinit(self: *Self) void {
            if (self.connect_context != undefined) {
                self.allocator.destroy(self.connect_context);
            }
            self.preferred.deinit();
            self.delegate.deinit();
            self.allocator.destroy(self);
        }

        pub fn info(self: *Self) []const u8 {
            return self.delegate.info();
        }

        // Filter device names to ensure they do not contain (case-insensitive)
        // either "Way2Call" or "ASI Telephone". This is used by the default
        // device requests to guarantee we never attempt to send or receive
        // host-side user audio through these known-bad devices, which
        // Microsoft insists on forcing into the default position(s).
        fn isValidDefaultCaptureDevice(d: Device) bool {
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.WAY2CALL) != null) return false;
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.ASI_TELEPHONE) != null) return false;
            return true;
        }

        // Return true if ths provided device can be used as a render device.
        //
        // In other words, this device does not present any of our blocked device names.
        fn isValidDefaultRenderDevice(d: Device) bool {
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.WAY2CALL) != null) return false;
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.ASI_TELEPHONE) != null) return false;
            if (ascii.indexOfIgnoreCasePos(d.name, 0, common.ASI_MICROPHONE) != null) return false;
            return true;
        }

        fn getDeviceFilterFor(self: *Self, direction: Direction) DeviceFilter {
            if (direction == Direction.Capture) {
                return isValidDefaultCaptureDevice;
            } else {
                return isValidDefaultRenderDevice;
            }
        }

        // Get the default device from the underlying operating system, and if
        // the device name is expected to be blocked, use the first preferred
        // device (if found), or the first device in the list if no preferred
        // devices are found.
        pub fn getDefaultDevice(self: *Self, direction: Direction) !Device {
            // Ask the native implementation for it's default device
            const device = try self.delegate.getDefaultDevice(direction);
            const directionFilter = self.getDeviceFilterFor(direction);
            if (directionFilter(device)) {
                // If the device is a valid default device, return it
                return device;
            }

            // The native default device was not a valid device, get the next candidate.
            var buf_a: [MAX_DEVICE_COUNT]Device = undefined;
            var all_devices = try self.getDevices(&buf_a, direction);
            var filters = [_]DeviceFilter{
                directionFilter,
            };

            var buf_b: [MAX_DEVICE_COUNT]Device = undefined;
            // Get a subset of all_devices that only includes valid device names.
            var valid_devices = helpers.filterItems(Device, all_devices, &buf_b, &filters);

            if (valid_devices.len == 0) {
                return AudError.DeviceNotFound;
            }

            for (self.preferred.items) |pref| {
                for (valid_devices) |dev| {
                    if (mem.eql(u8, pref, dev.id)) {
                        return dev;
                    }
                }
            }

            return helpers.firstItemMatching(Device, valid_devices, &filters) orelse AudError.DeviceNotFound;
        }

        // Get the default capture device (i.e., Microphone) that is not presented with a
        // blocked name.
        pub fn getDefaultCaptureDevice(self: *Self) !Device {
            return self.getDefaultDevice(Direction.Capture);
        }

        // Get the default render device (i.e., Speakers) that is not presented with a
        // blocked name.
        pub fn getDefaultRenderDevice(self: *Self) !Device {
            return self.getDefaultDevice(Direction.Render);
        }

        pub fn getDevices(self: *Self, buffer: []Device, direction: Direction) ![]Device {
            return self.delegate.getDevices(buffer, direction);
        }

        // Get the collection capture devices.
        pub fn getCaptureDevices(self: *Self, buffer: []Device) ![]Device {
            return self.delegate.getCaptureDevices(buffer);
        }

        // Get the collection of render devices.
        pub fn getRenderDevices(self: *Self, buffer: []Device) ![]Device {
            return self.delegate.getRenderDevices(buffer);
        }

        // Get the capture device found at the provided index.
        // The list of all devices is pre-filtered to only include Capture
        // devices.
        pub fn getCaptureDeviceAt(self: *Self, index: u16) Device {
            return self.delegate.getCaptureDeviceAt(index);
        }

        // Get the render device found at the provided index.
        // The list of all devices is pre-filtered to only include Capture
        // devices.
        pub fn getRenderDeviceAt(self: *Self, index: u16) Device {
            return self.delegate.getRenderDeviceAt(index);
        }

        // Push a new device native id to the top of the list of preferred
        // devices.
        //
        // This list will be used to select a default device whenever one of the
        // prohibited device names is returned from the platform as if it were a
        // default device.
        //
        // Each call to this method will push the provided id to the top of the
        // list and the entire list will be scanned until the first expected
        // device is found.
        pub fn pushPreferredNativeId(self: *Self, id: []const u8, direction: Direction) !void {
            try self.preferred.insert(0, id);
        }

        // Clear the accumulated list of preferred device ids.
        pub fn clearPreferredIds(self: *Self) void {
            self.preferred.clearRetainingCapacity();
        }

        // Get the first device where it's name and direction match the
        // provided values.
        pub fn getDeviceWithName(self: *Self, name: []const u8, direction: Direction) !Device {
            var buffer: [MAX_DEVICE_COUNT]Device = undefined;
            const devices = try self.getDevices(&buffer, direction);
            for (devices) |device| {
                if (ascii.indexOfIgnoreCasePos(device.name, 0, name) != null) {
                    return device;
                }
            }
            return AudError.DeviceNotFound;
        }

        // Connect the streams for the provided pair of devices and return
        // a ThreadContext object for external control.
        pub fn connect(self: *Self, render_device: Device, capture_device: Device) !*ConnectContext {
            if (render_device.direction != Direction.Render) {
                print("connect expected a render device but received a capture instead\n", .{});
                return AudError.ArgumentError;
            }

            if (capture_device.direction != Direction.Capture) {
                print("connect expected a capture device but received a render instead\n", .{});
                return AudError.ArgumentError;
            }

            // var buffer = TelephoneBuffer.init(self.allocator);
            var context = try self.allocator.create(ConnectContext);

            context.* = ConnectContext{
                .capture_device = capture_device,
                .render_device = render_device,
                // .buffer = buffer,
                // .bytes_per_frame: u32 = 0,
                // .bytes_per_sample: u32 = 0,
                // .capture_callback: fn (context: *ConnectContext, frame_count_min: u32, frame_count_max: u32) void,
            };
            self.connect_context = context;
            context.outer_thread = try Thread.spawn(connectDevices, self);

            return context;
        }

        pub fn wait(self: *Self) void {
            if (self.connected_thread != null) {
                self.connected_thread.wait();
            }
        }

        pub fn disconnect(self: *Self) void {
            self.connected_thread.stop();
        }

        // Must be unbound for threads to work with it.
        fn connectDevices(self: *Self) u8 {
            self.delegate.startCapture(self.connect_context);

            while (self.connect_context.is_active) {
                time.sleep(OuterThreadSleepMs);
            }

            return 0;
        }
    };
}

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

    var buffer: [MAX_DEVICE_COUNT]Device = undefined;

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

    try expectError(AudError.DeviceNotFound, api.getDefaultCaptureDevice());
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

test "Devices getDeviceWithName" {
    var api = try createFakeApi(fake_devices_w2c_defaults);
    defer api.deinit();

    var phone_speaker = try api.getDeviceWithName("ASI Telephone", Direction.Capture);
    try expectEqualStrings("5", phone_speaker.id);

    var phone_mic = try api.getDeviceWithName("ASI Telephone", Direction.Render);
    try expectEqualStrings("6", phone_mic.id);
}

test "Aud can access RingBuffer" {
    const rb = try TelephoneBuffer.init(talloc);
    defer rb.deinit();
}

// const TestRingBuffer = fifo.LinearFifo(f32, fifo.LinearFifoBufferType.Dynamic);
//
// const TestBufContext = struct {
//     buffer: TestRingBuffer,
//     should_exit: bool = false,
//     should_write: bool = false,
//     should_read: bool = false,
//     read_complete: bool = true,
//     read_mb: u128 = 0,
// };
//
// fn testWriter(ctx: *TestBufContext) u8 {
//     var last_value: f32 = 0.0;
//     var items: [10]f32 = undefined;
//     while (!ctx.should_exit) {
//         if (ctx.should_write) {
//             ctx.should_write = false;
//             var index: usize = 0;
//             while (index < items.len) {
//                 last_value += 0.001;
//                 if (last_value > 100.0) {
//                     last_value = 0.0;
//                 }
//                 items[index] = last_value;
//                 index += 1;
//             }
//             ctx.buffer.writeAssumeCapacity(&items);
//             ctx.should_read = true;
//         }
//     }
//     print("writer exiting now\n", .{});
//     return 0;
// }
//
// fn testReader(ctx: *TestBufContext) u8 {
//     var result: [1000]f32 = undefined;
//     var read_count: u128 = 0;
//     var loop_count: u128 = 0;
//     var len: usize = 0;
//     while (!ctx.should_exit) {
//         if (ctx.should_read and ctx.buffer.readableLength() > 0) {
//             ctx.should_read = false;
//             len = ctx.buffer.read(&result);
//             // const result = ctx.buffer.readableSlice(0);
//             // if (len != 10) {
//             // print("READ result: {d}\n", .{len});
//             // }
//             read_count += len * @sizeOf(f32);
//             ctx.read_mb = read_count / 1_024_000;
//
//             loop_count += 1;
//
//             if (loop_count > 1_024_000) {
//                 print("read {d} MB\n", .{ctx.read_mb});
//                 loop_count = 0;
//             }
//
//             // time.sleep(1);
//             // time.sleep(1 * time.ns_per_ms);
//             ctx.should_write = true;
//         }
//     }
//     print("reader exiting now\n", .{});
//     return 0;
// }
//
// test "Devices buffer" {
//     print("\n", .{});
//     print("\n", .{});
//     var buf = TestRingBuffer.init(talloc);
//     // try buf.ensureCapacity(128);
//     try buf.ensureCapacity(2048);
//     defer buf.deinit();
//
//     print("\n", .{});
//     print("----------------------------------------\n", .{});
//     var context = TestBufContext{ .buffer = buf };
//     const reader = try Thread.spawn(testReader, &context);
//     const writer = try Thread.spawn(testWriter, &context);
//
//     context.should_write = true;
//
//     while (context.read_mb < 1_000) {
//         time.sleep(100 * time.ns_per_ms);
//     }
//
//     context.should_exit = true;
//     writer.wait();
//     reader.wait();
//
//     print("Exiting successfully\n", .{});
//     print("----------------------------------------\n", .{});
//
//     // buf.writeAssumeCapacity(&[_]f32{
//     //     0.1,
//     //     0.2,
//     //     0.3,
//     //     0.4,
//     //     0.5,
//     // });
//
//     // var result: ?f32 = undefined;
//
//     // const byte_count = buf.readableLength();
//     // const bytes = buf.readableSlice(0);
//     // print("BYTES: {any}\n", .{bytes});
//
//     // while (buf.readableLength() > 0) {
//     //     result = buf.readItem();
//     //     print("RESULT: {d}\n", .{result});
//     // }
//     // result = buf.readItem();
//     // print("RESULT: {d}\n", .{result});
//     // result = buf.readItem();
//     // print("RESULT: {d}\n", .{result});
//     // result = buf.readItem();
//     // print("RESULT: {d}\n", .{result});
//     // result = buf.readItem();
//     // print("RESULT: {d}\n", .{result});
//     // result = buf.readItem();
//     // print("RESULT: {d}\n", .{result});
//
// }

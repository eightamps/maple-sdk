const std = @import("std");
const testing = std.testing;

const ASI_TELEPHONE: []const u8 = "ASI Telephone";
const WAY2CALL: []const u8 = "Way2Call";

pub const Direction = enum(u8) {
    Capture = 0,
    Render,
};

pub const Role = enum(u8) {
    Communication = 0,
    Console,
    Media,
};

pub const Matcher = struct {
    role: Role = Role.Communication,
    direction: Direction,
    is_default: bool = false,
    // NOTE(lbayes): I'd prefer these to either be arrays of strings or even
    // better, Regex. Unfortunately, I don't know how to declare an optional
    // array of arrays of u8's with variable lengths on every level, and it
    // seems the Regex library is still under development (for now). Just
    // going to make it a post-delimited input string that gets split when
    // used.
    matches: ?[]const u8 = null,
    not_matches: ?[]const u8 = null,
};

// DefaultCapture is a configured Matcher that will not allow devices with
// names that include "ASI Telephone" or "Way2Call" in their device name.
pub const DefaultCapture = Matcher{
    .role = Role.Communication,
    .direction = Direction.Capture,
    .is_default = true,
    .not_matches = ASI_TELEPHONE ++ "|" ++ WAY2CALL,
};

// DefaultRender is a configured Matcher that will not allow devices with
// names that include "ASI Telephone" or "Way2Call" in their device name.
pub const DefaultRender = Matcher{
    .role = Role.Communication,
    .direction = Direction.Render,
    .is_default = true,
    .not_matches = ASI_TELEPHONE ++ "|" ++ WAY2CALL,
};

// TODO(lbayes): I want this device to wrap platform-specific features and get
// configured at compile time.
// I've tried all kinds of things, but haven't been able to land on something
// that both works and doesn't feel leaky.
pub const AudioDevice = struct {};

test "Device Instantiated" {
    // const d = Device(Win32Device);
    // std.debug.print("\nYOO {s}\n", .{d});
    // d.start();
}

// pub const AudioDevice = struct {
//     pub fn init(comptime T: type) AudioDevice {
//         return AudioDevice{};
//     }
//
//     pub fn deinit(self: NativeDevice) void {}
//
//     pub fn start(self: NativeDevice) void {}
//
//     pub fn stop(self: NativeDevice) void {}
// };

// pub const AudioDevice = struct {
// matcher: ?Matcher = null,
// TODO(lbayes): Make these non-nullable
// device: ?NativeDevice = null,

// };

// pub fn CreateDevice(comptime T: type) T {

// pub fn CreateAudioDevice(comptime T: type, dev: T) AudioDevice {
// const AudioDevce = struct {
// matcher: ?Matcher = null,
// device: T = null,
// };
//
// return AudioDevice{
// .device = dev,
// };

// const a = AudioDevice{};
// return a;
// }

test "Matcher Capture & default" {
    const m = Matcher{ .direction = Direction.Capture };
    try testing.expectEqual(m.direction, Direction.Capture);

    // Verify default values:
    try testing.expectEqual(m.is_default, false);
    try testing.expectEqual(m.role, Role.Communication);
    try testing.expectEqual(m.matches, null);
    try testing.expectEqual(m.not_matches, null);
}

test "Matcher Render" {
    const m = Matcher{ .direction = Direction.Render };
    try testing.expectEqual(m.direction, Direction.Render);
}

// NOTE(lbayes): These are just some simple assertions to make sure I've got
// the test environment up and running.
test "DefaultCapture" {
    try testing.expect(DefaultCapture.role == Role.Communication);
    try testing.expect(DefaultCapture.direction == Direction.Capture);
    try testing.expect(DefaultCapture.is_default == true);
    try testing.expectEqual(DefaultCapture.not_matches, ASI_TELEPHONE ++ "|" ++ WAY2CALL);
}

test "DefaultRender" {
    try testing.expect(DefaultRender.role == Role.Communication);
    try testing.expect(DefaultRender.direction == Direction.Render);
    try testing.expect(DefaultRender.is_default == true);
    try testing.expectEqual(DefaultRender.not_matches, ASI_TELEPHONE ++ "|" ++ WAY2CALL);
}

// test "AudioDevice empty creation" {
// const a = AudioDevice{};
// }

// test "NativeDevice" {
// const a = AudioDevice.init(Win32Device);
// }

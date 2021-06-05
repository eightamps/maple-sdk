const std = @import("std");
const testing = std.testing;

const DEFAULT_EXCLUDES: []const u8 = ASI_TELEPHONE ++ "|" ++ WAY2CALL;
// const DEFAULT_EXCLUDES: []const u8 = ASI_TELEPHONE ++ "|" ++ WAY2CALL ++ "|" ++ "hda-dsp";
const EMPTY_MATCHES: []const u8 = "";

pub const MAX_DEVICE_COUNT: usize = 128;
pub const ASI_TELEPHONE: []const u8 = "ASI Telephone";
pub const WAY2CALL: []const u8 = "Way2Call";
pub const NULL_DEVICE_NAME = "[NullDevice]";

pub const Device = struct {
    id: u16,
    name: []const u8,
    direction: Direction = Direction.Render,
    is_default: bool = false,
    rank: u8 = 0,
    native_id: []const u8 = undefined,
    // sample_rate: u32,
    // channel_count: u8,

    pub fn equals(self: *Device, d: *Device) bool {
        return self.id == d.id and
            self.name == d.name and
            self.direction == d.direction and
            self.is_default == d.is_default;
    }
};

pub fn isRenderDevice(d: Device) bool {
    return d.direction == Direction.Render;
}

pub fn isCaptureDevice(d: Device) bool {
    return d.direction == Direction.Capture;
}

pub fn isDefaultDevice(d: Device) bool {
    return d.is_default;
}

pub const DeviceFilter = fn (Device) bool;

pub var NullDevice = Device{
    .id = 254,
    .name = NULL_DEVICE_NAME,
    .direction = Direction.Capture,
    .is_default = false,
};

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
    direction: Direction,
    role: Role = Role.Communication,
    is_default: bool = false,
    // NOTE(lbayes): I'd prefer these to either be arrays of strings or even
    // better, Regex. Unfortunately, I don't know how to declare an optional
    // array of arrays of u8's with variable lengths on every level, and it
    // seems the Regex library is still under development (for now). Just
    // going to make it a post-delimited input string that gets split when
    // used.
    matches: []const u8,
    not_matches: []const u8,
};

// DefaultCapture is a configured Matcher that will not allow devices with
// names that include "ASI Telephone" or "Way2Call" in their device name.
pub const DefaultCapture = Matcher{
    .role = Role.Communication,
    .direction = Direction.Capture,
    .is_default = true,
    .matches = EMPTY_MATCHES,
    .not_matches = DEFAULT_EXCLUDES,
};

// DefaultRender is a configured Matcher that will not allow devices with
// names that include "ASI Telephone" or "Way2Call" in their device name.
pub const DefaultRender = Matcher{
    .role = Role.Communication,
    .direction = Direction.Render,
    .is_default = true,
    .matches = EMPTY_MATCHES,
    .not_matches = DEFAULT_EXCLUDES,
};

test "Device Instantiated" {
    // const d = Device(Win32Device);
    // std.debug.print("\nYOO {s}\n", .{d});
    // d.start();
}

test "Matcher Capture & default" {
    const m = Matcher{
        .direction = Direction.Capture,
        .matches = EMPTY_MATCHES,
        .not_matches = EMPTY_MATCHES,
    };
    try testing.expectEqual(m.direction, Direction.Capture);

    // Verify default values:
    try testing.expectEqual(m.is_default, false);
    try testing.expectEqual(m.role, Role.Communication);
    try testing.expectEqual(m.matches, EMPTY_MATCHES);
    try testing.expectEqual(m.not_matches, EMPTY_MATCHES);
}

test "Matcher Render" {
    const m = Matcher{
        .direction = Direction.Render,
        .matches = EMPTY_MATCHES,
        .not_matches = EMPTY_MATCHES,
    };
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

const std = @import("std");
const testing = std.testing;

const DEFAULT_EXCLUDES: []const u8 = ASI_TELEPHONE ++ "|" ++ WAY2CALL;
// const DEFAULT_EXCLUDES: []const u8 = ASI_TELEPHONE ++ "|" ++ WAY2CALL ++ "|" ++ "hda-dsp";
const EMPTY_MATCHES: []const u8 = "";

pub const MAX_DEVICE_COUNT: usize = 128;
pub const ASI_TELEPHONE: []const u8 = "ASI Telephone";
pub const ASI_MICROPHONE: []const u8 = "ASI Microphone";
pub const WAY2CALL: []const u8 = "Way2Call";

pub const Device = struct {
    id: []const u8 = "",
    name: []const u8,
    direction: Direction = Direction.Render,
    is_default: bool = false,
    rank: u8 = 0,
    // sample_rate: u32,
    // channel_count: u8,
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

pub const Direction = enum(u8) {
    Capture = 0,
    Render,
};

pub const Role = enum(u8) {
    Communication = 0,
    Console,
    Media,
};

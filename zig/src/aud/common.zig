const std = @import("std");

const Thread = std.Thread;
const testing = std.testing;

const DEFAULT_EXCLUDES: []const u8 = ASI_TELEPHONE ++ "|" ++ WAY2CALL;
// const DEFAULT_EXCLUDES: []const u8 = ASI_TELEPHONE ++ "|" ++ WAY2CALL ++ "|" ++ "hda-dsp";
const EMPTY_MATCHES: []const u8 = "";

pub const MAX_DEVICE_COUNT: usize = 128;
pub const ASI_TELEPHONE: []const u8 = "ASI Telephone";
pub const ASI_MICROPHONE: []const u8 = "ASI Microphone";
pub const WAY2CALL: []const u8 = "Way2Call";

// Wraps an audio device from the underlying environment.
pub const Device = struct {
    id: []const u8 = "",
    name: []const u8,
    direction: Direction = Direction.Render,
    is_default: bool = false,
    c_index: c_int = 0,

    // sample_rate: u32,
    // channel_count: u8,
};

pub const ConnectContext = struct {
    capture_device: Device = undefined,
    render_device: Device = undefined,
    is_active: bool = true,
    outer_thread: *Thread = undefined,
    render_thread: *Thread = undefined,
    capture_thread: *Thread = undefined,
    bytes_per_frame: u32 = 0,
    bytes_per_sample: u32 = 0,
    capture_callback: fn (context: *ConnectContext, frame_count_min: u32, frame_count_max: u32) void = undefined,

    // TODO(lbayes): Figure out how to make Sample Rate runtime variable.
    // buffer: anytype,

    pub fn close(self: *ConnectContext) void {
        self.is_active = false;
    }
};

// Returns true if the provided device direction is Render
pub fn isRenderDevice(d: Device) bool {
    return d.direction == Direction.Render;
}

// Returns true if the provided device direction is Capture
pub fn isCaptureDevice(d: Device) bool {
    return d.direction == Direction.Capture;
}

// Returns true if the provided device is a default device
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

const RingBuffer = @import("../ring_buffer.zig").RingBuffer;
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

pub const AudError = error{
    ArgumentError,
    DeviceNotFound,
    StreamOpenFailure,
    StreamStartFailure,
    StreamLayoutFailure,
    FormatFailure,
};

pub const PrioritizedSampleRates = [_]u32{
    44800,
    24000,
    44100,
    9600,
};

// Wraps an audio device from the underlying environment.
pub const Device = struct {
    id: []const u8 = "",
    name: []const u8,
    direction: Direction = Direction.Render,
    is_default: bool = false,
    platform_index: usize = 0,
    platform_device: usize = 0,

    // sample_rate: u32,
    // channel_count: u8,
};

// Pack the ConnectContext so that it can be sent back and forth between
// exernal C libraries.
pub const ConnectContext = packed struct {
    capture_device: *Device = undefined,
    render_device: *Device = undefined,
    is_active: bool = true,
    outer_thread: *Thread = undefined,
    render_thread: *Thread = undefined,
    capture_thread: *Thread = undefined,
    buffer: *RingBuffer(u8) = undefined,

    pub fn close(self: *ConnectContext) void {
        self.is_active = false;
    }
};

// Returns true if the provided device direction is Render
pub fn isRenderDevice(d: *Device) bool {
    return d.direction == Direction.Render;
}

// Returns true if the provided device direction is Capture
pub fn isCaptureDevice(d: *Device) bool {
    return d.direction == Direction.Capture;
}

// Returns true if the provided device is a default device
pub fn isDefaultDevice(d: *Device) bool {
    return d.is_default;
}

pub const DeviceFilter = fn (*Device) bool;

pub const Direction = enum(u8) {
    Capture = 0,
    Render,
};

pub const Role = enum(u8) {
    Communication = 0,
    Console,
    Media,
};

const std = @import("std");
const dev = @import("devices.zig");
const testing = std.testing;
const mem = std.mem;

// A Stitch will connect two audio devices by sending the output of a capture
// device to the input of a render device.
//
// One of the two devices is generally requested by selecting a default
// device, and the other is requested by matching on the device name and
// direction.
const Stitch = struct {
    capture: ?dev.AudioDevice = null,
    render: ?dev.AudioDevice = null,

    // pub fn init(capture: dev.Matcher, render: dev.Matcher) Stitch {
    // std.debug.print("------------------------------\n", .{});
    // std.debug.print("stitchCreate with:\n", .{});
    // std.debug.print("capture: {s}\n", .{capture});
    // std.debug.print("render: {s}\n", .{render});
    // const s = Stitch{};
    // return s;
    // }
};

// Create a new stitch instance using the provided Matchers.
//
// Platform audio devices may be
//
// For platforms that support device "roles" (e.g., Windows), the Matcher will
// use the provided value, defaulting to the Communication role.
pub fn stitchCreate(capture: Matcher, render: Matcher) i8 {}

pub export fn stitchCreateMaple() i8 {
    // const s = Stitch.init(DefaultCapture, .{
    // .direction = Direction.Render,
    // });
    return 0;
}

pub export fn stitchCreateWay2Call() i8 {
    return 0;
}

test "Stitch creation" {
    const s = Stitch{};
}

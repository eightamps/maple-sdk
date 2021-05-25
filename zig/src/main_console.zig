const std = @import("std");
const audio = @import("audio.zig");
const platform = @import("platform.zig");

const print = std.debug.print;

pub fn main() u8 {
    std.debug.print("Main Console loaded\n", .{});
    const info = platform.info();
    std.debug.print("audio.info(): {s}\n", .{info});

    const device = audio.api.getDefaultDevice() catch |err| {
        print(">>>>>>>>> ERROR: {s}\n", .{err});
        return 1;
    };

    std.debug.print("device name: {s}\n", .{device.name});

    return 0;
}

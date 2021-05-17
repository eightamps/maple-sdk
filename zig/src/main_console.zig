const std = @import("std");
const audio = @import("audio.zig");

pub fn main() u8 {
    std.debug.print("Main Console loaded\n", .{});
    const info = audio.info();
    std.debug.print("audio.info(): {s}\n", .{info});
    return 0;
}

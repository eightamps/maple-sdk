const std = @import("std");

pub export fn hello() i32 {
    std.debug.print("api_other:hello\n");
    return 0;
}

pub fn info() []const u8 {
    return "FROM OTHER";
}

pub const AudioDevice = struct {};

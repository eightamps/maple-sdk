const std = @import("std");

pub export fn hello() i32 {
    std.debug.print("api_nix:hello\n", .{});
    return 0;
}

pub fn info() []const u8 {
    return "FROM NIX";
}

pub const AudioDevice = struct {};

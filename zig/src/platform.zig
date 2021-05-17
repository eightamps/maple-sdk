const std = @import("std");
const target_file = switch (std.Target.current.os.tag) {
    .windows => "platform_win.zig",
    .linux => "platform_nix.zig",
    else => "platform_other.zig",
};
const platform = @import(target_file);

pub usingnamespace platform;

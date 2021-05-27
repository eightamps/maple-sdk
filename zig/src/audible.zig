const std = @import("std");
const target_file = switch (std.Target.current.os.tag) {
    .windows => "os/win/audible.zig",
    .linux => "os/nix/audible.zig",
    else => "os/nix/audible.zig",
};
const audible = @import(target_file);

pub usingnamespace audible;

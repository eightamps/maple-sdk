const std = @import("std");
const stitch = @import("stitch.zig");
const audio = @import("platform/nix_audio.zig");

usingnamespace stitch;

// var general_purpose_allocator = std.heap.GeneralPurposeAllocator(.{}){};
// const gpa = &general_purpose_allocator.allocator;

pub export fn add(a: i32, b: i32) i32 {
    std.debug.print("sdk_nix.add with: {} {}\n", .{ a, b });
    return a + b;
}

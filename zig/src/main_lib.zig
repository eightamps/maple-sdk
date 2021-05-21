const std = @import("std");
const platform = @import("platform.zig");
const audio = @import("audio.zig");

// NOTE(lbayes): This will take exported symbols from the platform-specific
// implementations and export them from this library entry point.
pub usingnamespace platform;

test "hello test" {
    // NOTE(lbayes): Zig is currently overwriting stderr output with test print
    // statements. Throw a carriage return to make this a little prettier.
    std.debug.print("\n", .{});
    const result = platform.hello();
    try std.testing.expectEqual(result, 0);
}

test "info test" {
    const value = platform.info();
    try std.testing.expectEqual(value, "LINUX");
}

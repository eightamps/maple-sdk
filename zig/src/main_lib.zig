const std = @import("std");
const platform = @import("platform.zig");
const audio = @import("audio.zig");

// NOTE(lbayes): This will take exported symbols from the platform-specific
// implementations and export them from this library entry point.
pub usingnamespace platform;

test "info test" {
    const value = platform.info();
    try std.testing.expectEqual(value, "LINUX");
}

const std = @import("std");
const platform = @import("platform.zig");
const audio = @import("audio.zig");
const hid_client = @import("hid_client.zig");

// NOTE(lbayes): This will take exported symbols from the platform-specific
// implementations and export them from this library entry point.
pub usingnamespace platform;
pub usingnamespace hid_client;

test "info test" {
    const value = platform.info();
    try std.testing.expectEqual(value, "LINUX");
}

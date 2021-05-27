const std = @import("std");
const audible = @import("audible.zig");
const hid_client = @import("hid_client.zig");

// NOTE(lbayes): This will take exported symbols from the platform-specific
// implementations and export them from this library entry point.
pub usingnamespace audible;
pub usingnamespace hid_client;

test "info test" {
    const value = audible.info();
    try std.testing.expectEqual(value, "LINUX");
}

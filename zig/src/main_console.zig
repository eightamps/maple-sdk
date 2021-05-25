const std = @import("std");
const platform = @import("platform.zig");

const print = std.debug.print;

pub fn main() u8 {
    std.debug.print("Main Console loaded\n", .{});

    var api = platform.AudioApi{};

    const device = api.getDefaultDevice() catch |err| {
        print(">>>>>>>>> ERROR: {s}\n", .{err});
        return 1;
    };
    defer api.deinit();

    std.debug.print("device name: {s}\n", .{device.name});

    return 0;
}

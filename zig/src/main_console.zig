const std = @import("std");
const audible = @import("audible.zig");
const phony_client = @import("phony_client.zig");

const print = std.debug.print;

usingnamespace phony_client;

pub fn main() !u8 {
    std.debug.print("Main Console loaded\n", .{});

    // const client = try PhonyClient.open();
    // defer client.close();

    var api = audible.AudioApi{};
    const device = api.getDefaultDevice() catch |err| {
        print(">>>>>>>>> ERROR: {s}\n", .{err});
        return 1;
    };
    defer api.deinit();
    std.debug.print("device name: {s}\n", .{device.name});

    return 0;
}

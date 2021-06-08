const std = @import("std");
const aud = @import("aud.zig");
const phony_client = @import("phony_client.zig");

const ascii = std.ascii;
const print = std.debug.print;
const heap = std.heap;

usingnamespace phony_client;

pub fn main() !u8 {
    print("--------------------------------------------------\n", .{});
    print("Main Console loaded\n", .{});

    // Create the configured allocator
    var gpa = heap.GeneralPurposeAllocator(.{}){};
    defer {
        const leaked = gpa.deinit();
        if (leaked) {
            @panic("MEMORY LEAK DETECTED");
        }
    }

    // Initialize the native audio devices api (names need to change)
    var audio_api = try aud.NativeDevices.init(&gpa.allocator);
    defer audio_api.deinit();

    var buf: [aud.MAX_DEVICE_COUNT]aud.Device = undefined;
    const devices = try audio_api.getRenderDevices(&buf);

    for (devices) |dev| {
        print("Dev: {s} {s}\n", .{ dev.name, dev.id });

        if (ascii.indexOfIgnoreCasePos(dev.name, 0, "MM-1 Analog Stereo") != null) {
            print("Pushed preferred device with name: [{s}] and id: [{s}]\n", .{ dev.name, dev.id });
            try audio_api.pushPreferredNativeId(dev.id, aud.Direction.Render);
        }
    }

    // Get a default capture device
    const render_device = try audio_api.getDefaultRenderDevice();
    std.debug.print("render_device.name: {s}\n", .{render_device.name});

    return 0;
}

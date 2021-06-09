const aud = @import("aud.zig");
const phony_client = @import("phony_client.zig");
const std = @import("std");

const ascii = std.ascii;
const print = std.debug.print;
const heap = std.heap;
const time = std.time;

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

    // Could provide UI to let users make a ranked list of preferred devices
    for (devices) |dev| {
        print("Dev: {s} {s}\n", .{ dev.name, dev.id });

        if (ascii.indexOfIgnoreCasePos(dev.name, 0, "MM-1 Analog Stereo") != null) {
            print("Pushed preferred device with name: [{s}] and id: [{s}]\n", .{ dev.name, dev.id });
            try audio_api.pushPreferredNativeId(dev.id, aud.Direction.Render);
        }
    }

    // Get default render device
    const render = try audio_api.getDefaultRenderDevice();
    print("render.name: {s}\n", .{render.name});

    // Get default capture device
    const capture = try audio_api.getDefaultCaptureDevice();
    print("capture.name: {s}\n", .{capture.name});

    // Connect the device pair
    const to_phone = try audio_api.connect(render, capture);
    const from_phone = try audio_api.connect(render, capture);

    time.sleep(1 * time.ns_per_ms);
    to_phone.stop();
    from_phone.stop();

    print("Main Console exiting now\n", .{});
    return 0;
}

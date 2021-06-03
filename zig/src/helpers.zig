const std = @import("std");

const expectEqual = std.testing.expectEqual;

const FakeDirection = enum(u8) {
    Render,
    Capture,
};

const FakeDevice = struct {
    id: u32,
    name: []const u8,
    direction: FakeDirection,
    is_default: bool = false,
};

var fake_devices = [_]FakeDevice{
    .{
        .id = 0,
        .name = "abcd",
        .direction = FakeDirection.Render,
    },
    .{
        .id = 1,
        .name = "efgh",
        .direction = FakeDirection.Capture,
    },
    .{
        .id = 2,
        .name = "ijkl",
        .direction = FakeDirection.Render,
        .is_default = true,
    },
    .{
        .id = 3,
        .name = "mnop",
        .direction = FakeDirection.Render,
    },
    .{
        .id = 4,
        .name = "qrst",
        .direction = FakeDirection.Render,
    },
    .{
        .id = 5,
        .name = "uvwx",
        .direction = FakeDirection.Capture,
        .is_default = true,
    },
};

fn isRenderDevice(d: FakeDevice) bool {
    return d.direction == FakeDirection.Render;
}

fn isCaptureDevice(d: FakeDevice) bool {
    return d.direction == FakeDirection.Capture;
}

fn isDefaultDevice(d: FakeDevice) bool {
    return d.is_default;
}

fn isFalse(d: FakeDevice) bool {
    return false;
}

var renderFilter = .{
    isRenderDevice,
};

var captureFilter = .{
    isCaptureDevice,
};

var defaultCaptureFilter = .{
    isDefaultDevice,
    isCaptureDevice,
};

var alwaysNullFilter = .{
    isFalse,
};

pub fn filterItems(comptime T: type, input: []T, output: []T, filters: []fn (d: T) bool) []T {
    var input_index: u32 = 0;
    var output_index: u32 = 0;

    outer: while (input_index < input.len) : (input_index += 1) {
        const device = input[input_index];
        // If any of the provided filters return true, skip this entry.
        for (filters) |filter| {
            if (!filter(device)) {
                continue :outer;
            }
        }
        output[output_index] = input[input_index];
        output_index += 1;
    }

    return output[0..output_index];
}

pub fn firstItemMatching(comptime T: type, input: []T, filters: []fn (d: T) bool) ?T {
    var input_index: u32 = 0;

    outer: while (input_index < input.len) : (input_index += 1) {
        var device = input[input_index];
        for (filters) |filter| {
            if (!filter(device)) {
                continue :outer;
            }
        }

        // If each of the provided filters return true, return this entry.
        return device;
    }

    return null;
}

test "filterItems gets a subset" {
    var buffer: [10]FakeDevice = undefined;

    const results = filterItems(FakeDevice, &fake_devices, &buffer, &renderFilter);
    try expectEqual(results.len, 4);
    try expectEqual(results[0].name, "abcd");
    try expectEqual(results[1].name, "ijkl");
    try expectEqual(results[2].name, "mnop");
    try expectEqual(results[3].name, "qrst");
}

test "filterItems with empty result" {
    var buffer: [10]FakeDevice = undefined;

    const results = filterItems(FakeDevice, &fake_devices, &buffer, &alwaysNullFilter);
    try expectEqual(results.len, 0);
}

test "filterItems with empty input" {
    var buffer: [10]FakeDevice = undefined;

    const results = filterItems(FakeDevice, &buffer, &buffer, &alwaysNullFilter);
    try expectEqual(results.len, 0);
}

test "filterItems gets a different subset" {
    var buffer: [10]FakeDevice = undefined;

    const results = filterItems(FakeDevice, &fake_devices, &buffer, &captureFilter);
    try expectEqual(results.len, 2);
    try expectEqual(results[0].name, "efgh");
    try expectEqual(results[1].name, "uvwx");
}

test "firstItemMatching gets the first match" {
    const result = try firstItemMatching(FakeDevice, &fake_devices, &defaultCaptureFilter) orelse error.Fail;

    try expectEqual(result.id, 5);
    try expectEqual(result.name, "uvwx");
}

test "firstitemMatching gets null result" {
    const result = firstItemMatching(FakeDevice, &fake_devices, &alwaysNullFilter);
    try expectEqual(result, null);
}

const std = @import("std");

const Allocator = std.mem.Allocator;
const DefaultPrng = std.rand.DefaultPrng;
const Thread = std.Thread;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const io = std.io;
const mem = std.mem;
const print = std.debug.print;
const talloc = std.testing.allocator;
const time = std.time;

const WriteError = error{};

pub fn RingBuffer(comptime T: type, comptime capacity: usize) type {
    return struct {
        const Self = @This();
        const RBT = RingBuffer(T, capacity);
        const Writer = io.Writer(*Self, WriteError, writeF32);

        allocator: *Allocator,
        buffer: []T = undefined,
        write_index: usize = 0,
        read_index: usize = 0,

        byte_size: usize = @sizeOf(T),

        fn writeF32(self: *Self, bytes: []const u8) WriteError!usize {
            print("writeFn called with: {d} and {d}\n", .{ bytes.len, bytes[0] });
            print("byte_size: {d}\n", .{self.byte_size});

            var item_bytes = [4]u8{
                bytes[0],
                bytes[1],
                bytes[2],
                bytes[3],
            };

            var foo = mem.bytesAsValue(T, &item_bytes);
            print("to_bytes: {e}\n", .{foo.*});
            return 0;
        }

        inline fn incrementIndex(index: usize) usize {
            return (index +% 1) % capacity;
        }

        pub fn init(a: *Allocator) !*Self {
            var instance = try a.create(RBT);
            var buffer = try a.alloc(T, capacity);
            instance.* = RBT{
                .allocator = a,
                .buffer = buffer,
            };
            return instance;
        }

        pub fn deinit(self: *Self) void {
            self.allocator.free(self.buffer);
            self.allocator.destroy(self);
        }

        pub fn push(self: *Self, item: T) void {
            const index = self.write_index;
            const next_index = incrementIndex(index);
            if (next_index == self.read_index) {
                self.read_index = incrementIndex(next_index);
            }
            self.buffer[index] = item;
            self.write_index = next_index;
        }

        pub fn hasNext(self: *Self) bool {
            return self.getDistance() > 0;
        }

        pub fn getDistance(self: *Self) usize {
            return (self.write_index -% self.read_index) % capacity;
        }

        pub fn pop(self: *Self) T {
            var index = self.read_index;
            var result = self.buffer[index];
            self.read_index = incrementIndex(index);
            return result;
        }

        pub fn writer(self: *Self) Writer {
            return Writer{
                .context = self,
            };
        }
    };
}

test "RingBuffer push loops around buffer" {
    print("\n\n", .{});
    const rb = try RingBuffer(u8, 4).init(talloc);
    defer rb.deinit();

    try expectEqual(rb.getDistance(), 0);
    rb.push('a');
    try expectEqual(rb.getDistance(), 1);
    rb.push('b');
    try expectEqual(rb.getDistance(), 2);
    rb.push('c');
    try expectEqual(rb.getDistance(), 3);
    rb.push('d');
    try expectEqual(rb.getDistance(), 3);

    try expectEqual(rb.pop(), 'b');
    try expectEqual(rb.getDistance(), 2);
    try expectEqual(rb.pop(), 'c');
    try expectEqual(rb.getDistance(), 1);
    try expectEqual(rb.pop(), 'd');
    try expectEqual(rb.getDistance(), 0);

    rb.push('f');
    rb.push('g');
    rb.push('h');
    rb.push('i');
    rb.push('j');
    rb.push('k');

    try expectEqual(rb.getDistance(), 3);
    try expectEqual(rb.pop(), 'i');
    try expectEqual(rb.getDistance(), 2);
    try expectEqual(rb.pop(), 'j');
    try expectEqual(rb.getDistance(), 1);
    try expectEqual(rb.pop(), 'k');
    try expectEqual(rb.getDistance(), 0);
}

// A type of RingBuffer configured for f32 audio samples at the provided sample
// rate and seconds to buffer.
pub fn AudioSampleRingBuffer(comptime sample_rate: u32, comptime seconds_to_buffer: usize) type {
    return RingBuffer(f32, sample_rate * seconds_to_buffer);
}

// A type of RingBuffer used by tests with a buffer duration of of 44800
// samples x 5 seconds.
const FourtyFourEightByFiveSeconds = AudioSampleRingBuffer(44800, 5);

const TestBufContext = struct {
    buffer: *FourtyFourEightByFiveSeconds,
    should_exit: bool = false,
    sample_rate: u32 = 44800,
    read_sample_count: u128 = 0,
};

const TimeSamples: usize = 10000;
const SamplePeriodNs: u64 = 1_000_000_000 / 44_800;

fn testThreadWriter(ctx: *TestBufContext) u8 {
    var last_value: f32 = 0.0;
    var sample_count_per_loop: u64 = 1000;
    var ns_per_loop: u64 = SamplePeriodNs * sample_count_per_loop;
    while (!ctx.should_exit) {
        const start = time.nanoTimestamp();
        var index: usize = 0;
        while (index < sample_count_per_loop) {
            ctx.buffer.push(last_value);
            index += 1;
        }

        var duration = time.nanoTimestamp() - start;
        if (duration < ns_per_loop) {
            time.sleep(ns_per_loop - @intCast(u64, duration));
        } else {
            // TODO(lbayes): What is periodically slowing down our writer?
            print("WARNING: Fake Writer SLOWED DOWN FOR {d}ns\n", .{duration});
        }
    }
    return 0;
}

fn testThreadReader(ctx: *TestBufContext) u8 {
    var read_sample_count: usize = 0;
    var sample_seconds: u128 = 0;
    var last_sample_seconds: u128 = 0;
    var start: i128 = 0;
    var duration: i128 = 0;
    while (!ctx.should_exit) {
        read_sample_count += ctx.buffer.getDistance();
        if (read_sample_count > 0) {
            ctx.read_sample_count = read_sample_count;

            sample_seconds = read_sample_count / 44800;
            if (sample_seconds != last_sample_seconds) {
                duration = time.milliTimestamp() - start;
                last_sample_seconds = sample_seconds;
                start = time.milliTimestamp();
            }

            while (ctx.buffer.hasNext()) {
                _ = ctx.buffer.pop();
            }
            time.sleep(1000); // ns
        }
    }
    return 0;
}

test "RingBuffer works for audio devices" {
    var buf = try FourtyFourEightByFiveSeconds.init(talloc);
    defer buf.deinit();

    var context = TestBufContext{ .buffer = buf };
    const reader = try Thread.spawn(testThreadReader, &context);
    const writer = try Thread.spawn(testThreadWriter, &context);

    while (context.read_sample_count < 100) {
        time.sleep(1 * time.ns_per_ms);
    }

    context.should_exit = true;
    writer.wait();
    reader.wait();
}

test "RingBuffer with io.Writer" {
    var buf = try FourtyFourEightByFiveSeconds.init(talloc);
    defer buf.deinit();

    const value: f32 = 0.5;
    var bytes = mem.asBytes(&value);

    var writer = buf.writer();
    _ = try writer.write(bytes);
}

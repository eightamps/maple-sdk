const Timer = @import("timer.zig").Timer;
const std = @import("std");

const Allocator = std.mem.Allocator;
const DefaultPrng = std.rand.DefaultPrng;
const Thread = std.Thread;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const io = std.io;
const math = std.math;
const mem = std.mem;
const print = std.debug.print;
const talloc = std.testing.allocator;
const time = std.time;

const WriteError = error{};

pub fn RingBuffer(comptime T: type) type {
    return struct {
        const Self = @This();

        allocator: *Allocator,
        capacity: usize = 0,
        buffer: []T = undefined,
        write_index: usize = 0,
        read_index: usize = 0,

        pub fn init(a: *Allocator, capacity: usize) !*Self {
            var instance = try a.create(Self);
            var buffer = try a.alloc(T, capacity);
            instance.* = Self{
                .allocator = a,
                .buffer = buffer,
                .capacity = capacity,
            };
            return instance;
        }

        pub fn deinit(self: *Self) void {
            self.allocator.free(self.buffer);
            self.allocator.destroy(self);
        }

        inline fn incrementIndex(self: *Self, index: usize) usize {
            return (index +% 1) % self.capacity;
        }

        pub fn push(self: *Self, item: T) void {
            const index = self.write_index;
            const next_index = self.incrementIndex(index);
            if (next_index == self.read_index) {
                self.read_index = self.incrementIndex(next_index);
            }
            self.buffer[index] = item;
            self.write_index = next_index;
        }

        pub fn hasNext(self: *Self) bool {
            return self.getDistance() > 0;
        }

        pub fn getDistance(self: *Self) usize {
            return (self.write_index -% self.read_index) % self.capacity;
        }

        pub fn pop(self: *Self) T {
            var index = self.read_index;
            var result = self.buffer[index];
            self.read_index = self.incrementIndex(index);
            return result;
        }
    };
}

test "RingBuffer push loops around buffer" {
    const rb = try RingBuffer(u8).init(talloc, 4);
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

const AudioSampleBuffer = RingBuffer(f32);
const AudioSampleCapacity = 44800 * 5;

const TestBufContext = struct {
    buffer: *AudioSampleBuffer,
    timer: *Timer,
    should_exit: bool = false,
    sample_rate: u32 = 44800,
    read_sample_count: i128 = 0,
    read_sample_sum: f32 = 0.0,
};

// const SamplePeriodNs: u64 = 1_000_000_000 / 44_800; // 22,321
// const SampleCountPerLoop: u64 = @divFloor(MinSleepNs, SamplePeriodNs) + SamplePeriodNs;
const NsPerSec = 1_000_000_000;
const MinSleepNs = 60_000; // Observed ~57,000 ns on my workstation
const NsCostPerSample = 1000;
const MaxSampleCountPerSleep = MinSleepNs / NsCostPerSample;

const TestValues = struct {
    samples_per_sleep: i128 = 0,
};

fn getSamplesPerSleep(sample_rate: i128) i128 {
    const period_ns = @divFloor(NsPerSec, sample_rate); // 22,321ns period for 44,800Hz
    return @divFloor(MinSleepNs, period_ns) + 1;
}

fn testThreadWriter(ctx: *TestBufContext) u8 {
    var samples_per_sleep = getSamplesPerSleep(ctx.sample_rate);
    // print("SAMPLES_PER_SLEEP: {d}\n", .{samples_per_sleep});

    var last_value: f32 = 0.0;
    while (!ctx.should_exit) {
        var index: usize = 0;
        while (index < samples_per_sleep) {
            // print("buffer.push: {e}\n", .{last_value});
            ctx.buffer.push(last_value);
            last_value += 0.0001;
            index += 1;
        }

        _ = time.sleep(1); // Actually ~60k ns on workstation
    }
    return 0;
}

fn testThreadReader(ctx: *TestBufContext) u8 {
    var samples: usize = 0;
    var sample: f32 = 0.0;
    while (!ctx.should_exit) {
        while (ctx.buffer.hasNext()) {
            ctx.read_sample_sum += ctx.buffer.pop();
            ctx.read_sample_count += 1;
        }
        _ = time.sleep(1); // Actually ~60k ns on workstation
    }

    return 0;
}

test "RingBuffer works for audio devices" {
    var buf = try AudioSampleBuffer.init(talloc, AudioSampleCapacity);
    defer buf.deinit();

    var timer = try Timer.init(talloc);
    defer timer.deinit();
    timer.warnings = false;

    var context = TestBufContext{
        .buffer = buf,
        .timer = timer,
    };

    const reader = try Thread.spawn(testThreadReader, &context);
    const writer = try Thread.spawn(testThreadWriter, &context);

    // timer.start();
    time.sleep(1 * time.ns_per_ms);

    // TODO(lbayes): More investigation needed. When we drive the above sample
    // for 3000ms, we get ~56000 +/-200 samples per second of time
    //
    // const duration_s = @divFloor(timer.stop(), time.ns_per_s);
    // const samples_per_second = @divFloor(context.read_sample_count, duration_s);
    // print("\n\n", .{});
    // print("samples per second: {d}\n", .{samples_per_second});
    //
    // print("sample_count: {d}\n", .{context.read_sample_count});
    // print("sample_sum: {d}\n", .{context.read_sample_sum});

    context.should_exit = true;
    writer.wait();
    reader.wait();
}

fn createSinePoint(time_index: u32, freq_hz: u32, sample_rate: u32) f32 {
    return @sin(math.pi * @as(f32, 2.0) * @intToFloat(f32, time_index) * @intToFloat(f32, freq_hz) / @intToFloat(f32, sample_rate));
}

fn perfThreadWriter(context: *TestBufContext) u8 {
    var sample_count: u32 = 100;
    var sample_index: u32 = 0;

    const frame_ns: u64 = @divFloor(1_000_000_000, (context.sample_rate * sample_count));

    var sample: f32 = 0.0; // = createSinePoint(0, context.sample_rate, context.sample_rate);
    var duration_ns: u64 = undefined;
    var start_sleep_ns: u64 = undefined;
    var sleep_duration_ns: u64 = undefined;
    var loop_start: u64 = undefined;
    var index: u32 = 0;
    var tone_hz: u32 = 700;

    while (!context.should_exit) {
        loop_start = @intCast(u64, time.nanoTimestamp());

        // Stuff some samples into the buffer
        while (sample_index < sample_count) {
            sample = createSinePoint(index, tone_hz, context.sample_rate);
            context.buffer.push(sample);
            sample_index += 1;
        }
        sample_index = 0;

        // Sleep so that we only write at sample_rate hz.
        duration_ns = @intCast(u64, time.nanoTimestamp()) - loop_start;
        start_sleep_ns = @intCast(u64, time.nanoTimestamp());

        if (frame_ns > duration_ns) {
            // print("SLEEPER: {d} {d} {d}\n", .{ frame_ns, duration_ns, frame_ns - duration_ns });
            // time.sleep(100_000); // fraction of a millisecond
            time.sleep(@intCast(u64, (frame_ns - duration_ns)));
            sleep_duration_ns = @intCast(u64, time.nanoTimestamp()) - start_sleep_ns;
            print("Writer sleeping for: {d}\n", .{sleep_duration_ns});
        }
        index +%= 1 % (context.sample_rate * 3);
    }

    print("perf writer exiting!\n", .{});
    return 0;
}

fn perfThreadReader(context: *TestBufContext) u8 {
    var buf = context.buffer;
    var sample: f32 = 0.0;
    var sample_count: u32 = 0;
    var start: i128 = time.nanoTimestamp();
    var duration: i128 = undefined;

    while (!context.should_exit) {
        duration = time.nanoTimestamp() - start;
        if (duration > time.ns_per_s) {
            print("Second has passed with: {d}ms\n", .{@divFloor(duration, 1_000_000)});
            duration = 0;
            start = time.nanoTimestamp();
        }

        while (buf.hasNext()) {
            sample = buf.pop();
            // print("SAMPLE: {e}\n", .{sample});
            sample_count +%= 1;
            // if (sample_count % context.sample_rate == 0) {
            // print("YOOOOOOOOOO {d}\n", .{sample_count});
            // }
        }
        time.sleep(100_000); // fraction of a millisecond
    }

    print("perf reader exiting!\n", .{});
    return 0;
}

test "RingBuffer Perf Test" {
    print("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", .{});
    var buf = try AudioSampleBuffer.init(talloc, AudioSampleCapacity);
    defer buf.deinit();

    var timer = try Timer.init(talloc);
    defer timer.deinit();
    timer.warnings = false;

    var context = TestBufContext{
        .buffer = buf,
        .timer = timer,
    };

    const writer = try Thread.spawn(perfThreadWriter, &context);
    const reader = try Thread.spawn(perfThreadReader, &context);

    // Increase sleep time to 10 seconds to see writer/reader interaction
    time.sleep(1 * time.ns_per_ms);
    context.should_exit = true;

    writer.wait();
    reader.wait();
    print("------------------------------\n", .{});
}

test "RingBuffer -> timer is available" {
    const t = try Timer.init(talloc);
    defer t.deinit();
}

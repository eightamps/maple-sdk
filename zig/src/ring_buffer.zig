const std = @import("std");

const Allocator = std.mem.Allocator;
const Thread = std.Thread;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const print = std.debug.print;
const talloc = std.testing.allocator;
const time = std.time;

pub fn RingBuffer(comptime T: type, comptime capacity: usize) type {
    return struct {
        const Self = @This();
        const RBT = RingBuffer(T, capacity);

        allocator: *Allocator,
        buffer: []T = undefined,
        capacity: usize = capacity,
        write_index: usize = 0,
        read_index: usize = 0,

        pub fn init(a: *Allocator) !*Self {
            var instance = try a.create(RBT);
            var buffer = try a.alloc(T, capacity);
            instance.* = RBT{
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

        fn getDistance(self: *Self) usize {
            if (self.write_index < self.capacity) {
                return self.write_index - self.read_index;
            } else if (self.write_index - self.read_index >= self.capacity) {
                return self.capacity;
            } else {
                return (self.write_index - self.read_index) % self.capacity;
            }
        }

        pub fn push(self: *Self, item: T) void {
            self.buffer[self.write_index % self.capacity] = item;
            self.write_index += 1;
        }

        pub fn hasNext(self: *Self) bool {
            return self.getDistance() > 0;
        }

        pub fn popCountRemaining(self: *Self) usize {
            return self.getDistance();
        }

        pub fn pop(self: *Self) T {
            var w_index = self.write_index;

            if (w_index - self.read_index >= self.capacity) {
                self.read_index = w_index - self.capacity;
            }
            const result = self.buffer[self.read_index % self.capacity];

            self.read_index += 1;
            return result;
        }
    };
}

test "RingBuffer push loops around buffer" {
    print("\n\n", .{});
    const rb = try RingBuffer(u8, 4).init(talloc);
    defer rb.deinit();

    try expectEqual(rb.popCountRemaining(), 0);
    rb.push('a');
    try expectEqual(rb.popCountRemaining(), 1);
    rb.push('b');
    try expectEqual(rb.popCountRemaining(), 2);
    rb.push('c');
    try expectEqual(rb.popCountRemaining(), 3);
    rb.push('d');
    try expectEqual(rb.popCountRemaining(), 4);
    rb.push('e');
    try expectEqual(rb.popCountRemaining(), 4);

    try expectEqual(rb.pop(), 'b');
    try expectEqual(rb.popCountRemaining(), 3);
    try expectEqual(rb.pop(), 'c');
    try expectEqual(rb.popCountRemaining(), 2);
    try expectEqual(rb.pop(), 'd');
    try expectEqual(rb.popCountRemaining(), 1);
    try expectEqual(rb.pop(), 'e');
    try expectEqual(rb.popCountRemaining(), 0);

    rb.push('f');
    rb.push('g');

    rb.push('h');
    rb.push('i');
    rb.push('j');
    rb.push('k');
    try expectEqual(rb.popCountRemaining(), 4);
    try expectEqual(rb.pop(), 'h');
    try expectEqual(rb.popCountRemaining(), 3);
    try expectEqual(rb.pop(), 'i');
    try expectEqual(rb.popCountRemaining(), 2);
    try expectEqual(rb.pop(), 'j');
    try expectEqual(rb.popCountRemaining(), 1);
    try expectEqual(rb.pop(), 'k');
    try expectEqual(rb.popCountRemaining(), 0);
}

const TestRingBufferCapacity = 2048000;
const TestRingBuffer = RingBuffer(f32, TestRingBufferCapacity);

const TestBufContext = struct {
    buffer: *TestRingBuffer,
    should_exit: bool = false,
    should_write: bool = false,
    read_complete: bool = true,
    read_sample_count: u128 = 0,
};

const TimeSamples: usize = 10000;

const SamplePeriodNs: u64 = 1_000_000_000 / 44_800;

fn testWriter(ctx: *TestBufContext) u8 {
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

fn testReader(ctx: *TestBufContext) u8 {
    var read_sample_count: usize = 0;
    var sample_seconds: u128 = 0;
    var last_sample_seconds: u128 = 0;
    var start: i128 = 0;
    var duration: i128 = 0;
    while (!ctx.should_exit) {
        read_sample_count += ctx.buffer.popCountRemaining();
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

test "Devices buffer" {
    var buf = try TestRingBuffer.init(talloc);
    // try buf.ensureCapacity(128);
    defer buf.deinit();

    var context = TestBufContext{ .buffer = buf };
    const reader = try Thread.spawn(testReader, &context);
    const writer = try Thread.spawn(testWriter, &context);

    context.should_write = true;

    while (context.read_sample_count < 100) {
        time.sleep(1 * time.ns_per_ms);
    }

    context.should_exit = true;
    writer.wait();
    reader.wait();

    // buf.writeassumecapacity(&[_]f32{
    //     0.1,
    //     0.2,
    //     0.3,
    //     0.4,
    //     0.5,
    // });

    // var result: ?f32 = undefined;

    // const byte_count = buf.readableLength();
    // const bytes = buf.readableSlice(0);
    // print("BYTES: {any}\n", .{bytes});

    // while (buf.readableLength() > 0) {
    //     result = buf.readItem();
    //     print("RESULT: {d}\n", .{result});
    // }
    // result = buf.readItem();
    // print("RESULT: {d}\n", .{result});
    // result = buf.readItem();
    // print("RESULT: {d}\n", .{result});
    // result = buf.readItem();
    // print("RESULT: {d}\n", .{result});
    // result = buf.readItem();
    // print("RESULT: {d}\n", .{result});
    // result = buf.readItem();
    // print("RESULT: {d}\n", .{result});

}

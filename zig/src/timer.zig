const std = @import("std");

const Allocator = std.mem.Allocator;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const expectEqualStrings = std.testing.expectEqualStrings;
const print = std.debug.print;
const talloc = std.testing.allocator;
const time = std.time;

pub const Timer = struct {
    const Self = @This();

    allocator: *Allocator,
    warnings: bool = true,
    margin_ns: i128 = 10000,
    start_ns: i128 = 0,
    end_ns: i128 = 0,
    duration_ns: i128 = 0,

    // Allocate a timer instance to hold state for various time-related
    // features.
    pub fn init(a: *Allocator) !*Timer {
        var instance = try a.create(Timer);
        instance.* = Timer{
            .allocator = a,
        };
        return instance;
    }

    // Allocat a timer instance that will not emit any warnings to std.debug.
    pub fn initQuiet(a: *Allocator) !*Timer {
        var instance = try init(a);
        instance.warnings = false;
        return instance;
    }

    // Deinitialize the timer and free any allocations.
    pub fn deinit(self: *Self) void {
        self.allocator.destroy(self);
    }

    // Reset timer state, usually called from start()
    pub fn reset(self: *Self) void {
        self.start_ns = 0;
        self.end_ns = 0;
        self.duration_ns = 0;
    }

    // Start a timer and hold the value waiting for a subsequent call to stop.
    pub fn start(self: *Self) void {
        self.reset();
        self.start_ns = time.nanoTimestamp();
    }

    // Stop a previosly set timer and return the duration in nanoseconds.
    pub fn stop(self: *Self) i128 {
        self.end_ns = time.nanoTimestamp();
        if (self.start_ns == 0) {
            if (self.warnings) {
                print("WARNING: timer.stop() called without an associated call to start(), returning 0 now.\n", .{});
            }
            return 0;
        }
        self.duration_ns = self.end_ns - self.start_ns;

        return self.duration_ns;
    }

    // Sleep for the provided nanoseconds, but return the duration in ns for
    // how long the sleep actually blocked.
    //
    // NOTE(lbayes): This additional work costs roughly 1000ns with warnings
    // off, and about 10k ns with warnings on.
    pub fn sleep(self: *Self, duration_ns: u64) i128 {
        const start_ns = time.nanoTimestamp();
        time.sleep(duration_ns);
        const actual_duration_ns = time.nanoTimestamp() - start_ns;

        if (self.warnings and actual_duration_ns > duration_ns + self.margin_ns) {
            print("WARNING: timer.sleep ran much longer ({}ns OVER) than requested ({}ns plus margin_ns {}ns)\n", .{ actual_duration_ns - duration_ns, duration_ns, self.margin_ns });
            print("NOTE: Either increase t.margin_ns or set t.warnings = false to silence this message\n", .{});
        }

        return actual_duration_ns;
    }
};

test "Timer is instantiable" {
    const t = try Timer.initQuiet(talloc);
    defer t.deinit();
}

test "timer.stop returns 0 if never started" {
    const t = try Timer.initQuiet(talloc);
    defer t.deinit();

    const duration_ns = t.stop();
    try expectEqual(duration_ns, 0);
}

test "timer start and stop" {
    const t = try Timer.initQuiet(talloc);
    defer t.deinit();

    t.start();
    const duration_ns = t.stop();

    try expect(duration_ns < 10_000); // Actually ~500 on my linux box
}

test "timer actual sleep duration" {
    const t = try Timer.initQuiet(talloc);
    defer t.deinit();

    t.start();
    const actual_sleep_ns = t.sleep(1);
    const duration_ns = t.stop();

    try expect(actual_sleep_ns > 10_000); // Actually ~57k on my linux box
    try expect(duration_ns > 10_000); // Actually ~58k on my linux boxZ
}

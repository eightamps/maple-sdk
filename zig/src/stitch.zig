const std = @import("std");
const aud = @import("./aud.zig");

const Allocator = std.mem.Allocator;
const Device = aud.Device;
const Direction = aud.Direction;
const NativeDevices = aud.NativeDevices;
const Thread = std.Thread;
const talloc = std.testing.allocator;
const time = std.time;

// Must be unbound for threads to work with it.
fn work(worker: *StitchWorker) u8 {
    while (worker.is_active) {
        time.sleep(10 * time.ns_per_ms);
    }
    return 0;
}

const StitchWorker = struct {
    is_active: bool = true,
    thread: *Thread = undefined,

    fn kill(self: *StitchWorker) void {
        self.is_active = false;
    }
};

pub const Stitch = struct {
    allocator: *Allocator,

    pub fn init(a: *Allocator) !*Stitch {
        var instance = try a.create(Stitch);
        instance.* = Stitch{
            .allocator = a,
        };
        return instance;
    }

    pub fn deinit(self: *Stitch) void {
        self.allocator.destroy(self);
    }

    pub fn connect(self: *Stitch, render: Device, capture: Device) !*StitchWorker {
        var worker = StitchWorker{};
        worker.thread = try Thread.spawn(work, &worker);
        return &worker;
    }
};

test "Stitch is instantiable" {
    const stitch = try Stitch.init(talloc);
    defer stitch.deinit();

    const devices = try NativeDevices.init(talloc);
    defer devices.deinit();

    const capture = try devices.getDefaultCaptureDevice();
    const render = try devices.getDefaultRenderDevice();
    // const render = try devices.getDeviceWithName("ASI Telephone", Direction.Render);

    const w = try stitch.connect(render, capture);
    time.sleep(1000 * time.ns_per_ms);
    w.kill();
}

const std = @import("std");
const common = @import("../aud_common.zig");

const sio = @cImport({
    @cInclude("soundio/soundio.h");
});

usingnamespace sio;

const Allocator = std.mem.Allocator;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const heap = std.heap;
const mem = std.mem;
const print = std.debug.print;

const DEFAULT_DEVICE_NAME = "[unknown]";

fn failed(status: c_int) bool {
    return status != 0;
}

fn index_failed(status: c_int) bool {
    return status < 0;
}

pub fn info() []const u8 {
    return "LINUX";
}

pub const Device = struct {
    direction: common.Direction,
    allocator: *Allocator,
    name: []const u8 = DEFAULT_DEVICE_NAME,

    pub fn deinit(self: *Device) void {
        self.allocator.destroy(self);
    }
};

pub const Devices = struct {
    allocator: *Allocator,
    soundio: *SoundIo,
    ring_buffer: *SoundIoRingBuffer = undefined,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);

        const soundio = soundio_create();
        if (soundio == null) {
            print("soundio failed to allocate\n", .{});
            return error.Fail;
        }
        print("soundio successfully created context\n", .{});

        {
            const status = soundio_connect_backend(soundio, @intToEnum(sio.SoundIoBackend, sio.SoundIoBackendPulseAudio));
            if (failed(status)) {
                print("soundio failed to connect to backend\n", .{});
                // Free the soundio object before exiting
                soundio_destroy(soundio);
                return error.Fail;
            }
        }

        sio.soundio_flush_events(soundio);
        print("soundio flushed events\n", .{});

        instance.* = Devices{
            .allocator = a,
            .soundio = soundio,
        };

        return instance;
    }

    pub fn deinit(self: *Devices) void {
        print("AudibleApi.deinit called\n", .{});
        soundio_destroy(self.soundio);
        self.allocator.destroy(self);
    }

    pub fn getDevice(self: *Devices, matcher: common.Matcher) !*Device {
        std.debug.print("Linux getDevice()\n", .{});
        switch (matcher.direction) {
            common.Direction.Render => {
                return self.getRenderDevice(matcher);
            },
            common.Direction.Capture => {
                return self.getCaptureDevice(matcher);
            },
        }
    }

    fn createDevice(self: *Devices, direction: common.Direction) !*Device {
        const device = try self.allocator.create(Device);
        device.* = Device{
            .allocator = self.allocator,
            .direction = direction,
        };
        return device;
    }

    pub fn getRenderDevice(self: *Devices, matcher: common.Matcher) !*Device {
        if (matcher.is_default) {
            const index = try self.getDefaultRenderDeviceIndex();
            const sio_device = try self.getSioRenderDeviceByIndex(index);
            const device_name_cs = @ptrCast([*:0]const u8, sio_device.name);
            const device_name = device_name_cs[0..mem.len(device_name_cs)];
            print(">>>>>>>>>>>>> name: {s}\n", .{device_name});
            var itr = mem.split(matcher.not_matches, "|");
            {
                print("-----------\n", .{});
                var name = itr.next();
                print("device_name: {s}\n", .{device_name});
                while (name != null) : (name = itr.next()) {
                    print("device_name: \"{s}\" vs name: \"{s}\"\n", .{ device_name, name });
                    const unwrapped = name orelse continue;
                    if (mem.indexOf(u8, device_name, unwrapped) != null) {
                        print("DEVICE NAME IS NOT VALID!\n", .{});
                        return error.Fail;
                    }
                }
                print("-----------\n", .{});
            }
            // const count = try self.getRenderDeviceCount();
        }
        return try self.createDevice(common.Direction.Render);
    }

    pub fn getCaptureDevice(self: *Devices, matcher: common.Matcher) !*Device {
        return try self.createDevice(common.Direction.Capture);
    }

    fn getSioRenderDeviceByIndex(self: *Devices, index: c_int) !*SoundIoDevice {
        const sio_device = soundio_get_output_device(self.soundio, index);
        if (sio_device == null) {
            print("soundio_get_output_device failed\n", .{});
            return error.Fail;
        }
        print("soundio_get_output_device success\n", .{});
        return sio_device;
    }

    fn getDefaultRenderDeviceIndex(self: *Devices) !c_int {
        const index = soundio_default_output_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_output_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        print("soundio_default_output_device_index index: {}\n", .{index});
        return index;
    }

    fn getRenderDeviceCount(self: *Devices) !c_int {
        const count = soundio_output_device_count(self.soundio);
        if (index_failed(count)) {
            print("soundio_output_device_count failed with: {}\n", .{count});
            return error.Fail;
        }
        print("soundio_output_device_count: {}\n", .{count});
        return count;
    }

    fn getDefaultCaptureDeviceIndex(self: *Devices) !c_int {
        const index = soundio_default_input_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_input_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        print("soundio_default_input_device_index: {}\n", .{index});
        return index;
    }

    fn getCaptureDeviceCount(self: *Devices) !c_int {
        const count = soundio_input_device_count(self.soundio);
        if (index_failed(count)) {
            print("soundio_input_device_count failed with: {}\n", .{count});
            return error.Fail;
        }
        print("soundio_input_device_count: {}\n", .{count});
        return count;
    }
};

test "Nix Native.Devices is instantiable" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();
}

test "Nix Native.Devices info" {
    const name = info();
    try expectEqual(name, "LINUX");
}

test "Nix Native.Default device" {
    const devices = try Devices.init(std.testing.allocator);
    defer devices.deinit();

    const device = try devices.getDevice(common.DefaultCapture);
    defer device.deinit();
}

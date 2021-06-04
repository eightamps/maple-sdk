const std = @import("std");
const common = @import("../aud_common.zig");
const helpers = @import("../../helpers.zig");

const sio = @cImport({
    @cInclude("soundio/soundio.h");
});

const Allocator = std.mem.Allocator;
const expect = std.testing.expect;
const print = std.debug.print;

usingnamespace common;
usingnamespace sio;

fn failed(status: c_int) bool {
    return status != 0;
}

fn index_failed(status: c_int) bool {
    return status < 0;
}

pub const Devices = struct {
    allocator: *Allocator,
    soundio: *SoundIo,
    ring_buffer: *SoundIoRingBuffer = undefined,
    devices: []Device = undefined,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);

        const soundio = soundio_create();
        if (soundio == null) {
            print("soundio failed to allocate\n", .{});
            return error.Fail;
        }
        print("soundio successfully created context\n", .{});

        {
            const value = @intToEnum(sio.SoundIoBackend, sio.SoundIoBackendPulseAudio);
            const status = soundio_connect_backend(soundio, value);
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
        soundio_destroy(self.soundio);
        self.allocator.destroy(self);
    }

    pub fn info(self: *Devices) []const u8 {
        return "nix";
    }

    pub fn getDefaultDevice(self: *Devices, direction: Direction) !Device {
        if (direction == Direction.Capture) {
            return self.getDefaultCaptureDevice();
        } else {
            return self.getDefaultRenderDevice();
        }
    }

    pub fn getDefaultCaptureDevice(self: *Devices) !Device {
        return error.Fail;
    }

    pub fn getDefaultRenderDevice(self: *Devices) !Device {
        const index = soundio_default_output_device_index(self.soundio);
        if (index_failed(index)) {
            print("soundio_default_output_device_index failed with: {}\n", .{index});
            return error.Fail;
        }
        print("soundio_default_output_device_index index: {}\n", .{index});
        const sio_device = try self.getRenderDeviceByIndex(index);
        const device = Device{
            .id = 0,
            .name = "asdf",
        };
        return device;
    }

    fn getRenderDeviceByIndex(self: *Devices, index: c_int) !*SoundIoDevice {
        const sio_device = soundio_get_output_device(self.soundio, index);
        if (sio_device == null) {
            print("soundio_get_output_device failed\n", .{});
            return error.Fail;
        }
        print("soundio_get_output_device success\n", .{});
        return sio_device;
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

    pub fn getDevices(self: *Devices, buffer: []Device, direction: Direction) ![]Device {
        // const filter = if (direction == Direction.Capture) isCaptureDevice else isRenderDevice;
        // var filters = [_]DeviceFilter{filter};
        // return helpers.filterItems(Device, self.devices, buffer, &filters);
        return error.Fail;
    }

    pub fn getCaptureDevices(self: *Devices, buffer: []Device) ![]Device {
        return self.getDevices(buffer, Direction.Capture);
    }

    pub fn getRenderDevices(self: *Devices, buffer: []Device) ![]Device {
        return self.getDevices(buffer, Direction.Render);
    }

    pub fn getCaptureDeviceAt(self: *Devices, index: u16) *Device {
        var buffer: [MAX_DEVICE_COUNT]Device = undefined;
        var result = try self.getCaptureDevices(&buffer);
        return &result[index];
    }

    pub fn getRenderDeviceAt(self: *Devices, index: u16) *Device {
        var buffer: [MAX_DEVICE_COUNT]Device = undefined;
        var result = try self.getRenderDevices(&buffer);
        return &result[index];
    }
};

test "Nix Devices is instantiable" {
    var alloc = std.testing.allocator;
    var api = try Devices.init(alloc);
    defer api.deinit();

    print("YOOOOOOOOO\n", .{});
    expect(false);
}

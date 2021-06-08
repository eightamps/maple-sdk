const std = @import("std");
const common = @import("common.zig");
const helpers = @import("../helpers.zig");

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const Device = common.Device;
const DeviceFilter = common.DeviceFilter;
const Direction = common.Direction;
const ascii = std.ascii;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const fs = std.fs;
const heap = std.heap;
const isCaptureDevice = common.isCaptureDevice;
const isDefaultDevice = common.isDefaultDevice;
const isRenderDevice = common.isRenderDevice;
const json = std.json;
const mem = std.mem;
const print = std.debug.print;

const InitType = enum(u8) {
    Path,
    Devices,
};

const JsonData = struct {
    render_devices: []Device,
    capture_devices: []Device,
};

pub const Devices = struct {
    allocator: *Allocator,
    devices: []Device,
    initialized_with: InitType,

    fn loadFakeDevices(a: *Allocator, path: []const u8) ![]Device {
        const f = try fs.cwd().openFile(path, .{ .read = true });
        defer f.close();

        const max_data_size: usize = @sizeOf(Device) * 64;
        const bytes = try f.reader().readAllAlloc(a, max_data_size);
        defer a.free(bytes);

        var stream = json.TokenStream.init(bytes);
        var devices = try json.parse([]Device, &stream, .{ .allocator = a });

        // Print device names and details
        // for (devices) |device| {
        //     print("device: {d} {s} \t\t({e})\n", .{ device.id, device.name, device.direction });
        // }

        return devices;
    }

    pub fn initWithDevicesPath(a: *Allocator, devices_path: []const u8) !*Devices {
        const instance = try a.create(Devices);
        const devices = try loadFakeDevices(a, devices_path);
        instance.* = Devices{
            .allocator = a,
            .devices = devices,
            .initialized_with = InitType.Path,
        };
        return instance;
    }

    pub fn initWithDevices(a: *Allocator, devices: []Device) !*Devices {
        const instance = try a.create(Devices);
        instance.* = Devices{
            .allocator = a,
            .devices = devices,
            .initialized_with = InitType.Devices,
        };
        return instance;
    }

    pub fn deinit(self: *Devices) void {
        // Clear JSON parse memory only if created with JSON data
        if (self.initialized_with == InitType.Path) {
            json.parseFree([]Device, self.devices, .{ .allocator = self.allocator });
            // for (self.devices) |dev| {
            // self.allocator.destroy(&dev);
            // }
        }

        self.allocator.destroy(self);
    }

    pub fn info(self: *Devices) []const u8 {
        return "fake";
    }

    pub fn getDefaultDevice(self: *Devices, direction: Direction) !Device {
        var buffer: [common.MAX_DEVICE_COUNT]Device = undefined;
        var dir_filter = if (direction == Direction.Capture) isCaptureDevice else isRenderDevice;
        var filters = [_]DeviceFilter{
            isDefaultDevice,
            dir_filter,
        };

        var result = helpers.filterItems(Device, self.devices, &buffer, &filters);
        if (result.len > 0) {
            return result[0];
        }

        return error.Fail;
    }

    pub fn getDefaultCaptureDevice(self: *Devices) !Device {
        return self.getDefaultDevice(Direction.Capture);
    }

    pub fn getDefaultRenderDevice(self: *Devices) !Device {
        return self.getDefaultDevice(Direction.Render);
    }

    pub fn getDevices(self: *Devices, buffer: []Device, direction: Direction) ![]Device {
        const filter = if (direction == Direction.Capture) isCaptureDevice else isRenderDevice;
        var filters = [_]DeviceFilter{filter};
        return helpers.filterItems(Device, self.devices, buffer, &filters);
    }

    pub fn getCaptureDevices(self: *Devices, buffer: []Device) ![]Device {
        return self.getDevices(buffer, Direction.Capture);
    }

    pub fn getRenderDevices(self: *Devices, buffer: []Device) ![]Device {
        return self.getDevices(buffer, Direction.Render);
    }

    pub fn getCaptureDeviceAt(self: *Devices, index: u16) *Device {
        var buffer: [common.MAX_DEVICE_COUNT]Device = undefined;
        var result = try self.getCaptureDevices(&buffer);
        return &result[index];
    }

    pub fn getRenderDeviceAt(self: *Devices, index: u16) *Device {
        var buffer: [common.MAX_DEVICE_COUNT]Device = undefined;
        var result = try self.getRenderDevices(&buffer);
        return &result[index];
    }
};

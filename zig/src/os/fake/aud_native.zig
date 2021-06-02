const std = @import("std");
const common = @import("../aud_common.zig");

const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;
const ascii = std.ascii;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;
const fs = std.fs;
const heap = std.heap;
const json = std.json;
const mem = std.mem;
const print = std.debug.print;

usingnamespace common;

const DEFAULT_DEVICE_NAME = "[unknown]";

const WAY2CALL_SUBSTR = "Way2Call";
const ASI_TEL_SUBSTR = "ASI Telephone";

const InitType = enum(u8) {
    Path,
    Devices,
};

const JsonData = struct {
    render_devices: []Device,
    capture_devices: []Device,
};

const DeviceFilter = fn (device: Device) bool;

pub fn info() []const u8 {
    return "TEST";
}

pub const Devices = struct {
    allocator: *Allocator,
    devices: []Device,
    initialized_with: InitType,

    fn includeAll(device: Device) bool {
        return true;
    }

    // Filter device names to ensure they do not contain (case-insensitive) either
    // "Way2Call" or "ASI Telephone". This is used by the default device requests
    // to guarantee we never attempt to send or receive host-side user audio through
    // these known-bad devices, which Microsoft insists on forcing into the default
    // position(s).
    fn filterW2CAndAsi(device: Device) bool {
        if (ascii.indexOfIgnoreCasePos(device.name, 0, WAY2CALL_SUBSTR) != null) return false;
        if (ascii.indexOfIgnoreCasePos(device.name, 0, ASI_TEL_SUBSTR) != null) return false;
        return true;
    }

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
        }

        self.allocator.destroy(self);
    }

    fn getFilteredDevicesByDirection(self: *Devices, result: *ArrayList(*Device), direction: Direction, filter: DeviceFilter) !void {
        var i: u16 = 0;
        while (i < self.devices.len) : (i += 1) {
            const device = self.devices[i];
            if (device.direction == direction and filter(device)) {
                // print("device: {d} {s}\n", .{ device.id, device.name });
                try result.append(&self.devices[i]);
            }
        }
    }

    fn getDefaultDeviceByDirection(self: *Devices, direction: Direction) !*Device {
        var subset = ArrayList(*Device).init(std.testing.allocator);
        defer subset.deinit();

        try self.getFilteredDevicesByDirection(&subset, direction, filterW2CAndAsi);
        var devices = subset.items;

        if (devices.len == 0) {
            return error.Fail;
        }

        var i: u16 = 0;
        while (i < devices.len) : (i += 1) {
            var device = devices[i];
            if (device.is_default and device.direction == direction) {
                return device;
            }
        }

        return devices[0];
    }

    pub fn getDevicesByDirection(self: *Devices, result: *ArrayList(*Device), direction: Direction) !void {
        try self.getFilteredDevicesByDirection(result, direction, includeAll);
    }

    pub fn getRenderDevices(self: *Devices, result: *ArrayList(*Device)) !void {
        return self.getDevicesByDirection(result, Direction.Render);
    }

    pub fn getCaptureDevices(self: *Devices, result: *ArrayList(*Device)) !void {
        return self.getDevicesByDirection(result, Direction.Capture);
    }

    pub fn getDefaultCaptureDevice(self: *Devices) !*Device {
        return try self.getDefaultDeviceByDirection(Direction.Capture);
    }

    pub fn getDefaultRenderDevice(self: *Devices) !*Device {
        return try self.getDefaultDeviceByDirection(Direction.Render);
    }

    pub fn getCaptureDeviceAt(self: *Devices, index: u16) *Device {
        return &self.devices[index];
    }

    pub fn getRenderDeviceAt(self: *Devices, index: u16) *Device {
        return &self.devices[index];
    }
};

const std = @import("std");
const common = @import("../aud_common.zig");
const helpers = @import("../../helpers.zig");

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

pub const Devices = struct {
    allocator: *Allocator,
    devices: []Device = undefined,

    pub fn init(a: *Allocator) !*Devices {
        const instance = try a.create(Devices);
        instance.* = Devices{
            .allocator = a,
        };
        return instance;
    }

    pub fn deinit(self: *Devices) void {
        self.allocator.destroy(self);
    }

    pub fn info(self: *Devices) []const u8 {
        return "nix";
    }

    pub fn getDefaultDevice(self: *Devices, direction: Direction) !Device {
        var buffer: [MAX_DEVICE_COUNT]Device = undefined;
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

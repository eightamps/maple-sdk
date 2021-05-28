const std = @import("std");
const audible = @import("../../audible_all.zig");

const Allocator = std.mem.Allocator;
const print = std.debug.print;

const DEFAULT_DEVICE_NAME = "[unknown]";

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

pub fn info() []const u8 {
    return "LINUX";
}

pub const AudioApi = struct {
    allocator: *Allocator,

    pub fn init(a: *Allocator) !*AudioApi {
        const instance = try a.create(AudioApi);
        instance.* = AudioApi{
            .allocator = a,
        };
        return instance;
    }

    pub fn deinit(self: *AudioApi) void {
        print("AudibleApi.deinit called\n", .{});
        self.allocator.destroy(self);
    }

    pub fn getDefaultDevice(self: *AudioApi) !AudioDevice {
        std.debug.print("Linux getDefaultDevice()\n", .{});
        var device = self.allocator.alloc(AudioDevice, 1);
        return device;
    }
};

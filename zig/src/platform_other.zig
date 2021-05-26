const std = @import("std");

const DEFAULT_DEVICE_NAME = "[unknown]";

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

pub fn info() []const u8 {
    return "OTHER";
}

pub const AudioApi = struct {
    is_initialized: bool = false,

    fn init(self: *AudioApi) !void {}

    pub fn deinit(self: *AudioApi) void {}

    pub fn getDefaultDevice(self: *AudioApi) !AudioDevice {
        std.debug.print("Other getDefaultDevice()\n", .{});
        //var enumerator: ?mm.IMMDeviceEnumerator = null;

        //var status: mm.HRESULT = mm.CoCreateInstance(&mm.CLSID_MMDeviceEnumerator, null, mm.CLSCTX_ALL, &mm.IID_IMMDeviceEnumerator, @ptrCast([*c]?*c_void, &enumerator));
        // EXIT_ON_ERROR(status, "CoCreateInstance with p_enumerator failed");

        // status = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, datadir,
        //     role, device);
        // EXIT_ON_ERROR(status, "GetDefaultAudioEndpoint failed");
        // log_info("get_default_device returned");
        return AudioDevice{};
    }
};

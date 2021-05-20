const std = @import("std");

const DEFAULT_DEVICE_NAME = "[unknown]";

pub export fn hello() i32 {
    std.debug.print("api_nix:hello\n", .{});
    return 0;
}

pub fn info() []const u8 {
    return "LINUX";
}

pub fn AudioApi() type {
    return struct {
        const Self = @This();

        pub fn getDefaultDevice() AudioDevice {
            std.debug.print("Linux getDefaultDevice()\n", .{});
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
}

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

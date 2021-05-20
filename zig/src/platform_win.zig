const std = @import("std");

const mm = @cImport({
    @cInclude("mmdeviceapi.h");
    @cInclude("audioclient.h");
    @cInclude("combaseapi.h");
});

usingnamespace std.os.windows;
// usingnamespace mm;

const DLL_PROCESS_ATTACH = 1;
const DLL_THREAD_ATTACH = 2;
const DLL_THREAD_DETACH = 3;
const DLL_PROCESS_DETACH = 0;

const DEFAULT_DEVICE_NAME = "[unknown]";

pub export fn hello() i32 {
    std.debug.print("api_win:hello\n", .{});
    return 0;
}

pub fn AudioApi() type {
    return struct {
        const Self = @This();

        pub fn getDefaultDevice() AudioDevice {
            std.debug.print("YOOOOO\n", .{});
            var enumerator: ?mm.IMMDeviceEnumerator = null;

            // var status: mm.HRESULT = mm.CoCreateInstance(&mm.CLSID_MMDeviceEnumerator, null, mm.CLSCTX_ALL, &mm.IID_IMMDeviceEnumerator, @ptrCast([*c]?*c_void, &enumerator));
            // EXIT_ON_ERROR(status, "CoCreateInstance with p_enumerator failed");

            // status = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, datadir,
            //     role, device);
            // EXIT_ON_ERROR(status, "GetDefaultAudioEndpoint failed");
            // log_info("get_default_device returned");
            return AudioDevice{};
        }
    };
}

pub fn info() []const u8 {
    return "WINDOWS";
}

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

// Confirmed that process attach and detach are actually called from a simple C# application.
pub export fn DllMain(hInstance: HINSTANCE, ul_reason_for_call: DWORD, lpReserved: LPVOID) BOOL {
    switch (ul_reason_for_call) {
        DLL_PROCESS_ATTACH => {
            std.debug.print("win32.dll PROCESS ATTACH\n", .{});
        },
        DLL_THREAD_ATTACH => {},
        DLL_THREAD_DETACH => {},
        DLL_PROCESS_DETACH => {
            std.debug.print("win32.dll PROCESS DETACH\n", .{});
        },
        else => {},
    }
    return 1;
}

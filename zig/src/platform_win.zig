const std = @import("std");

const win = @cImport({
    @cDefine("WIN32_LEAN_AND_MEAN", "1");
    @cInclude("windows.h");
    @cInclude("initguid.h");
    @cInclude("mmdeviceapi.h");
    @cInclude("audioclient.h");
    @cInclude("combaseapi.h");
});

usingnamespace win;

const DEFAULT_DEVICE_NAME = "[unknown]";

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

pub fn info() []const u8 {
    return "WINDOWS";
}

pub fn AudioApi() type {
    return struct {
        const Self = @This();

        pub fn getDefaultDevice() AudioDevice {
            _ = CoInitialize(null);

            // Comment the following 2 lines to get a working Windows build.
            var ptr: LPVOID = null;
            std.debug.print("pre ptr: {s}\n", .{ptr});
            var status: HRESULT = CoCreateInstance(&CLSID_MMDeviceEnumerator, null, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &ptr);
            std.debug.print("STATUS: {d}\n", .{status});
            std.debug.print("post ptr: {s}\n", .{ptr});

            var ptrValue = @ptrToInt(ptr);
            std.debug.print("ptrValue: 0x{x}\n", .{ptrValue});

            // var enumerator: IMMDeviceEnumerator = @intToPtr(&IMMDeviceEnumerator, ptrValue);

            // var enumerator2: IMMDeviceEnumerator = @ptrCast(&IMMDeviceEnumerator, &enumerator);
            // var devices: ?*c_void = null;
            // status = enumerator.GetDefaultAudioEndpoint(EDataFlow.eCommunications, ERole.eCapture, &devices);

            // status = enumerator.GetDefaultAudioEndpoint(datadir, role, device);
            std.debug.print("STATUS: {d}\n", .{status});
            // EXIT_ON_ERROR(status, "GetDefaultAudioEndpoint failed");
            // log_info("get_default_device returned");
            return AudioDevice{};
        }
    };
}

// Confirmed that process attach and detach are actually called from a simple C# application.
pub export fn DllMain(hInstance: std.os.windows.HINSTANCE, ul_reason_for_call: DWORD, lpReserved: LPVOID) BOOL {
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

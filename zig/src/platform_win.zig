const std = @import("std");

const mm = @cImport({
    // @cDefine("WIN32_LEAN_AND_MEAN", "1");
    // @cInclude("windows.h");
    @cInclude("mmdeviceapi.h");
    @cInclude("audioclient.h");
    @cInclude("combaseapi.h");
});

usingnamespace std.os.windows;
// usingnamespace mm;

// const DLL_PROCESS_ATTACH = 1;
// const DLL_THREAD_ATTACH = 2;
// const DLL_THREAD_DETACH = 3;
// const DLL_PROCESS_DETACH = 0;

const DEFAULT_DEVICE_NAME = "[unknown]";

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

pub fn info() []const u8 {
    return "WINDOWS";
}

// pub extern fn CoCreateInstance(rclsid: [*c]const IID, pUnkOuter: LPUNKNOWN, dwClsContext: DWORD, riid: [*c]const IID, ppv: [*c]LPVOID) HRESULT;

// extern "user32" fn CoCreateInstance(rclsid: REFCLSID, pUnkOuter: LPUNKNOWN, dwClsContext: DWORD, riid: REFIID, ppv: *LPVOID) c_int;

// HRESULT CoCreateInstance(
//   REFCLSID  rclsid,
//   LPUNKNOWN pUnkOuter,
//   DWORD     dwClsContext,
//   REFIID    riid,
//   LPVOID    *ppv
// );

// extern "user32" stdcallcc fn MessageBoxA(hWnd: ?HANDLE, lpText: ?LPCTSTR, lpCaption: ?LPCTSTR, uType: UINT) c_int;

// extern "user32" stdcallcc fn CoCreateInstance(&CLSID_MMDeviceEnumerator, null, CLSCTX_ALL, &IID_IMMDeviceEnumerator, @ptrCast([*c]?*c_void, &enumerator));

// extern "user32" fn CoCreateInstance(hWnd: ?HANDLE, lpText: ?LPCTSTR, lpCaption: ?LPCTSTR, uType: UINT) c_int;
// const CLSID_MMDeviceEnumerator: mm.CLSID = mm.StringFromCLSID(__LIBID_(&mm.MMDeviceEnumerator));

// var CLSID_MMDeviceEnumerator: ?CLSID = null;

// const CLSID_MMDeviceEnumerator: ?CLSID = __uuidof(MMDeviceEnumerator);

// static const CLSID CLSID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
// static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
// static const IID IID_IAudioClient = __uuidof(IAudioClient);
// static const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

// pub extern fn CoCreateInstance(CLSID_MMDeviceEnumerator, null, CLSCTX_ALL, IID_IMMDeviceEnumerator, &enumerator);
// pub extern fn CoCreateInstance(rclsid: [*c]const IID, pUnkOuter: LPUNKNOWN, dwClsContext: DWORD, riid: [*c]const IID, ppv: [*c]LPVOID) HRESULT;

pub fn AudioApi() type {
    return struct {
        const Self = @This();

        pub fn getDefaultDevice() AudioDevice {
            std.debug.print("getDefaultDevice\n", .{});
            var ptr: ?*c_void = null;
            var status: HRESULT = mm.CoCreateInstance(&mm.CLSID_MMDeviceEnumerator, null, mm.CLSCTX_ALL, &mm.IID_IMMDeviceEnumerator, &ptr);

            // var status: mm.HRESULT = mm.CoCreateInstance(CLSID_MMDeviceEnumerator, null, mm.CLSCTX_ALL, &mm.IID_IMMDeviceEnumerator, &enumerator);
            // EXIT_ON_ERROR(status, "CoCreateInstance with p_enumerator failed");

            // status = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, datadir,
            //     role, device);
            // EXIT_ON_ERROR(status, "GetDefaultAudioEndpoint failed");
            // log_info("get_default_device returned");
            return AudioDevice{};
        }
    };
}

// Confirmed that process attach and detach are actually called from a simple C# application.
pub export fn DllMain(hInstance: std.os.windows.HINSTANCE, ul_reason_for_call: DWORD, lpReserved: LPVOID) BOOL {
    switch (ul_reason_for_call) {
        mm.DLL_PROCESS_ATTACH => {
            std.debug.print("win32.dll PROCESS ATTACH\n", .{});
        },
        mm.DLL_THREAD_ATTACH => {},
        mm.DLL_THREAD_DETACH => {},
        mm.DLL_PROCESS_DETACH => {
            std.debug.print("win32.dll PROCESS DETACH\n", .{});
        },
        else => {},
    }
    return 1;
}

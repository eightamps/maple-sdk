const std = @import("std");
const builtin = @import("builtin");
const audio = @import("platform/audio_win.zig");
const stitch = @import("stitch.zig");

usingnamespace std.os.windows;

// Export stitch features on shared library
usingnamespace stitch;
usingnamespace audio;

// extern "user32" fn MessageBoxA(hWnd: ?HANDLE, lpText: ?LPCTSTR, lpCaption: ?LPCTSTR, uType: UINT) c_int;

const DLL_PROCESS_ATTACH = 1;
const DLL_THREAD_ATTACH = 2;
const DLL_THREAD_DETACH = 3;
const DLL_PROCESS_DETACH = 0;

// export fn hello(data: *c_void, size: i32) i32 {
// _ = MessageBoxA(null, "hello, Im in Exported Function", "title", 0);
// return 0;
// }

pub export fn add(a: i32, b: i32) i32 {
    std.debug.print("sdk_win.add with: {d} {d}\n", .{ a, b });
    return a + b;
}

// Confirmed that process attach and detach are actually called from a simple C# application.
pub export fn DllMain(hInstance: HINSTANCE, ul_reason_for_call: DWORD, lpReserved: LPVOID) BOOL {
    switch (ul_reason_for_call) {
        DLL_PROCESS_ATTACH => {
            std.debug.print("win32.dll PROCESS ATTACH\n", .{});
            // _ = stitch_new();
            // _ = MessageBoxA(null, "hello, Im in DllMain", "title", 0);
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

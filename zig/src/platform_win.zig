const std = @import("std");

usingnamespace std.os.windows;

const DLL_PROCESS_ATTACH = 1;
const DLL_THREAD_ATTACH = 2;
const DLL_THREAD_DETACH = 3;
const DLL_PROCESS_DETACH = 0;

pub export fn hello() i32 {
    std.debug.print("api_win:hello\n", .{});
    return 0;
}

pub fn info() []const u8 {
    return "FROM WIN";
}

pub const AudioDevice = struct {};

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

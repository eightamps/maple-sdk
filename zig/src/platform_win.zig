const std = @import("std");
const win32 = @import("win32");

const print = std.debug.print;

usingnamespace win32.media.audio.core_audio;
usingnamespace win32.media.audio.direct_music;
usingnamespace win32.storage.structured_storage;
usingnamespace win32.system.com;
usingnamespace win32.system.properties_system;
usingnamespace win32.system.system_services;
usingnamespace win32.zig;

const DEFAULT_DEVICE_NAME = "[unknown]";

const AllocationError = error{
    OutOfMemory,
};

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

pub fn info() []const u8 {
    return "WINDOWS";
}

pub fn AudioApi() type {
    return struct {
        const Self = @This();

        pub fn getDefaultDevice() !AudioDevice {
            var status: i32 = 0;

            const config_value = @enumToInt(COINIT_APARTMENTTHREADED) | @enumToInt(COINIT_DISABLE_OLE1DDE);
            status = CoInitialize(null); // CoInitializeEx(null, @intToEnum(COINIT, config_value));
            if (FAILED(status)) {
                print("CoInitialize FAILED: {d}\n", .{status});
                return AllocationError.OutOfMemory;
            }
            defer CoUninitialize();

            var enumerator: *IMMDeviceEnumerator = undefined;
            status = CoCreateInstance(CLSID_MMDeviceEnumerator, null, CLSCTX_ALL, IID_IMMDeviceEnumerator, @ptrCast(**c_void, &enumerator));
            if (FAILED(status)) {
                print("CoCreateInstance FAILED: {d}\n", .{status});
                return AllocationError.OutOfMemory;
            }
            // defer enumerator.IMMDeviceEnumerator_Release(); // No such method

            print("pre enumerator: {s}\n", .{enumerator});

            var device: *IMMDevice = undefined;
            status = enumerator.IMMDeviceEnumerator_GetDefaultAudioEndpoint(EDataFlow.eCapture, ERole.eCommunications, &device);
            if (FAILED(status)) {
                print("DEVICE STATUS: {d}\n", .{status});
                return AllocationError.OutOfMemory;
            }
            // defer device.IMMDevice_Release(); // No such method

            var properties: *IPropertyStore = undefined;
            status = device.IMMDevice_OpenPropertyStore(STGM_READ, &properties);
            if (FAILED(status)) {
                print("DEVICE PROPS: {d}\n", .{status});
                return AllocationError.OutOfMemory;
            }

            var count: u32 = 0;
            status = properties.IPropertyStore_GetCount(&count);
            if (FAILED(status)) {
                print("GetCount failed: {d}\n", .{status});
                return AllocationError.OutOfMemory;
            }

            var index: u32 = 0;
            while (index < count - 1) : (index += 1) {
                var propKey: PROPERTYKEY = undefined;
                var propValue: PROPVARIANT = undefined;

                print("index: {d}\n", .{index});
                status = properties.IPropertyStore_GetAt(index, &propKey);
                if (FAILED(status)) {
                    print("Failed to getAt {x}\n", .{status});
                    return AllocationError.OutOfMemory;
                }
                print("Looping propeties with: {s}\n", .{propKey});
                status = properties.IPropertyStore_GetValue(&propKey, &propValue);
            }

            // print("post device: {s}\n", .{device.IMMDevice_GetId()});

            return AudioDevice{};
        }
    };
}

// Confirmed that process attach and detach are actually called from a simple C# application.
// pub export fn DllMain(hInstance: std.os.windows.HINSTANCE, ul_reason_for_call: DWORD, lpReserved: LPVOID) BOOL {
//     switch (ul_reason_for_call) {
//         DLL_PROCESS_ATTACH => {
//             print("win32.dll PROCESS ATTACH\n", .{});
//         },
//         DLL_THREAD_ATTACH => {},
//         DLL_THREAD_DETACH => {},
//         DLL_PROCESS_DETACH => {
//             print("win32.dll PROCESS DETACH\n", .{});
//         },
//         else => {},
//     }
//     return 1;
// }

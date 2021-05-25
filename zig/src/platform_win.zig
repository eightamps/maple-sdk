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

pub const AudioDevice = struct {
    name: []const u8 = DEFAULT_DEVICE_NAME,
};

pub fn info() []const u8 {
    return "WINDOWS";
}

pub const AudioApi = struct {
    is_initialized: bool = false,

    fn init(self: *AudioApi) !void {
        {
            const status = CoInitialize(null);
            if (FAILED(status)) {
                print("CoInitialize FAILED: {d}\n", .{status});
                return error.Fail;
            }
        }
    }

    pub fn deinit(self: *AudioApi) void {
        CoUninitialize();
    }

    pub fn getDefaultDevice(self: *AudioApi) !AudioDevice {
        if (!self.is_initialized) {
            try self.init();
        }

        var enumerator: *IMMDeviceEnumerator = undefined;
        {
            const status = CoCreateInstance(CLSID_MMDeviceEnumerator, null, CLSCTX_ALL, IID_IMMDeviceEnumerator, @ptrCast(**c_void, &enumerator));
            if (FAILED(status)) {
                print("CoCreateInstance FAILED: {d}\n", .{status});
                return error.Fail;
            }
        }
        defer _ = enumerator.IUnknown_Release();
        print("enumerator: {s}\n", .{enumerator});

        var device: *IMMDevice = undefined;
        {
            const status = enumerator.IMMDeviceEnumerator_GetDefaultAudioEndpoint(EDataFlow.eCapture, ERole.eCommunications, &device);
            if (FAILED(status)) {
                print("DEVICE STATUS: {d}\n", .{status});
                return error.Fail;
            }
            if (device == undefined) {
                print("GetDefaultAudioEndpoint failed to initialize device\n", .{});
                return error.Fail;
            }
        }
        defer _ = device.IUnknown_Release();
        print("---------------------\n", .{});
        print("default device: {s}\n", .{device});

        var audio_client: *IAudioClient = undefined;
        {
            const status = device.IMMDevice_Activate(
                IID_IAudioClient,
                @enumToInt(CLSCTX_ALL),
                null,
                @ptrCast(**c_void, &audio_client),
            );
            if (FAILED(status)) {
                print("AudioClient failed {d}\n", .{status});
                return error.Fail;
            }
        }
        defer _ = audio_client.IUnknown_Release();
        print("Audio Clien: {s}\n", .{audio_client});

        // var properties: *IPropertyStore = undefined;
        // {
        //     const status = device.IMMDevice_OpenPropertyStore(STGM_READ, &properties);
        //     if (FAILED(status)) {
        //         print("DEVICE PROPS: {d}\n", .{status});
        //         return error.Fail;
        //     }
        // }
        // defer _ = properties.IUnknown_Release();

        // var count: u32 = 0;
        // {
        //     const status = properties.IPropertyStore_GetCount(&count);
        //     if (FAILED(status)) {
        //         print("GetCount failed: {d}\n", .{status});
        //         return error.Fail;
        //     }
        // }

        // var index: u32 = 0;
        // while (index < count - 1) : (index += 1) {
        //     var propKey: PROPERTYKEY = undefined;
        //     var propValue: PROPVARIANT = undefined;

        //     print("index: {d}\n", .{index});
        //     const p_status = properties.IPropertyStore_GetAt(index, &propKey);
        //     if (FAILED(p_status)) {
        //         print("Failed to getAt {x}\n", .{p_status});
        //         return error.Fail;
        //     }

        //     print("Looping properties with: {s}\n", .{propKey});
        //     const v_status = properties.IPropertyStore_GetValue(&propKey, &propValue);
        //     print("Found value: {s}\n", .{propValue});
        // }

        // print("post device: {s}\n", .{device.IMMDevice_GetId()});

        return AudioDevice{};
    }
};

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

const std = @import("std");
const audible = @import("../../audible_all.zig");
const win32 = @import("win32");

const fmt = std.fmt;
const mem = std.mem;
const print = std.debug.print;

usingnamespace win32.media.audio.core_audio;
usingnamespace win32.media.audio.direct_music;
usingnamespace win32.storage.structured_storage;
usingnamespace win32.system.com;
usingnamespace win32.system.properties_system;
usingnamespace win32.system.system_services;
usingnamespace win32.zig;

// const DEFAULT_DEVICE_NAME: []u8 = "[unknown]";

fn printU16String(ptr: [*:0]const u16) []u8 {
    const len = mem.len(ptr);
    var result: [256]u8 = undefined;

    var smaller = result[0 .. len + 1];

    print("THE GODDAMN UTF16 STRING is: ", .{}); // in next lines
    var i: u16 = 0;
    while (i < len) : (i += 1) {
        print("{c}", .{@truncate(u8, ptr[i])});
        smaller[i] = @truncate(u8, ptr[i]);
    }
    smaller[i] = @as(u8, 0);

    print("\n", .{});
    print("SMALLER: {s}\n", .{smaller});

    return smaller;
}

pub const AudioDevice = struct {
    name: []const u8 = "[unknown]",
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

        var device_id: PWSTR = undefined;
        {
            const status = device.IMMDevice_GetId(&device_id);
            if (FAILED(status)) {
                print("Failed to get device id: {d}\n", .{status});
                return error.Fail;
            }
        }
        print("--------------------\n", .{});
        print("device_id: {d}\n", .{device_id});
        print("device_id len: {d}\n", .{mem.len(device_id)});

        // var audio_client: *IAudioClient = undefined;
        // {
        //     const status = device.IMMDevice_Activate(
        //         IID_IAudioClient,
        //         @enumToInt(CLSCTX_ALL),
        //         null,
        //         @ptrCast(**c_void, &audio_client),
        //     );
        //     if (FAILED(status)) {
        //         print("AudioClient failed {d}\n", .{status});
        //         return error.Fail;
        //     }
        // }
        // defer _ = audio_client.IUnknown_Release();
        // print("Audio Client: {s}\n", .{audio_client});

        var properties: *IPropertyStore = undefined;
        {
            const status = device.IMMDevice_OpenPropertyStore(STGM_READ, &properties);
            if (FAILED(status)) {
                print("DEVICE PROPS: {d}\n", .{status});
                return error.Fail;
            }
        }
        defer _ = properties.IUnknown_Release();
        print("--------------\n", .{});
        print("props: {s}\n", .{properties});

        var friendly_name: PROPVARIANT = undefined;
        var friendly_name_ptr: [*:0]u16 = undefined;
        {
            const status = properties.IPropertyStore_GetValue(&DEVPKEY_Device_FriendlyName, &friendly_name);
            if (FAILED(status)) {
                print("friendly_name failed: {d}\n", .{status});
                return error.Fail;
            }

            var vt = friendly_name.Anonymous.Anonymous.vt;
            // var ptr = &friendly_name.Anonymous.Anonymous.Anonymous.cVal;
            // var ptr = friendly_name.Anonymous.Anonymous.Anonymous.pcVal;
            // var ptr = &friendly_name.Anonymous.Anonymous.Anonymous.calpstr;
            // var ptr = &friendly_name.Anonymous.Anonymous.Anonymous.calpwstr;
            // var ptr = friendly_name.Anonymous.Anonymous.Anonymous.pszVal;
            // var ptr = friendly_name.Anonymous.Anonymous.Anonymous.pwszVal;
            friendly_name_ptr = friendly_name.Anonymous.Anonymous.Anonymous.pwszVal;
            _ = printU16String(friendly_name_ptr);
        }

        // Enumerate over the prop store for each prop
        var count: u32 = 0;
        {
            const status = properties.IPropertyStore_GetCount(&count);
            if (FAILED(status)) {
                print("GetCount failed: {d}\n", .{status});
                return error.Fail;
            }
        }

        print("-------------------------\n", .{});
        var index: u32 = 0;
        while (index < count - 1) : (index += 1) {
            print("-----\n", .{});
            var prop_key: PROPERTYKEY = undefined;
            var prop_value: PROPVARIANT = undefined;

            print("index: {d}\n", .{index});
            const p_status = properties.IPropertyStore_GetAt(index, &prop_key);
            if (FAILED(p_status)) {
                print("Failed to getAt {x}\n", .{p_status});
                return error.Fail;
            }

            print("Looping properties with: {s}\n", .{prop_key});
            const v_status = properties.IPropertyStore_GetValue(&prop_key, &prop_value);

            const vt = prop_value.Anonymous.Anonymous.vt;
            print("Found value: {s}\n", .{prop_value});
            if (31 == vt) {
                const v_ptr = prop_value.Anonymous.Anonymous.Anonymous.pwszVal;
                //print("crapper: {s}\n", .{v_ptr});
                // const v_ptr = prop_value.Anonymous.Anonymous.Anonymous.pwszVal;
                _ = printU16String(v_ptr);
            } else {
                print("vt NOT NOT NOT 31 - {d}\n", .{vt});
            }
        }
        print("-------------------------\n", .{});

        // print("post device: {s}\n", .{device.IMMDevice_GetId()});
        var foo = printU16String(friendly_name_ptr);
        print("FOOOOOOOOOOOOOOOOOOOOOOOOO: {s}\n", .{foo});

        var audio_device = AudioDevice{
            .name = foo,
        };
        // mem.copy(u8, audio_device.name, friendly_name_ptr);

        return audio_device;
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

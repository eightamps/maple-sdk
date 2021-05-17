const std = @import("std");
const stitch = @import("../stitch.zig");

const msaudio = @cImport({
    @cInclude("mmdeviceapi.h");
    @cInclude("audioclient.h");
    @cInclude("windows.h");
});

usingnamespace stitch;
usingnamespace msaudio;

// static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
// static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
// static const IID IID_IAudioClient = __uuidof(IAudioClient);
// static const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

pub fn stitch_get_audio_device(matcher: Matcher) AudioDevice {
    const device = AudioDevice{
        .matcher = matcher,
    };
    return device;
}

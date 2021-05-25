const std = @import("std");
const c = @cImport({
    @cInclude("libusb-1.0/libusb.h");
});

const print = std.debug.print;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;

pub const HidClientStatus = enum(u8) {
    Success,
    Failure,
};

pub const HidClient = struct {
    const Self = @This();

    vid: u32,
    pid: u32,
    libusb_context: *c.libusb_context = undefined,

    pub fn init(self: HidClient) HidClientStatus {
        return HidClientStatus.Success;
    }
};

test "HidClient is instantiable" {
    const client = HidClient{
        .vid = 0xe335,
        .pid = 0x8a01,
    };

    _ = client.init();

    try expectEqual(client.vid, 0xe335);
    try expectEqual(client.pid, 0x8a01);
}

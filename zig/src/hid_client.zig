const std = @import("std");
// const libusb = @cImport({
//     @cInclude("libusb-1.0/libusb.h");
// });
// usingnamespace libusb;

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
    // libusb_context: *libusb_context = undefined,

    pub fn open(self: HidClient) HidClientStatus {
        // var usb_ctx: *libusb_context = undefined;
        // var status = libusb_init(usb_ctx);
        // defer libusb_exit(usb_ctx);

        // int status = libusb_init(&usb_ctx);
        // c->usb_context = usb_ctx;

        // if (status != EXIT_SUCCESS) {
        // log_err("Failed to initialise libusb");
        // return -ECONNREFUSED; // Connection refused
        // }
        return HidClientStatus.Success;
    }
};

test "HidClient is instantiable" {
    const client = HidClient{
        .vid = 0xe335,
        .pid = 0x8a01,
    };

    _ = client.open();

    try expectEqual(client.vid, 0xe335);
    try expectEqual(client.pid, 0x8a01);
}

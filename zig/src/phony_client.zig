const std = @import("std");
const hid_client = @import("hid_client.zig");

usingnamespace hid_client;

const print = std.debug.print;
const expect = std.testing.expect;

const EIGHT_AMPS_VENDOR_ID: u16 = 0x335e;
const MAPLE_PRODUCT_ID: u16 = 0x8a01;

// fn FAILURE(status: i32) bool {
// return status != 0;
// };

pub const PhonyClient = struct {
    hid: HidClient,

    pub fn close(self: PhonyClient) void {
        self.hid.close();
    }

    pub fn open() !PhonyClient {
        const hid = try HidClient.open(EIGHT_AMPS_VENDOR_ID, MAPLE_PRODUCT_ID);

        return PhonyClient{
            .hid = hid,
        };
    }
};

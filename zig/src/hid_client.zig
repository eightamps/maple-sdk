const std = @import("std");
const libusb = @cImport({
    @cInclude("libusb-1.0/libusb.h");
});
usingnamespace libusb;

const print = std.debug.print;
const expect = std.testing.expect;
const expectEqual = std.testing.expectEqual;

fn FAILURE(status: i32) bool {
    return status != 0;
}

pub const HidClient = struct {
    vid: u16,
    pid: u16,
    usb_context: ?*libusb_context,
    device_handle: ?*libusb_device_handle,
    device: ?*libusb_device,

    pub fn close(self: HidClient) void {
        // if (self.device_handle != null) {
        // }
        // if (self.device) != null) {
        // }
        if (self.usb_context != null) {
            libusb_exit(self.usb_context);
        }
    }

    pub fn open(vid: u16, pid: u16) !HidClient {
        var ctx: *libusb_context = undefined;
        {
            const status = libusb_init(@ptrToInt(&ctx));
            if (FAILURE(status)) {
                print("libusb_init failed {d}\n", .{status});
                return error.Fail;
            }
        }

        var device_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
        {
            // device_handle = try find_device(ctx, vid, pid);
            if (device_handle == null) {
                print("libusb_find_device did not find device\n", .{});
                return error.Fail;
            }
            print("Successfully found the expected HID device\n", .{});
        }

        var device = libusb_get_device(device_handle);
        {
            const bus_no = libusb_get_bus_number(device);
            const dev_addr = libusb_get_device_address(device);
            print("Found hid device at bus: 0x{x} {d} and dev addr 0x{x} {d}\n", .{ bus_no, bus_no, dev_addr, dev_addr });

            var desc: *libusb_device_descriptor = undefined;
            {
                const status = libusb_get_device_descriptor(device, @ptrToInt(&desc));
                if (FAILURE(status)) {
                    return error.Fail;
                }
                print("idVendor: 0x{x}", .{desc.idVendor});
                print("idProduct: 0x{x}", .{desc.idProduct});
                print("iSerialNumber: {d}", .{desc.iSerialNumber});
                print("bNumConfigurations: {d}", .{desc.bNumConfigurations});
            }
        }

        return HidClient{
            .vid = vid,
            .pid = pid,
            .usb_context = ctx,
            .device_handle = device_handle,
            .device = device,
        };
    }
};

test "HidClient is instantiable" {
    const client = try HidClient.open(0xe335, 0x8a01);
    // const client = try HidClient.open(0x27c6, 0x538d);
    // const client = try HidClient.open(0x0bda, 0x565a);
    // const client = try HidClient.open(0x1d6b, 0x0002);
    defer client.close();

    try expectEqual(client.vid, 0xe335);
    try expectEqual(client.pid, 0x8a01);
}

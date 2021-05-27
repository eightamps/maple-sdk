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
    is_open: bool,
    usb_context: ?*libusb_context,
    device_handle: ?*libusb_device_handle,
    device: ?*libusb_device,

    pub fn close(self: HidClient) void {
        // if (self.is_open) {
        // release interface
        // }
        // if (self.device_handle != null) {
        // }
        // if (self.device) != null) {
        // }
        if (self.usb_context != null) {
            libusb_exit(self.usb_context);
        }
    }

    pub fn open(vid: u16, pid: u16) !HidClient {
        var is_open = false;

        var ctx: *libusb_context = undefined;
        {
            const status = libusb_init(@ptrToInt(&ctx));
            if (FAILURE(status)) {
                print("libusb_init failed {d}\n", .{status});
                return error.Fail;
            }
            print("libusb_init SUCCESS\n", .{});
        }

        const device_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
        {
            if (device_handle == null) {
                print("libusb_find_device did not find device\n", .{});
                return error.Fail;
            }
            print("Successfully found the expected HID device\n", .{});
        }

        const device = libusb_get_device(device_handle);
        {
            if (device == null) {
                print("libusb_get_device FAILURE\n", .{});
                return error.Fail;
            }
            const bus_no = libusb_get_bus_number(device);
            const dev_addr = libusb_get_device_address(device);
            print("Found hid device at bus hex:0x{x} d:{d} and dev addr hex:0x{x} d:{d}\n", .{ bus_no, bus_no, dev_addr, dev_addr });

            var desc: libusb_device_descriptor = undefined;
            {
                const status = libusb_get_device_descriptor(device, &desc);
                if (FAILURE(status)) {
                    print("libusb_get_device_descriptor FAILED with: {d}\n", .{status});
                    return error.Fail;
                }
                print("get device desc with SUCCESS: {d}\n", .{status});
                print("desc: {}\n", .{desc});
                print("idVendor: 0x{x}\n", .{desc.idVendor});
                print("idProduct: 0x{x}\n", .{desc.idProduct});
                print("iSerialNumber: {d}\n", .{desc.iSerialNumber});
                print("bNumConfigurations: {d}\n", .{desc.bNumConfigurations});
            }
        }

        {
            const status = libusb_set_auto_detach_kernel_driver(device_handle, 1);
            if (FAILURE(status)) {
                print("libusb_set_auto_detach_kernel_driver FAILED with: {d}\n", .{status});
                print("CONTINUING ANYWAY\n", .{});
                // return error.Fail;
            } else {
                print("libusb_set_auto_detach_kernel_driver SUCCEEDED\n", .{});
            }
        }

        {
            const status = libusb_claim_interface(device_handle, 2);
            if (FAILURE(status)) {
                print("libusb_claim_interface FAILED WITH: {d}\n", .{status});
                return error.Fail;
            }
            print("libusb_claim_interface SUCCEEDED\n", .{});
            is_open = true;
        }

        return HidClient{
            .vid = vid,
            .pid = pid,
            .is_open = is_open,
            .usb_context = ctx,
            .device_handle = device_handle,
            .device = device,
        };
    }
};

test "HidClient open" {
    // TODO(lbayes): Figure out how to fake/mock the real
    // USB interface.
    //
    // const client = try HidClient.open(0x335e, 0x8a01);
    // defer client.close();

    // try expectEqual(client.vid, 0x335e);
    // try expectEqual(client.pid, 0x8a01);
}

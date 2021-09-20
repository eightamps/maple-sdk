//
// Created by lukebayes on 9/20/2021
//

#ifndef MAPLE_HID_STATUS_H
#define MAPLE_HID_STATUS_H

#include <libusb-1.0/libusb.h>

enum hid_status {
  /** Operation not supported or unimplemented on this platform */
  // LIBUSB_ERROR_NOT_SUPPORTED = -12
  HID_ERROR_NOT_SUPPORTED = LIBUSB_ERROR_NOT_SUPPORTED + -256,
  // error space down
  /** Insufficient memory */
  HID_ERROR_NO_MEM,
  /** System call interrupted (perhaps due to signal) */
  HID_ERROR_INTERRUPTED,
  /** Pipe error */
  HID_ERROR_PIPE,
  /** Overflow */
  HID_ERROR_OVERFLOW,
  /** Operation timed out */
  HID_ERROR_TIMEOUT,
  /** Resource busy */
  HID_ERROR_BUSY,
  /** Entity not found */
  HID_ERROR_NOT_FOUND,
  /** No such device (it may have been disconnected) */
  HID_ERROR_NO_DEVICE,
  /** Access denied (insufficient permissions) */
  HID_ERROR_ACCESS,
  /** Invalid parameter */
  HID_ERROR_INVALID_PARAM,
  /** Input/output error */
  HID_ERROR_IO,
  /** Success (no error) */
  HID_SUCCESS = 0,
  /** Other error */
  HID_ERROR_OTHER = -99,
};

/** \ingroup libusb_asyncio
 * Transfer status codes */
enum hid_transfer_status {
  /** Transfer failed */
  HID_TRANSFER_ERROR = -128,

  /** Transfer timed out */
  HID_TRANSFER_TIMED_OUT,

  /** Transfer was cancelled */
  HID_TRANSFER_CANCELLED,

  /** For bulk/interrupt endpoints: halt condition detected (endpoint
   * stalled). For control endpoints: control request not supported. */
  HID_TRANSFER_STALL,

  /** Device was disconnected */
  HID_TRANSFER_NO_DEVICE,

  /** Device sent more data than requested */
  HID_TRANSFER_OVERFLOW,
};


int hid_status_from_libusb(int status);
const char *hid_status_message(int status);

#endif // MAPLE_HID_STATUS_H


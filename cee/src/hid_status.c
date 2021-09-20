
#include "hid_status.h"
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

int hid_status_from_libusb(int status) {
  if (status < LIBUSB_SUCCESS && status > LIBUSB_ERROR_OTHER) {
    // We have a libusb status
    return status - 256;
  } else if (status > LIBUSB_SUCCESS && status <= LIBUSB_TRANSFER_OVERFLOW) {
    // We have a libusb transfer status
    return status - 129;
  }

  return status;
}

const char *hid_status_message(int status) {
  switch (status) {
    case HID_SUCCESS:
      return "HID_SUCCESS";
    case HID_ERROR_IO:
      return "HID_ERROR_IO";
    case HID_ERROR_INVALID_PARAM:
      return "HID_ERROR_INVALID_PARAM";
    case HID_ERROR_ACCESS:
      return "HID_ERROR_ACCESS";
    case HID_ERROR_NO_DEVICE:
      return "HID_ERROR_NO_DEVICE";
    case HID_ERROR_NOT_FOUND:
      return "HID_ERROR_NOT_FOUND";
    case HID_ERROR_BUSY:
      return "HID_ERROR_BUSY";
    case HID_ERROR_TIMEOUT:
      return "HID_ERROR_TIMEOUT";
    case HID_ERROR_OVERFLOW:
      return "HID_ERROR_OVERFLOW";
    case HID_ERROR_PIPE:
      return "HID_ERROR_PIPE";
    case HID_ERROR_INTERRUPTED:
      return "HID_ERROR_INTERRUPTED";
    case HID_ERROR_NO_MEM:
      return "HID_ERROR_NO_MEM";
    case HID_ERROR_NOT_SUPPORTED:
      return "HID_ERROR_NOT_SUPPORTED";
    case HID_ERROR_OTHER:
      return "HID_ERROR_OTHER";
    case HID_TRANSFER_ERROR:
      return "HID_TRANSFER_ERROR";
    case HID_TRANSFER_TIMED_OUT:
      return "HID_TRANSFER_TIMED_OUT";
    case HID_TRANSFER_CANCELLED:
      return "HID_TRANSFER_CANCELLED";
    case HID_TRANSFER_STALL:
      return "HID_TRANSFER_STALL";
    case HID_TRANSFER_NO_DEVICE:
      return "HID_TRANSFER_NO_DEVICE";
    case HID_TRANSFER_OVERFLOW:
      return "HID_TRANSFER_OVERFLOW";
    default:
      return "HID_UNKNOWN_ERROR";
  }
}



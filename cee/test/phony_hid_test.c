//
// Created by lukebayes on 4/23/21.
//

#include "../src/phony_hid.h"
#include "minunit.h"
#include "phony_hid_test.h"
#include "test_helper.h"
#include <string.h>

char *test_hid_in_report_to_struct(void) {
  phony_hid_in_report_t *in = calloc(sizeof(phony_hid_in_report_t), 1);
  muAssert(in != NULL, "Expected in report");

  phony_hid_in_report_to_struct(in, 0x01);
  muAssert(in->loop, "Expected loop");
  muAssert(in->ring == 0, "Expected no ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x02);
  muAssert(in->loop == 0, "Expected no loop");
  muAssert(in->ring, "Expected ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x03);
  muAssert(in->loop, "Expected loop");
  muAssert(in->ring, "Expected ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x04);
  muAssert(in->loop == 0, "Expected no loop");
  muAssert(in->ring == 0, "Expected ring");
  muAssert(in->line_not_found == 1, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x05);
  muAssert(in->loop == 1, "Expected no loop");
  muAssert(in->ring == 0, "Expected ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 1, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  free(in);
  return NULL;
}

char *test_phony_hid_new(void) {
  phony_hid_context_t *c = phony_hid_new();
  muAssert(c != NULL, "Expected Not Null");
  phony_hid_free(c);
  return NULL;
}

char *test_phony_hid_open_not_found(void) {
  phony_hid_context_t *c = phony_hid_new();
  int status = phony_hid_open(c);
  // Until we configure the fake to return a device and device_handle.
  muAssert(status == HID_ERROR_NOT_FOUND, "Expected Not found");
  phony_hid_free(c);
  return NULL;
}

char *test_phony_hid_libusb_error_codes(void) {
  // Ensure we have mapped LIBUSB error (and status, and transfer) codes to
  // the expected PHONY_HID status code AND resulting user message.
  LUSB_STATUS_MSG(LIBUSB_SUCCESS, "HID_SUCCESS");
  LUSB_STATUS_MSG(LIBUSB_ERROR_NOT_SUPPORTED, "HID_ERROR_NOT_SUPPORTED");
  LUSB_STATUS_MSG(LIBUSB_ERROR_NO_MEM, "HID_ERROR_NO_MEM");
  LUSB_STATUS_MSG(LIBUSB_ERROR_INTERRUPTED, "HID_ERROR_INTERRUPTED");
  LUSB_STATUS_MSG(LIBUSB_ERROR_PIPE, "HID_ERROR_PIPE");
  LUSB_STATUS_MSG(LIBUSB_ERROR_OVERFLOW, "HID_ERROR_OVERFLOW");
  LUSB_STATUS_MSG(LIBUSB_ERROR_TIMEOUT, "HID_ERROR_TIMEOUT");
  LUSB_STATUS_MSG(LIBUSB_ERROR_BUSY, "HID_ERROR_BUSY");
  LUSB_STATUS_MSG(LIBUSB_ERROR_NOT_FOUND, "HID_ERROR_NOT_FOUND");
  LUSB_STATUS_MSG(LIBUSB_ERROR_NO_DEVICE, "HID_ERROR_NO_DEVICE");
  LUSB_STATUS_MSG(LIBUSB_ERROR_ACCESS, "HID_ERROR_ACCESS");
  LUSB_STATUS_MSG(LIBUSB_ERROR_INVALID_PARAM, "HID_ERROR_INVALID_PARAM");
  LUSB_STATUS_MSG(LIBUSB_ERROR_IO, "HID_ERROR_IO");
  LUSB_STATUS_MSG(LIBUSB_ERROR_OTHER, "HID_ERROR_OTHER");

  // Transfer codes
  LUSB_STATUS_MSG(LIBUSB_TRANSFER_COMPLETED, "HID_SUCCESS");
  LUSB_STATUS_MSG(LIBUSB_TRANSFER_ERROR, "HID_TRANSFER_ERROR");
  LUSB_STATUS_MSG(LIBUSB_TRANSFER_OVERFLOW, "HID_TRANSFER_OVERFLOW");
  LUSB_STATUS_MSG(LIBUSB_TRANSFER_TIMED_OUT, "HID_TRANSFER_TIMED_OUT");
  LUSB_STATUS_MSG(LIBUSB_TRANSFER_CANCELLED, "HID_TRANSFER_CANCELLED");
  LUSB_STATUS_MSG(LIBUSB_TRANSFER_STALL, "HID_TRANSFER_STALL");
  LUSB_STATUS_MSG(LIBUSB_TRANSFER_NO_DEVICE, "HID_TRANSFER_NO_DEVICE");
  return NULL;
}

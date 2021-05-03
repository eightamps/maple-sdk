//
// Created by lukebayes on 4/23/21.
//

#ifndef MAPLE_PHONY_HID_H
#define MAPLE_PHONY_HID_H

#include <stdbool.h>
#ifndef TEST_MODE
#include <libusb-1.0/libusb.h>
#else
#include "fakes/libusb_fake.h"
#endif

typedef struct {
  uint8_t loop; // 0
  uint8_t ring; // 1
  uint8_t line_not_found; // 2
  uint8_t line_in_use; // 3
  uint8_t polarity; // 4
  uint8_t unused__;
  uint8_t last_b__;
  uint8_t state;
}phony_hid_in_report_t;

typedef struct {
  uint8_t host_avail; // 0
  uint8_t off_hook; // 1
  uint8_t unused__;
  uint8_t last_b__;
}phony_hid_out_report_t;

typedef struct {
  phony_hid_out_report_t *out_report;
  phony_hid_in_report_t *in_report;
  bool is_open;
  bool is_interface_claimed;
  int vendor_id;
  int product_id;
  libusb_context *lusb_context;
  libusb_device_handle *device_handle;
  libusb_device *device;
  struct libusb_config_descriptor *config_descriptor;
  struct libusb_device_descriptor *device_descriptor;
}phony_hid_context_t;

enum phony_hid_error {
  /** Operation not supported or unimplemented on this platform */
  // LIBUSB_ERROR_NOT_SUPPORTED = -12
  PHONY_HID_ERROR_NOT_SUPPORTED = LIBUSB_ERROR_NOT_SUPPORTED + -256,
  // error space down
  /** Insufficient memory */
  PHONY_HID_ERROR_NO_MEM,
  /** System call interrupted (perhaps due to signal) */
  PHONY_HID_ERROR_INTERRUPTED,
  /** Pipe error */
  PHONY_HID_ERROR_PIPE,
  /** Overflow */
  PHONY_HID_ERROR_OVERFLOW,
  /** Operation timed out */
  PHONY_HID_ERROR_TIMEOUT,
  /** Resource busy */
  PHONY_HID_ERROR_BUSY,
  /** Entity not found */
  PHONY_HID_ERROR_NOT_FOUND,
  /** No such device (it may have been disconnected) */
  PHONY_HID_ERROR_NO_DEVICE,
  /** Access denied (insufficient permissions) */
  PHONY_HID_ERROR_ACCESS,
  /** Invalid parameter */
  PHONY_HID_ERROR_INVALID_PARAM,
  /** Input/output error */
  PHONY_HID_ERROR_IO,
  /** Success (no error) */
  PHONY_HID_SUCCESS = 0,
  /** Other error */
  PHONY_HID_ERROR_OTHER = -99,
};

/** \ingroup libusb_asyncio
 * Transfer status codes */
enum phony_hid_transfer_status {
  /** Transfer failed */
  PHONY_HID_TRANSFER_ERROR = -128,

  /** Transfer timed out */
  PHONY_HID_TRANSFER_TIMED_OUT,

  /** Transfer was cancelled */
  PHONY_HID_TRANSFER_CANCELLED,

  /** For bulk/interrupt endpoints: halt condition detected (endpoint
   * stalled). For control endpoints: control request not supported. */
  PHONY_HID_TRANSFER_STALL,

  /** Device was disconnected */
  PHONY_HID_TRANSFER_NO_DEVICE,

  /** Device sent more data than requested */
  PHONY_HID_TRANSFER_OVERFLOW,
};

const char *phony_hid_status_message(int status);

int phony_hid_status_from_libusb(int status);

/**
 * Create a new HID client context.
 *
 * @return phony_hid_context_t*
 */
phony_hid_context_t *phony_hid_new(void);

int phony_hid_in_report_to_struct(phony_hid_in_report_t *in_report, uint8_t value);

int phony_hid_set_vendor_id(phony_hid_context_t *c, int vid);

int phony_hid_set_product_id(phony_hid_context_t *c, int pid);

int phony_hid_open(phony_hid_context_t *c);

int phony_hid_get_report(phony_hid_context_t *c);

int phony_hid_set_off_hook(phony_hid_context_t *c, bool is_offhook);

int phony_hid_set_hostavail(phony_hid_context_t *c, bool is_hostavail);

int phony_hid_reset_device(phony_hid_context_t *c);

int phony_hid_close(phony_hid_context_t *c);

int phony_hid_get_configuration_descriptors(phony_hid_context_t *c);

void phony_hid_free(phony_hid_context_t *c);

#endif // MAPLE_PHONY_HID_H

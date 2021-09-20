//
// Created by lukebayes on 4/23/21.
//

#ifndef MAPLE_PHONY_HID_H
#define MAPLE_PHONY_HID_H

#include "hid_client.h"
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

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

//
// Created by lukebayes on 4/23/21.
//

#ifndef MAPLE_PHONY_HID_H
#define MAPLE_PHONY_HID_H

#include <libusb-1.0/libusb.h>
#include <stdbool.h>

typedef struct PhonyHidInReport {
  uint8_t loop; // 0
  uint8_t ring; // 1
  uint8_t line_not_found; // 2
  uint8_t line_in_use; // 3
  uint8_t polarity; // 4
  uint8_t unused__;
  uint8_t last_b__;
  uint8_t state;
} PhonyHidInReport;

typedef struct PhonyHidOutReport {
  uint8_t host_avail; // 0
  uint8_t off_hook; // 1
  uint8_t unused__;
  uint8_t last_b__;
} PhonyHidOutReport;

typedef struct PhonyHidContext {
  struct PhonyHidOutReport *out_report;
  struct PhonyHidInReport *in_report;
  bool is_open;
  bool is_interface_claimed;
  int vendor_id;
  int product_id;
  libusb_context *lusb_context;
  libusb_device_handle *device_handle;
  libusb_device *device;
  struct libusb_config_descriptor *config_descriptor;
  struct libusb_device_descriptor *device_descriptor;
}PhonyHidContext;

/**
 * Create a new HID client context.
 *
 * @return PhonyHidContext*
 */
struct PhonyHidContext *phony_hid_new(void);

int phony_hid_in_report_to_struct(PhonyHidInReport *in_report, uint8_t value);

int phony_hid_set_vendor_id(struct PhonyHidContext *c, int vid);

int phony_hid_set_product_id(struct PhonyHidContext *c, int pid);

int phony_hid_open(struct PhonyHidContext *c);

int phony_hid_get_report(struct PhonyHidContext *c);

int phony_hid_set_off_hook(struct PhonyHidContext *c, bool is_offhook);

int phony_hid_set_hostavail(struct PhonyHidContext *c, bool is_hostavail);

int phony_hid_close(struct PhonyHidContext *c);

void phony_hid_free(struct PhonyHidContext *c);

#endif // MAPLE_PHONY_HID_H

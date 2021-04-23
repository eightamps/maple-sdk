//
// Created by lukebayes on 4/23/21.
//

#ifndef MAPLE_PHONY_HID_H
#define MAPLE_PHONY_HID_H

#include <libusb-1.0/libusb.h>
#include <stdbool.h>

#define EIGHT_AMPS_VID 0x335e
#define MAPLE_V3_PID 0x8a01

typedef struct PhonyInReport {
  uint8_t loop;
  uint8_t ring;
  uint8_t ring2;
  uint8_t line_in_use;
  uint8_t polarity;
  uint8_t unused__;
  uint8_t last_b__;
  uint8_t state;
} PhonyInReport;

typedef struct PhonyOutReport {
  uint8_t host_avail;
  uint8_t off_hook;
  uint8_t unused__;
  uint8_t last_b__;
}PhonyOutReport;

typedef enum PhonyHidState {
  PHONY_NOT_READY = 0,
  PHONY_READY,
  PHONY_OFF_HOOK,
  PHONY_RINGING,
  PHONY_LINE_NOT_FOUND,
  PHONY_LINE_IN_USE,
  PHONY_HOST_NOT_FOUND,
}PhonyHidState;

typedef struct PhonyHidContext {
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
 * Get a human readable label from an HID Phony state value.
 * @param state
 * @return const char *label
 */
const char *phony_hid_state_to_str(int state);

/**
 * Create a new HID client context.
 *
 * @return PhonyHidContext*
 */
struct PhonyHidContext *phony_hid_new(void);

int phony_hid_set_vendor_id(struct PhonyHidContext *c, int vid);

int phony_hid_set_product_id(struct PhonyHidContext *c, int pid);

int phony_hid_open(struct PhonyHidContext *c);

int phony_hid_close(struct PhonyHidContext *c);

int phony_hid_update_report(struct PhonyHidContext *c);

int phony_hid_set_offhook(struct PhonyHidContext *c, bool is_offhook);

int phony_hid_set_hostavail(struct PhonyHidContext *c, bool is_hostavail);

int phony_hid_reset_device(struct PhonyHidContext *c);

int phony_hid_free(struct PhonyHidContext *c);

#endif // MAPLE_PHONY_HID_H

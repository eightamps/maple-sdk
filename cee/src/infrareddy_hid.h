//
// Created by lukebayes on 9/20/2021
//

#ifndef MAPLE_INFRAREDDY_HID_H
#define MAPLE_INFRAREDDY_HID_H

#include "hid_status.h"
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

typedef struct {
  uint16_t id;
  uint32_t type;
  uint16_t len;
  uint8_t *data;
}infrareddy_encode_request_t;

typedef struct {
  uint16_t id;
}infrareddy_decode_request_t;

typedef struct {
  uint16_t id;
  uint16_t tag;
  uint32_t status;
}infrareddy_hid_in_report_t;

typedef struct {
  uint16_t id; // 0x50
  uint32_t data_protocol; // 0x60
  uint16_t len; // 0x61
  uint8_t *data; // 0x62
}infrareddy_decode_report_t;

typedef struct {
  infrareddy_encode_request_t *encode_request;
  infrareddy_decode_request_t *decode_request;
  infrareddy_decode_report_t *decode_report;
  infrareddy_hid_in_report_t *in_report;
  bool is_open;
  bool is_interface_claimed;
  int vendor_id;
  int product_id;
  libusb_context *lusb_context;
  libusb_device_handle *device_handle;
  libusb_device *device;
  struct libusb_config_descriptor *config_descriptor;
  struct libusb_device_descriptor *device_descriptor;
}infrareddy_hid_context_t;

/**
 * Create a new HID client context.
 *
 * @return infrareddy_hid_context_t*
 */
infrareddy_hid_context_t *infrareddy_hid_new(void);

int infrareddy_hid_set_vendor_id(infrareddy_hid_context_t *c, int vid);

int infrareddy_hid_set_product_id(infrareddy_hid_context_t *c, int pid);

int infrareddy_hid_open(infrareddy_hid_context_t *c);

int infrareddy_hid_encode(infrareddy_hid_context_t *c, uint16_t len, unsigned char *data);

int infrareddy_hid_get_report(infrareddy_hid_context_t *c);

void infrareddy_hid_free(infrareddy_hid_context_t *c);

int infrareddy_hid_reset_device(infrareddy_hid_context_t *c);


#endif // MAPLE_INFRAREDDY_HID_H

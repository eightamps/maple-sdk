//
// Created by lukebayes on 4/17/21.
//

#include "phony.h"
#include <errno.h>
#include <hidapi/hidapi.h>
#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 255


PhonyContext *phony_new(void) {
  return phony_new_with_vid_and_pid(EIGHT_AMPS_VID, MAPLE_V3_PID);
}

PhonyContext *phony_new_with_vid_and_pid(uint16_t vid, uint16_t pid) {
  // printf("Attempting to allocate PhonyContext client\n");
  PhonyContext *ref = malloc(sizeof(PhonyContext));
  if (ref == NULL) {
    // printf("ERROR: PhonyContext unable to allocate\n");
    // TODO(lbayes): Probably should not be calling exit() from within a
    //  shared library.
    exit(ENOMEM);
  }
  ref->vid = vid;
  ref->pid = pid;

  return ref;
}

int phony_init(PhonyContext *c) {
  int res = hid_init();
  // printf("PhonyContext hid_init result: %d\n", res);
  if (res != EXIT_SUCCESS) return res;

  c->device = hid_open(c->vid, c->pid, NULL);
  if (c->device == NULL) return ECONNREFUSED;

  return EXIT_SUCCESS;
}

int phony_take_off_hook(PhonyContext *phony) {
  return -1;
}

int phony_hang_up(PhonyContext *phony) {
  return -1;
}

int phony_dial(PhonyContext *c, const char *numbers) {
  printf("phony_dial called with %s\n", numbers);

  hid_device *device = c->device;
  int report_id = 0x03;
  int status = EXIT_SUCCESS;

  size_t data_size = 3;
  unsigned char *data = malloc(data_size);
  memset(data, 0x0, data_size);
  data[0] = report_id;
  data[1] = 0;
  data[2] = 0x11;

  status = hid_write(device, data, data_size);
  printf("GET Report 2 response: %d \n", status);
  for (int i = 0; i < data_size; i++) {
    printf("DATA: %d 0x%02x\n", i, data[i]);
  }

  /*
  memset(data, 0x0, data_size);
  data[0] = report_id;
  data[1] = 0x00;
  status = hid_get_feature_report(device, data, data_size);
  printf("SEND Report 1 response: %d \n", status);
  for (int i = 0; i < data_size; i++) {
    printf("DATA: %d 0x%02x\n", i, data[i]);
  }
   */


  /*
  libusb_descr
  size_t r_data_size = 8;
  unsigned char *r_data = malloc(r_data_size);
  memset(r_data, 0x0, r_data_size);

  printf("About to read:\n");
  status = hid_read(device, r_data, r_data_size);
  if (status != EXIT_SUCCESS) {
    printf("READ FAILED\n");
  } else {
    printf("READ SUCCEEDED\n");
  }
  printf("READ COMPLETE WITH: %02x\n",  r_data[0]);


  size_t size = 18;
  unsigned char *data = malloc(size);
  memset(data, 0x0, size); // Clear the allocated region.

  int status = hid_get_feature_report(c->device, data, 18);
  printf("STATUS %d\n", status);
  for (int i = 0; i < size; i++) {
    printf("%04d 0x%02x\n", i, data[i]);
  }
   */
  // hid_send_feature_report(c->device, data, size);

  // struct PhonyInReport *in_report = (PhonyOutReport *)data;
  // printf("YOOOOOOOOOOOOOOOOOOOOOO\n");
  // printf("loop: %d\n", in_report->loop);
  // printf("ring: %d\n", in_report->ring);
  // printf("line_in_use: %d\n", in_report->line_in_use);
}

int phony_info(PhonyContext *c) {
  // TODO(lbayes): Add these fields to the PhonyContext struct and apply these
  //  entries to those fields.
  int res;
  wchar_t str[MAX_STR];
  // unsigned char buf[65];
  hid_device *device = c->device;
  res = hid_get_manufacturer_string(device, str, MAX_STR);
  if (res != EXIT_SUCCESS) return ECOMM;
  wprintf(L"Manufacturer String status: %d, value: %ls\n", res, str);

  res = hid_get_product_string(device, str, MAX_STR);
  if (res != EXIT_SUCCESS) return ECOMM;
  wprintf(L"Product String status: %d, value: %ls\n", res, str);

  // TODO(lbayes): I'm getting junk characters out of this API call. Ensure we
  //  write a unique serial number to the firmware.
  // res = hid_get_serial_number_string(device, str, MAX_STR);
  // if (res != EXIT_SUCCESS) return ECOMM;
  // wprintf(L"Serial Number status: %d, value: %ls\n", res, str);

  // Not sure if this is safe on all HID devices yet.
  // res = hid_get_indexed_string(device, 0, str, MAX_STR);
  // if (res != EXIT_SUCCESS) return ECOMM;
  // wprintf(L"Indexed String 1 status: %d, value: %s\n", res, str);

  return EXIT_SUCCESS;
}

void phony_free(PhonyContext *phony) {
  free(phony);
}

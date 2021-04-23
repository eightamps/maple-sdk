//
// Created by lukebayes on 4/17/21.
//

#include "phony.h"
#include <errno.h>
#include <hidapi/hidapi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

int phony_init(PhonyContext *phony) {
  int res = hid_init();
  // printf("PhonyContext hid_init result: %d\n", res);
  if (res != EXIT_SUCCESS) return res;

  phony->device = hid_open(phony->vid, phony->pid, NULL);
  if (phony->device == NULL) return ECONNREFUSED;

  return EXIT_SUCCESS;
}

int phony_dial(PhonyContext *phony, const char *numbers) {
  printf("Attempting to dial [%s]\n", numbers);
}

int phony_info(PhonyContext *phony) {
  // TODO(lbayes): Add these fields to the PhonyContext struct and apply these
  //  entries to those fields.
  int res;
  wchar_t str[MAX_STR];
  // unsigned char buf[65];
  hid_device *device = phony->device;
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

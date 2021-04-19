//
// Created by lukebayes on 4/17/21.
//

#include "phony.h"
#include <hidapi/hidapi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_STR 255


PhonyStatus phony_free(Phony *phony) {
  free(phony);
  return PhonyStatusSuccess;
}

Phony *phony_new(uint16_t vid, uint16_t pid) {
  // printf("Attempting to allocate Phony client\n");
  Phony *ref = malloc(sizeof(Phony));
  if (ref == NULL) {
    // printf("ERROR: Phony unable to allocate\n");
    exit(1);
  }
  ref->vid = vid;
  ref->pid = pid;

  return ref;
}

PhonyStatus phony_init(Phony *phony) {
  int res = hid_init();
  // printf("Phony hid_init result: %d\n", res);
  if (res != 0) {
    // printf("ERROR: Failed to initialize HID device library\n");
    return PhonyStatusFailureHidInit;
  }

  phony->device = hid_open(phony->vid, phony->pid, NULL);
  if (phony->device == NULL) {
    // printf("ERROR: Failed to connect to device width vid: 0x%x and pid: 0x%x\n",
           // phony->vid,
           // phony->pid);
    return PhonyStatusFailureDeviceConnect;
  }

  return PhonyStatusSuccess;
}

PhonyStatus phony_info(Phony *phony) {
  // TODO(lbayes): Add these fields to the Phony struct and apply these
  //  values to those fields.
  int res;
  wchar_t *str[MAX_STR];
  // unsigned char buf[65];
  hid_device *device = phony->device;
  res = hid_get_manufacturer_string(device, str, MAX_STR);
  if (res != 0) return PhonyStatusFailureCommunication;
  wprintf(L"Manufacturer String status: %d, value: %ls\n", res, str);

  res = hid_get_product_string(device, str, MAX_STR);
  if (res != 0) return PhonyStatusFailureCommunication;
  wprintf(L"Product String status: %d, value: %ls\n", res, str);

  // TODO(lbayes): I'm getting junk characters out of this API call. Ensure we
  //  write a unique serial number to the firmware.
  // res = hid_get_serial_number_string(device, str, MAX_STR);
  // wprintf(L"Serial Number status: %d, value: %ls\n", res, str);

  // Not sure if this is safe on all HID devices yet.
  // res = hid_get_indexed_string(device, 0, str, MAX_STR);
  // wprintf(L"Indexed String 1 status: %d, value: %s\n", res, str);

  return PhonyStatusSuccess;
}

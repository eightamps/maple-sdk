//
// Created by lukebayes on 4/30/21.
//

#ifndef MAPLE_HID_CLIENT_H
#define MAPLE_HID_CLIENT_H

#ifdef TEST_MODE
#include "fakes/libusb_fake.h"
#else
#include <libusb-1.0/libusb.h>
#endif

typedef struct {
  int vid;
  int pid;
  libusb_context *usb_context;
  libusb_device_handle *usb_device_handle;
  libusb_device *usb_device;
}hid_client_context_t;

hid_client_context_t *hid_client_new(void);

int hid_client_open(hid_client_context_t *c);

int hid_client_set_vid(hid_client_context_t *c, int vid);
int hid_client_get_vid(hid_client_context_t *c);

int hid_client_set_pid(hid_client_context_t *c, int pid);
int hid_client_get_pid(hid_client_context_t *c);

void hid_client_free(hid_client_context_t *c);

#endif //MAPLE_HID_CLIENT_H

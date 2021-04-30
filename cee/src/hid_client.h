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
}HidClientContext;

HidClientContext *hid_client_new(void);

int hid_client_open(HidClientContext *c);

int hid_client_set_vid(HidClientContext *c, int vid);
int hid_client_get_vid(HidClientContext *c);

int hid_client_set_pid(HidClientContext *c, int pid);
int hid_client_get_pid(HidClientContext *c);

void hid_client_free(HidClientContext *c);

#endif //MAPLE_HID_CLIENT_H

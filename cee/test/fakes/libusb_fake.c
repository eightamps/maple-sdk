//
// Created by lukebayes on 4/30/21.
//

#include "libusb_fake.h"
#include <stdlib.h>

static int next_result = LIBUSB_SUCCESS;

void libusb_fake_set_next_result(int result) {
  next_result = result;
}

int libusb_init(libusb_context **c) {
  return next_result;
}

int libusb_interrupt_transfer(struct libusb_device_handle *devh,
    unsigned char endpoint, unsigned char *data, int length, int *transferred,
    unsigned int timeout) {

  return LIBUSB_SUCCESS;
}

const char * libusb_error_name(int code) {
  return "FAKE-ERROR-NAME";
}

int libusb_set_auto_detach_kernel_driver(libusb_device_handle *devh,
                                         int enable) {
  return LIBUSB_SUCCESS;
}

int libusb_claim_interface(libusb_device_handle *devh, int interface_number) {
  return LIBUSB_SUCCESS;
}

libusb_device_handle * libusb_open_device_with_vid_pid(libusb_context *ctx,
                                                       uint16_t vid, uint16_t
                                                       pid) {
  return NULL;
}

libusb_device * libusb_get_device(libusb_device_handle *devh) {
  return NULL;
}

uint8_t libusb_get_bus_number(libusb_device *dev) {
  return 0;
}

uint8_t libusb_get_device_address(libusb_device *dev) {
  return 0;
}

int libusb_get_device_descriptor(libusb_device *dev,
                                 libusb_device_descriptor *desc) {
  return LIBUSB_SUCCESS;
}

int libusb_reset_device(libusb_device_handle *devh) {
  return LIBUSB_SUCCESS;
}

void libusb_exit(libusb_context *c) {
  free(c);
  next_result = LIBUSB_SUCCESS;
}

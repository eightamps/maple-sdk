//
// Created by lukebayes on 4/30/21.
//

#include "libusb.h"
#include <stdlib.h>

static int next_result = LIBUSB_SUCCESS;

void libusb_fake_set_next_result(int result) {
  next_result = result;
}

int libusb_init(libusb_context **context) {
  struct libusb_context *ctx;
  ctx = calloc(sizeof(libusb_context), 1);
  *context = (libusb_context *)ctx;
  return next_result;
}

int libusb_interrupt_transfer(struct libusb_device_handle *devh,
    unsigned char endpoint, unsigned char *data, int length, int *transferred,
    unsigned int timeout) {

  return LIBUSB_SUCCESS;
}

const char * libusb_error_name(int error_code) {
  switch (error_code) {
    case LIBUSB_ERROR_IO:
      return "LIBUSB_ERROR_IO";
    case LIBUSB_ERROR_INVALID_PARAM:
      return "LIBUSB_ERROR_INVALID_PARAM";
    case LIBUSB_ERROR_ACCESS:
      return "LIBUSB_ERROR_ACCESS";
    case LIBUSB_ERROR_NO_DEVICE:
      return "LIBUSB_ERROR_NO_DEVICE";
    case LIBUSB_ERROR_NOT_FOUND:
      return "LIBUSB_ERROR_NOT_FOUND";
    case LIBUSB_ERROR_BUSY:
      return "LIBUSB_ERROR_BUSY";
    case LIBUSB_ERROR_TIMEOUT:
      return "LIBUSB_ERROR_TIMEOUT";
    case LIBUSB_ERROR_OVERFLOW:
      return "LIBUSB_ERROR_OVERFLOW";
    case LIBUSB_ERROR_PIPE:
      return "LIBUSB_ERROR_PIPE";
    case LIBUSB_ERROR_INTERRUPTED:
      return "LIBUSB_ERROR_INTERRUPTED";
    case LIBUSB_ERROR_NO_MEM:
      return "LIBUSB_ERROR_NO_MEM";
    case LIBUSB_ERROR_NOT_SUPPORTED:
      return "LIBUSB_ERROR_NOT_SUPPORTED";
    case LIBUSB_ERROR_OTHER:
      return "LIBUSB_ERROR_OTHER";
    case LIBUSB_TRANSFER_ERROR:
      return "LIBUSB_TRANSFER_ERROR";
    case LIBUSB_TRANSFER_TIMED_OUT:
      return "LIBUSB_TRANSFER_TIMED_OUT";
    case LIBUSB_TRANSFER_CANCELLED:
      return "LIBUSB_TRANSFER_CANCELLED";
    case LIBUSB_TRANSFER_STALL:
      return "LIBUSB_TRANSFER_STALL";
    case LIBUSB_TRANSFER_NO_DEVICE:
      return "LIBUSB_TRANSFER_NO_DEVICE";
    case LIBUSB_TRANSFER_OVERFLOW:
      return "LIBUSB_TRANSFER_OVERFLOW";

    case 0:
      return "LIBUSB_SUCCESS / LIBUSB_TRANSFER_COMPLETED";
    default:
      return "**UNKNOWN**";
  }
}

int libusb_set_auto_detach_kernel_driver(libusb_device_handle *devh,
    int enable) {
  return LIBUSB_SUCCESS;
}

int libusb_claim_interface(libusb_device_handle *devh, int interface_number) {
  return LIBUSB_SUCCESS;
}

int libusb_release_interface(libusb_device_handle *devh, int interface_number) {
  return LIBUSB_SUCCESS;
}

void libusb_close(libusb_device_handle *devh) {
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

int libusb_get_config_descriptor(libusb_device *dev, uint8_t config_index,
    libusb_config_descriptor **config) {
  return LIBUSB_SUCCESS;
}

void libusb_exit(libusb_context *c) {
  free(c);
  next_result = LIBUSB_SUCCESS;
}

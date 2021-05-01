//
// Created by lukebayes on 4/30/21.
//

#ifndef MAPLE_LIBUSB_FAKE_H
#define MAPLE_LIBUSB_FAKE_H

#include <stdint.h>

typedef struct {
}libusb_context;

typedef struct libusb_device_handle {
}libusb_device_handle;

typedef struct libusb_device_descriptor {
  uint16_t idVendor;
  uint16_t idProduct;
}libusb_device_descriptor;

typedef struct {
}libusb_device;

/** \ingroup libusb_misc
 * Error codes. Most libusb functions return 0 on success or one of these
 * codes on failure.
 * You can call libusb_error_name() to retrieve a string representation of an
 * error code or libusb_strerror() to get an end-user suitable description of
 * an error code.
 */
enum libusb_error {
  /** Success (no error) */
  LIBUSB_SUCCESS = 0,

  /** Input/output error */
  LIBUSB_ERROR_IO = -1,

  /** Invalid parameter */
  LIBUSB_ERROR_INVALID_PARAM = -2,

  /** Access denied (insufficient permissions) */
  LIBUSB_ERROR_ACCESS = -3,

  /** No such device (it may have been disconnected) */
  LIBUSB_ERROR_NO_DEVICE = -4,

  /** Entity not found */
  LIBUSB_ERROR_NOT_FOUND = -5,

  /** Resource busy */
  LIBUSB_ERROR_BUSY = -6,

  /** Operation timed out */
  LIBUSB_ERROR_TIMEOUT = -7,

  /** Overflow */
  LIBUSB_ERROR_OVERFLOW = -8,

  /** Pipe error */
  LIBUSB_ERROR_PIPE = -9,

  /** System call interrupted (perhaps due to signal) */
  LIBUSB_ERROR_INTERRUPTED = -10,

  /** Insufficient memory */
  LIBUSB_ERROR_NO_MEM = -11,

  /** Operation not supported or unimplemented on this platform */
  LIBUSB_ERROR_NOT_SUPPORTED = -12,

  /* NB: Remember to update LIBUSB_ERROR_COUNT below as well as the
     message strings in strerror.c when adding new error codes here. */

  /** Other error */
  LIBUSB_ERROR_OTHER = -99,
};

// Fake-only functions
void libusb_fake_set_next_result(int result);

int libusb_init(libusb_context **context);
int libusb_interrupt_transfer(struct libusb_device_handle *devh,
    unsigned char endpoint, unsigned char *data, int length, int *transferred,
    unsigned int timeout);
const char * libusb_error_name(int code);
int libusb_set_auto_detach_kernel_driver(libusb_device_handle *devh,
                                         int enable);
int libusb_claim_interface(libusb_device_handle *devh, int interface_number);
libusb_device_handle * libusb_open_device_with_vid_pid(libusb_context *ctx,
    uint16_t vid, uint16_t pid);
libusb_device * libusb_get_device(libusb_device_handle *devh);
uint8_t libusb_get_bus_number(libusb_device *dev);
uint8_t libusb_get_device_address(libusb_device *dev);
int libusb_get_device_descriptor(libusb_device *dev,
                                 libusb_device_descriptor *desc);
int libusb_reset_device(libusb_device_handle *devh);

void libusb_exit(libusb_context *c);
#endif //MAPLE_LIBUSB_FAKE_H

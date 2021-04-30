//
// Created by lukebayes on 4/30/21.
//

#include "libusb_fake.h"
#include <stdlib.h>

static int next_result = EXIT_SUCCESS;

void libusb_fake_set_next_result(int result) {
  next_result = result;
}

int libusb_init(libusb_context *c) {
  return next_result;
}

void libusb_exit(libusb_context *c) {
  free(c);
  next_result = EXIT_SUCCESS;
}

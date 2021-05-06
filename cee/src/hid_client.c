//
// Created by lukebayes on 4/30/21.
//

#include "log.h"
#include "hid_client.h"
#include <stdlib.h>
#include <errno.h>

hid_client_context_t *hid_client_new(void) {
  hid_client_context_t *c = calloc(sizeof(hid_client_context_t), 1);
  if (c == NULL) {
    log_err("hid_client_new failed to allocated");
    return NULL;
  }

  return c;
}

int hid_client_open(hid_client_context_t *c) {
  if (c == NULL) {
    log_err("hid_client_open requires valid context");
    return -EINVAL;
  }

  libusb_context *usb_ctx = NULL;
  int status = libusb_init(&usb_ctx);
  c->usb_context = usb_ctx;

  if (status != EXIT_SUCCESS) {
    log_err("Failed to initialise libusb");
    return -ERROR_CONNECTION_REFUSED; // Connection refused
  }

  return EXIT_SUCCESS;
}

int hid_client_set_vid(hid_client_context_t *c, int vid) {
  if (c == NULL) {
    log_err("hid_client_set_vid requires valid context");
    return -EINVAL;
  }
  c->vid = vid;
  return EXIT_SUCCESS;
}

int hid_client_get_vid(hid_client_context_t *c) {
  if (c == NULL) {
    log_err("hid_client_get_vid requires valid context");
    return -EINVAL;
  }
  return c->vid;
}

int hid_client_set_pid(hid_client_context_t *c, int pid) {
  if (c == NULL) {
    log_err("hid_client_set_pid requires valid context");
    return -EINVAL;
  }
  c->pid = pid;
  return EXIT_SUCCESS;
}

int hid_client_get_pid(hid_client_context_t *c) {
  if (c == NULL) {
    log_err("hid_client_get_pid requires valid context");
    return -EINVAL;
  }
  return c->pid;
}

void hid_client_free(hid_client_context_t *c) {
  if (c != NULL) {
    if (c->usb_context != NULL) {
      libusb_exit(c->usb_context);
    }
    free(c);
  }
}

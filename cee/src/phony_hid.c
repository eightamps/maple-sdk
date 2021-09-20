//
// Created by lukebayes on 4/23/21.
//

#include "hid_status.h"
#include "hid_client.h"
#include "phony_hid.h"
#include "log.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#define MAPLE_PHONE_INTERFACE             2
#define PHONY_ENDPOINT_IN                 0x81
#define PHONY_ENDPOINT_OUT                0x01


static const int INFINITE_TIMEOUT = 0; /* timeout in ms, or zero for infinite */

static uint8_t struct_to_out_report(phony_hid_out_report_t *r) {
  log_info("struct_to_out_report with:");
  log_info("host_avail: %d", r->host_avail);
  log_info("off_hook: %d", r->off_hook);
  uint8_t state = 0;

  if (r->off_hook) {
    r->host_avail = true; // We always become available if we're going off hook.
    state = state | (2<<0);
  } else {
    state &= ~(2 << 0);
  }

  if (r->host_avail) {
    state = state | (1<<0);
  } else {
    state &= ~(1 << 0);
  }
  log_info("output: 0x%02x", state);
  return state;
}

int phony_hid_in_report_to_struct(phony_hid_in_report_t *in_report, uint8_t value) {
  in_report->loop = (value >> 0) & 1;
  in_report->ring = (value >> 1) & 1;
  in_report->line_not_found = (value >> 2) & 1;
  in_report->line_in_use = (value >> 3) & 1;
  in_report->polarity = (value >> 4) & 1;

  // TODO(lbayes): Found this condition experimentally while working on other
  //  issues. Figure out why this is happening. We're getting a firmware
  //  response that indicates loop = true and line_not_found = true, when we
  //  expect to see liu = true. It should not be possible to have liu = true
  //  and line_not_found = true simultaneously. Suspicious of bitshifting logic.
  //  The problem could be related to the "PACKED" macro in our firmware
  //  library.
  if (in_report->loop && in_report->line_not_found) {
    in_report->line_not_found = false;
    in_report->line_in_use = true;
  }

  log_info("in_report_to_struct with:");
  log_info("RAW VALUE: 0x%02x", value);
  log_info("loop: 0x%02x", in_report->loop);
  log_info("ring: 0x%02x", in_report->ring);
  log_info("line_not_found: 0x%02x", in_report->line_not_found);
  log_info("line_in_use: 0x%02x", in_report->line_in_use);
  log_info("polarity: 0x%02x", in_report->polarity);

  return HID_SUCCESS;
}

static int interrupt_transfer(phony_hid_context_t *c, uint8_t addr,
    unsigned char *data, uint8_t len) {
  int r;
  int transferred = 0;

  libusb_device_handle *dev_h = c->device_handle;

  r = libusb_interrupt_transfer(dev_h, addr, data, len, &transferred,
      INFINITE_TIMEOUT);
  if (r == LIBUSB_ERROR_NO_DEVICE ||
      r == LIBUSB_ERROR_IO) {
    phony_hid_close(c);
    return r;
  } else if (r != LIBUSB_SUCCESS) {
    log_err("Interrupt read error %d %s", r, libusb_error_name(r));
    return r;
  } else {
    log_info("Successfully interrupt_transferred %d bytes", transferred);

    if (transferred < len) {
      log_err("Interrupt transfer short read transferred %d bytes",
          transferred);
      return HID_ERROR_PIPE;
    }

    for (int i = 0; i < len; i++) {
      log_info("i: %d q: 0x%02x", i, data[i]);
    }
  }

  return r;
}

static int phony_hid_set_report(phony_hid_context_t *c) {
  uint8_t addr = PHONY_ENDPOINT_OUT;
  uint8_t len = 2 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);
  phony_hid_out_report_t *out = c->out_report;
  log_info("out_report->host_avail: %d", out->host_avail);
  log_info("out_report->off_hook: %d", out->off_hook);
  data[2] = struct_to_out_report(out);

  return interrupt_transfer(c, addr, data, len);
}

int phony_hid_get_report(phony_hid_context_t *c) {
  int status;
  uint8_t addr = PHONY_ENDPOINT_IN;
  uint8_t len = 1 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);

  status = interrupt_transfer(c, addr, data, len);
  log_info("phony_hid_get_report finished with status: %d", status);
  if (status == HID_SUCCESS) {
    phony_hid_in_report_to_struct(c->in_report, data[1]);
  }

  return status;
}

phony_hid_context_t *phony_hid_new(void) {
  phony_hid_context_t *c = calloc(sizeof(phony_hid_context_t), 1);
  if (c == NULL) {
    log_err("phony_hid_new unable to allocate");
    return NULL;
  }
  // Configure default vid and pid
  c->vendor_id = 0x0;
  c->product_id = 0x0;
  c->in_report = calloc(sizeof(phony_hid_in_report_t), 1);
  c->out_report = calloc(sizeof(phony_hid_out_report_t), 1);
  return c;
}

int phony_hid_set_vendor_id(phony_hid_context_t *c, int vid) {
  if (c == NULL) {
    log_err("phony_hid_set_vendor_id requires a valid context");
    return HID_ERROR_INVALID_PARAM;
  }
  c->vendor_id = vid;
  return HID_SUCCESS;
}

int phony_hid_set_product_id(phony_hid_context_t *c, int pid) {
  if (c == NULL) {
    log_err("phony_hid_set_product_id requires a valid context");
    return HID_ERROR_INVALID_PARAM;
  }
  c->product_id = pid;
  return HID_SUCCESS;
}

static int auto_detach_kernel(phony_hid_context_t *c) {
  libusb_device_handle *dev_h = c->device_handle;
  int status = libusb_set_auto_detach_kernel_driver(dev_h, 1);
  if (status != LIBUSB_SUCCESS) {
    log_err("libusb_set_auto_detach_kernel_driver = 0 failed with: "
        "%d", status);
  }
  log_info("Successfully called libusb_set_auto_detach_kernel_driver");
  return status;
}

static int claim_interface(phony_hid_context_t *c, int interface) {
  libusb_device_handle *dev_h = c->device_handle;
  int status = libusb_claim_interface(dev_h, interface);
  if (status < 0) {
    log_err("libusb_claim_interface error %d %s", status,
        libusb_error_name(status));
  }
  c->is_interface_claimed = true;
  log_info("Successfully claimed interface %d", interface);
  return status;
}

static int find_device(phony_hid_context_t *c, int vid, int pid) {
  libusb_context *lusb_ctx = c->lusb_context;
  struct libusb_device_handle *dev_h = NULL;
  dev_h = libusb_open_device_with_vid_pid(lusb_ctx, vid, pid);

  if (dev_h == NULL) {
    log_err("failed to find hid device with vid: 0x%02x and pid: 0x%02x",
        vid, pid);
    return HID_ERROR_NOT_FOUND;
  }

  c->device_handle = dev_h;

  libusb_device *d = libusb_get_device(dev_h);
  c->device = d;
  int bus_no = libusb_get_bus_number(d);
  int dev_addr = libusb_get_device_address(d);
  log_info("Found hid device at bus: 0x%02x (%d) and dev addr 0x%02x (%d)",
      bus_no, bus_no, dev_addr, dev_addr);

  struct libusb_device_descriptor desc = {0};
  int rc = libusb_get_device_descriptor(d, &desc);

  if (rc == LIBUSB_SUCCESS) {
    // log_info("idVendor: 0x%02x", desc.idVendor);
    // log_info("idProduct: 0x%02x", desc.idProduct);
    // log_info("iSerialNumber: %u", desc.iSerialNumber);
    // log_info("bNumConfigurations: %u", desc.bNumConfigurations);
    c->device_descriptor = &desc;
  }

  return HID_SUCCESS;
}

/*
   static int phony_hid_get_config_descriptors(phony_hid_context_t *c) {
   int status;
   libusb_context *lusb_ctx = c->lusb_context;
   struct libusb_config_descriptor *config;
   status = libusb_get_config_descriptor(c->device, 0, &config);
   if (status != LIBUSB_SUCCESS) {
   log_err("failed to get config descriptor with: %d", status);
   return status;
   }

   log_info("Successfully got config descriptor with:");
   log_info("bNumberInterfaces: %u", config->bNumInterfaces);
   log_info("bDescriptorType: %u", config->bDescriptorType);
   log_info("wTotalLength: %u", config->wTotalLength);

   for (int i = 0; i < config->bNumInterfaces; i++) {
   struct libusb_interface *interface = config->interface[i];
   const struct libusb_interface_descriptor *desc = interface.altsetting;
   if (desc->bInterfaceNumber == MAPLE_PHONE_INTERFACE) {
   log_info("==================================================");
   log_info("INTERFACE:");
   log_info("bInterfaceNumber: %u", desc->bInterfaceNumber);
   log_info("bDescriptorType: %u", desc->bDescriptorType);
   log_info("bAlternateSetting: %u", desc->bAlternateSetting);
   log_info("bInterfaceClass: %u", desc->bInterfaceClass);
   log_info("bInterfaceProtocol: %u", desc->bInterfaceProtocol);
   log_info("bLength: %u", desc->bLength);
   log_info("bNumEndpoints: %u", desc->bNumEndpoints);
   log_info("iInterface: %u", desc->iInterface);

   for (int k = 0; k < desc->bNumEndpoints; k++) {
   const struct libusb_endpoint_descriptor ep_desc = desc->endpoint[k];
   log_info("---");
   log_info("ENDPOINT:");
   uint8_t addr = ep_desc.bEndpointAddress;
   log_info("ep_desc->bEndpointAddress 0x%02x ", addr);
   if (addr & LIBUSB_ENDPOINT_IN) {
   log_info("(EP IN)");
   } else {
   log_info("(EP OUT)");
   }

   log_info("ep_desc->bLength %u", ep_desc.bLength);
   log_info("ep_desc->bDescriptorType %u", ep_desc.bDescriptorType);

   log_info("ep_desc->bInterval %u", ep_desc.bInterval);
   log_info("ep_desc->bSynchAddress %u", ep_desc.bSynchAddress);
   log_info("ep_desc->bmAttributes %u", ep_desc.bmAttributes);
   log_info("ep_desc->extra_length %d", ep_desc.extra_length);
   log_info("ep_desc->wMaxPacketSize %u", ep_desc.wMaxPacketSize);
   }
   }
   }

   return status;
   }
   */

int phony_hid_open(phony_hid_context_t *c) {
  if (c->is_open) {
    return HID_SUCCESS;
  }

  int status;

  if (c->lusb_context == NULL) {
    libusb_context *lusb_ctx;
    status = libusb_init(&lusb_ctx);
    if (status != HID_SUCCESS) {
      log_err("Failed to initialise libusb");
      goto out;
    }
    c->lusb_context = lusb_ctx;
  }

  status = find_device(c, c->vendor_id, c->product_id);
  if (status != HID_SUCCESS) {
    log_err("Could not open HID device at vid 0x%02x and pid "
      "0x%02x", c->vendor_id, c->product_id);
    goto out;
  }
  log_info("Successfully found the expected HID device");

  status = auto_detach_kernel(c); // enable auto-detach
  if (status != HID_SUCCESS) {
    goto out;
  }

  status = claim_interface(c, MAPLE_PHONE_INTERFACE);
  if (status != HID_SUCCESS) {
    goto out;
  }

  c->is_open = true;

  status = phony_hid_set_hostavail(c, true);
  if (status != HID_SUCCESS) {
    log_err("phony unable to set hostavail: %d", status);
    goto out;
  }
  log_info("Succesfully set hostavail on HID device\n");

out:
  return hid_status_from_libusb(status);
}

int phony_hid_close(phony_hid_context_t *c) {
  if (c->is_open) {
    libusb_device_handle *dev_h = c->device_handle;
    if (dev_h != NULL) {
      if (c->is_interface_claimed) {
        int status = libusb_release_interface(dev_h, MAPLE_PHONE_INTERFACE);
        if (status != 0) {
          log_err("libusb_release_interface error %d", status);
        }
      }

      // NOTE(lbayes): We cannot close the libusb device because we just reset
      libusb_close(dev_h);
      c->is_open = false;
    }
  }
  log_info("Exiting now");
  return HID_SUCCESS;
}

int phony_hid_reset_device(phony_hid_context_t *c) {
  if (c->is_open) {
    libusb_device_handle *dev_h = c->device_handle;
    if (dev_h != NULL) {
      log_info("phony_hid reset device now");
      int status = libusb_reset_device(c->device_handle);
      if (status != LIBUSB_SUCCESS) {
        log_err("phony_hid reset_device error %d %s", status,
            libusb_error_name(status));
      } else {
        log_info("phony_hid successfully reset device");
      }

      c->is_open = false;
    }
  }

  return HID_SUCCESS;
}

int phony_hid_set_hostavail(phony_hid_context_t *c, bool is_hostavail) {
  log_info("phony_hid_set_hostavail to: %d", is_hostavail);
  c->out_report->host_avail = is_hostavail;
  return phony_hid_set_report(c);
}

int phony_hid_set_off_hook(phony_hid_context_t *c, bool is_offhook) {
  log_info("phony_Hid_set_offhook to: %d", is_offhook);
  c->out_report->off_hook = is_offhook;
  return phony_hid_set_report(c);
}

void phony_hid_free(phony_hid_context_t *c) {
  if (c != NULL) {
    phony_hid_reset_device(c);

    if (c->lusb_context) {
      libusb_exit(c->lusb_context);
    }
    if (c->in_report != NULL) {
      free(c->in_report);
    }
    if (c->out_report != NULL) {
      free(c->out_report);
    }
    free(c);
  }
}

//
// Created by lukebayes on 4/23/21.
//

#include "phony_hid.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MAPLE_PHONE_INTERFACE             2
#define PHONY_ENDPOINT_IN                 0x81
#define PHONY_ENDPOINT_OUT                0x01

const static int INFINITE_TIMEOUT = 0; /* timeout in ms, or zero for infinite */

static uint8_t struct_to_out_report(PhonyHidOutReport *r) {
  printf("struct_to_out_report with:\n");
  printf("host_avail: %d\n", r->host_avail);
  printf("off_hook: %d\n", r->off_hook);
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
  printf("output: 0x%02x\n", state);
  return state;
}

static int in_report_to_struct(PhonyHidInReport *in_report, uint8_t value) {
  in_report->loop = (value >> 0) & 1;
  in_report->ring = (value >> 1) & 1;
  in_report->line_in_use = (value >> 2) & 1;
  in_report->polarity = (value >> 3) & 1;

  printf("in_report_to_struct with:\n");
  printf("INPUT: 0x%02x\n", value);
  printf("loop: 0x%02x\n", in_report->loop);
  printf("ring: 0x%02x\n", in_report->ring);
  printf("ring2: 0x%02x\n", in_report->ring2);
  printf("line_in_use: 0x%02x\n", in_report->line_in_use);
  printf("polarity: 0x%02x\n", in_report->polarity);

  return EXIT_SUCCESS;
}

static int interrupt_transfer(struct PhonyHidContext *c, uint8_t addr,
                              unsigned char *data, uint8_t len) {
  int r = EXIT_SUCCESS;
  int transferred = 0;

  libusb_device_handle *dev_h = c->device_handle;

  r = libusb_interrupt_transfer(dev_h, addr, data, len, &transferred,
                                INFINITE_TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "Interrupt read error %d %s\n", r, libusb_error_name(r));
    return r;
  } else {
    printf("Successfully interrupt_transferred %d bytes\n", transferred);

    if (transferred < len) {
      fprintf(stderr, "Interrupt transfer short read transferred %d bytes\n",
              transferred);
      return ENODATA;
    }

    for (int i = 0; i < len; i++) {
      printf("i: %d q: 0x%02x\n", i, data[i]);
    }
    printf("\n");
  }
  return r;
}

static int phony_hid_set_report(struct PhonyHidContext *c) {
  uint8_t addr = PHONY_ENDPOINT_OUT;
  uint8_t len = 2 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);
  PhonyHidOutReport *out = c->out_report;

  printf("out_report->host_avail: %d\n", out->host_avail);
  printf("out_report->off_hook: %d\n", out->off_hook);
  data[2] = struct_to_out_report(c->out_report);

  return interrupt_transfer(c, addr, data, len);
}

int phony_hid_get_report(struct PhonyHidContext *c) {
  int status;
  uint8_t addr = PHONY_ENDPOINT_IN;
  uint8_t len = 1 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);

  status = interrupt_transfer(c, addr, &data, len);
  printf("phony_hid_get_report finished with status: %d\n", status);
  if (status == EXIT_SUCCESS) {
    in_report_to_struct(c->in_report, data[1]);
  }

  return status;
}

struct PhonyHidContext *phony_hid_new(void) {
  struct PhonyHidContext *c = calloc(sizeof(struct PhonyHidContext), 1);
  if (c == NULL) {
    fprintf(stderr, "phony_hid_new unable to allocate");
    return NULL;
  }
  // Configure default vid and pid
  c->vendor_id = EIGHT_AMPS_VID;
  c->product_id = MAPLE_V3_PID;
  c->in_report = calloc(sizeof(PhonyHidInReport), 1);
  c->out_report = calloc(sizeof(PhonyHidOutReport), 1);
  return c;
}

int phony_hid_set_vendor_id(struct PhonyHidContext *c, int vid) {
  if (c == NULL) {
    fprintf(stderr, "phony_hid_set_vendor_id requires a valid context\n");
    return EINVAL; // Invalid argument
  }
  c->vendor_id = vid;
  return 0;
}

int phony_hid_set_product_id(struct PhonyHidContext *c, int pid) {
  if (c == NULL) {
    fprintf(stderr, "phony_hid_set_product_id requires a valid context\n");
    return EINVAL; // Invalid argument
  }
  c->product_id = pid;
  return 0;
}

static int auto_detach_kernel(struct PhonyHidContext *c, int enable) {
  libusb_device_handle *dev_h = c->device_handle;
  int status = libusb_set_auto_detach_kernel_driver(dev_h, enable);
  if (status != 0) {
    fprintf(stderr, "libusb_set_auto_detach_kernel_driver = 0 failed with: "
                    "%d\n", status);
  }
  printf("Successfully called libusb_set_auto_detach_kernel_driver\n");
  return status;
}

static int claim_interface(struct PhonyHidContext *c, int interface) {
  libusb_device_handle *dev_h = c->device_handle;
  int status = libusb_claim_interface(dev_h, interface);
  if (status < 0) {
    fprintf(stderr, "libusb_claim_interface error %d %s\n", status,
            libusb_error_name(status));
  }
  c->is_interface_claimed = true;
  printf("Successfully claimed interface %d\n", interface);
  return status;
}

static int find_device(struct PhonyHidContext *c, int vid, int pid) {
  int status = EXIT_SUCCESS;
  libusb_context *lusb_ctx = c->lusb_context;
  struct libusb_device_handle *dev_h = NULL;
  dev_h = libusb_open_device_with_vid_pid(lusb_ctx, vid, pid);

  if (dev_h != NULL) {
    c->device_handle = dev_h;

    libusb_device *d = libusb_get_device(dev_h);
    c->device = d;
    int bus_no = libusb_get_bus_number(d);
    int dev_addr = libusb_get_device_address(d);

    printf("Found hid device at bus: 0x%02x (%d) and dev addr 0x%02x (%d)\n",
           bus_no, bus_no, dev_addr, dev_addr);

    struct libusb_device_descriptor desc = {0};
    int rc = libusb_get_device_descriptor(d, &desc);

    if (rc == 0) {
      printf("idVendor: 0x%02x\n", desc.idVendor);
      printf("idProduct: 0x%02x\n", desc.idProduct);
      // printf("bNumConfigurations: %u\n", desc.bNumConfigurations);
      // printf("iSerialNumber: %u\n", desc.iSerialNumber);
      c->device_descriptor = &desc;
    }
    return status;
  }

  return LIBUSB_ERROR_NOT_FOUND;
}

static int get_config_descriptors(struct PhonyHidContext *c) {
  int status = EXIT_SUCCESS;
  libusb_context *lusb_ctx = c->lusb_context;
  struct libusb_config_descriptor *config = {0};
  status = libusb_get_config_descriptor(c->device, 0, &config);
  if (status < 0) {
    fprintf(stderr, "failed to get config descriptor with: %d\n", status);
    return status;
  }

  printf("Successfully got config descriptor with:\n");
  printf("bNumberIntefaces: %u\n", config->bNumInterfaces);
  printf("bDescriptorType: %u\n", config->bDescriptorType);
  printf("wTotalLength: %u\n", config->wTotalLength);

  for (int i = 0; i < config->bNumInterfaces; i++) {
    struct libusb_interface interface = config->interface[i];
    const struct libusb_interface_descriptor *desc = interface.altsetting;
    if (desc->bInterfaceNumber == MAPLE_PHONE_INTERFACE) {
      printf("==================================================\n");
      printf("INTERFACE:\n");
      printf("bInterfaceNumber: %u\n", desc->bInterfaceNumber);
      printf("bDescriptorType: %u\n", desc->bDescriptorType);
      printf("bAlternateSetting: %u\n", desc->bAlternateSetting);
      printf("bInterfaceClass: %u\n", desc->bInterfaceClass);
      printf("bInterfaceProtocol: %u\n", desc->bInterfaceProtocol);
      printf("bLength: %u\n", desc->bLength);
      printf("bNumEndpoints: %u\n", desc->bNumEndpoints);
      printf("iInterface: %u\n", desc->iInterface);

      for (int i = 0; i < desc->bNumEndpoints; i++) {
        const struct libusb_endpoint_descriptor ep_desc = desc->endpoint[i];
        printf("---\n");
        printf("ENDPOINT:\n");
        uint8_t addr = ep_desc.bEndpointAddress;
        printf("ep_desc->bEndpointAddress 0x%02x ", addr);
        if (addr & LIBUSB_ENDPOINT_IN) {
          printf("(EP IN)\n");
        } else {
          printf("(EP OUT)\n");
        }

        printf("ep_desc->bLength %u\n", ep_desc.bLength);
        printf("ep_desc->bDescriptorType %u\n", ep_desc.bDescriptorType);

        printf("ep_desc->bInterval %u\n", ep_desc.bInterval);
        printf("ep_desc->bSynchAddress %u\n", ep_desc.bSynchAddress);
        printf("ep_desc->bmAttributes %u\n", ep_desc.bmAttributes);
        printf("ep_desc->extra_length %d\n", ep_desc.extra_length);
        printf("ep_desc->wMaxPacketSize %u\n", ep_desc.wMaxPacketSize);
      }
    }
  }

  return status;
}

int phony_hid_open(struct PhonyHidContext *c) {
  if (c->is_open) {
    return EXIT_SUCCESS;
  }

  int status;

  libusb_context *lusb_ctx = NULL;
  status = libusb_init(&lusb_ctx);
  if (status != EXIT_SUCCESS) {
    fprintf(stderr, "Failed to initialise libusb\n");
    goto out;
  }
  c->lusb_context = lusb_ctx;

  status = find_device(c, c->vendor_id, c->product_id);
  if (status != EXIT_SUCCESS) {
    fprintf(stderr, "Could not open HID device at vid 0x%02x and pid "
                    "0x%02x\n", c->vendor_id, c->product_id);
    goto out;
  }
  printf("Successfully found the expected HID device\n");

  status = auto_detach_kernel(c, 1); // enable auto-detach
  if (status != EXIT_SUCCESS) {
    goto out;
  }

  status = claim_interface(c, 2);
  if (status != EXIT_SUCCESS) {
    goto out;
  }

  c->is_open = true;

  status = phony_hid_set_hostavail(c, true);
  if (status != EXIT_SUCCESS) {
    log_err("phony unable to set hostavail: %d\n", status);
    goto out;
  }

  out:
  return status;
}

int phony_hid_close(struct PhonyHidContext *c) {
  int status = EXIT_SUCCESS;
  if (!c->is_open) {
    return status;
  }

  libusb_device_handle *dev_h = c->device_handle;
  if (dev_h != NULL) {
    if (c->is_interface_claimed) {
      status = libusb_release_interface(dev_h, MAPLE_PHONE_INTERFACE);
      if (status != 0) {
        fprintf(stderr, "libusb_release_interface error %d\n", status);
      }
    }

    // NOTE(lbayes): Ignore error, it's logged in the called method and any
    // failures here should not impact subsequent calls.
    status = libusb_reset_device(c->device_handle);
    if (status != 0) {
      fprintf(stderr, "phony_hid_reset_device error %d %s\n", status,
              libusb_error_name(status));
    } else {
      printf("Successfully reset_device\n");
    }

    libusb_close(dev_h);
    libusb_exit(NULL);
  }
  printf("Exiting now\n");
  return status;
}

int phony_hid_set_hostavail(struct PhonyHidContext *c, bool is_hostavail) {
  printf("phony_hid_set_hostavail to: %B\n", is_hostavail);
  c->out_report->host_avail = is_hostavail;
  return phony_hid_set_report(c);
}

int phony_hid_set_off_hook(struct PhonyHidContext *c, bool is_offhook) {
  printf("phony_Hid_set_offhook to: %B\n", is_offhook);
  c->out_report->off_hook = is_offhook;
  return phony_hid_set_report(c);
}

void phony_hid_free(struct PhonyHidContext *c) {
  if (c != NULL) {
    phony_hid_close(c);
    if (c->in_report != NULL) {
      free(c->in_report);
    }
    if (c->out_report != NULL) {
      free(c->out_report);
    }
    free(c);
  }
}

//
// Created by lukebayes on 4/23/21.
//

#include "libusb_helper.h"
#include <errno.h>
#include <hidapi/hidapi.h>
#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VERSION "0.1.0"
#define VENDOR_ID                     0x335e
#define PRODUCT_ID                    0x8a01
#define MAPLE_PHONE_INTERFACE         2
#define PHONY_ENDPOINT_IN 0x81
#define PHONY_ENDPOINT_OUT 0x01
#define ALT_INTERFACE_NUMBER          1
#define DEVICE_CONFIGURATION          1

#define EIGHT_AMPS_VID 0x335e
#define MAPLE_V3_PID 0x8a01

// HID Class-Specific Requests values. See section 7.2 of the HID specifications
#define HID_GET_REPORT                0x01
#define HID_GET_IDLE                  0x02
#define HID_GET_PROTOCOL              0x03
#define HID_SET_REPORT                0x09
#define HID_SET_IDLE                  0x0A
#define HID_SET_PROTOCOL              0x0B
#define HID_REPORT_TYPE_INPUT         0x01
#define HID_REPORT_TYPE_OUTPUT        0x02
#define HID_REPORT_TYPE_FEATURE       0x03

#define LIBUSB_REQUEST_TYPE_CLASS     HID_REPORT_TYPE_FEATURE

#define CTRL_IN PHONY_ENDPOINT_IN
#define CTRL_OUT PHONY_ENDPOINT_OUT

// #define CTRL_IN PHONY_ENDPOINT_IN |LIBUSB_REQUEST_TYPE_CLASS|MAPLE_PHONE_INTERFACE
// #define CTRL_OUT PHONY_ENDPOINT_OUT |LIBUSB_REQUEST_TYPE_CLASS|MAPLE_PHONE_INTERFACE

const static uint8_t PHONY_PACKET_IN_LEN = 3;
const static uint8_t PHONY_PACKET_OUT_LEN = 3;
const static uint8_t PHONY_EP_IN_ADDR = 0x81;
const static uint8_t PHONY_EP_OUT_ADDR = 0x01;

const static int TIMEOUT = 5000; /* timeout in ms */

static int is_interface_claimed = false;

static struct PhonyHidContext {
  libusb_context *lusb_context;
  libusb_device_handle *device_handle;
  libusb_device *device;
  struct libusb_config_descriptor *config_descriptor;
  struct libusb_device_descriptor *device_descriptor;
}PhonyHidContext;

void print_8_bits(uint8_t x) {
  // print_bits found here:
  // https://stackoverflow.com/a/53850409/105023
  for(int i = sizeof(x) << 3; i; i--) {
    putchar('0' + ((x >> (i - 1)) & 1));
  }
}

void print_16_bits(uint16_t x) {
  // print_bits found here:
  // https://stackoverflow.com/a/53850409/105023
  for(int i = sizeof(x) << 3; i; i--) {
    putchar('0' + ((x >> (i - 1)) & 1));
  }
}


static int find_device(struct PhonyHidContext *ctx, uint16_t vid,
                       uint16_t pid) {
  int status = EXIT_SUCCESS;
  libusb_context *lusb_ctx = ctx->lusb_context;
  struct libusb_device_handle *dev_h = NULL;
  dev_h = libusb_open_device_with_vid_pid(lusb_ctx, vid, pid);
  ctx->device_handle = dev_h;

  if (dev_h != NULL) {
    libusb_device *d = libusb_get_device(dev_h);
    ctx->device = d;
    int bus_no = libusb_get_bus_number(d);
    int dev_addr = libusb_get_device_address(d);

    printf("Found hid device at bus: 0x%02x (%d) and dev addr 0x%02x (%d)\n",
           bus_no, bus_no, dev_addr, dev_addr);

    struct libusb_device_descriptor desc = {0};
    int rc = libusb_get_device_descriptor(d, &desc);

    if (rc == 0) {
      printf("with following props:\n");
      printf("idVendor: 0x%02x\n", desc.idVendor);
      printf("idProduct: 0x%02x\n", desc.idProduct);
      printf("bNumConfigurations: %u\n", desc.bNumConfigurations);
      printf("iSerialNumber: %u\n", desc.iSerialNumber);
      ctx->device_descriptor = &desc;
    }
    return status;
  }
  return LIBUSB_ERROR_NOT_FOUND;
}

static int get_config_descriptors(struct PhonyHidContext *ctx) {
  int status = EXIT_SUCCESS;
  libusb_context *lusb_ctx = ctx->lusb_context;
  struct libusb_config_descriptor *config = {0};
  status = libusb_get_config_descriptor(ctx->device, 0, &config);
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

static int interrupt_transfer(struct PhonyHidContext *ctx, uint8_t addr,
    unsigned char *data, uint8_t len) {
  int r = EXIT_SUCCESS;
  int transferred = 0;

  libusb_device_handle *dev_h = ctx->device_handle;

  r = libusb_interrupt_transfer(dev_h, addr, data, len, &transferred,
                                TIMEOUT);
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


static int interrupt_in_transfer(struct PhonyHidContext *ctx, uint8_t len) {
  /*
  unsigned char answer[PACKET_IN_LEN];
  memset(answer, 0x0, PACKET_IN_LEN);

  r = libusb_interrupt_transfer(dev_h, PHONY_ENDPOINT_IN, answer, PACKET_IN_LEN,
                                &transferred, TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "Interrupt read error %d %s\n", r, libusb_error_name(r));
    // return r;
  }

  r = libusb_interrupt_transfer(dev_h, PHONY_ENDPOINT_OUT, question, PACKET_IN_LEN,
                                &transferred,TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "Interrupt write error %d - %s\n", r, libusb_error_name(r));
    // return r;
  }
 */

  return 0;
}

static int set_hostavail(struct PhonyHidContext *ctx, int is_host_avail) {
  printf(">> set_hostavail\n");
  uint8_t addr = PHONY_EP_OUT_ADDR;
  uint8_t len = 2 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);
  // TODO(lbayes): This is NOT correct, we need to bit shift here, otherwise
  //  we're clobbering various other bits on the full byte.
  data[2] = is_host_avail;

  // data[0] = addr; // first bit active indicates hostavail = true
  // data[1] = 0x1;
  int r = interrupt_transfer(ctx, addr, data, len);

  return r;
}


static int get_phone_state(struct PhonyHidContext *ctx) {
  printf(">> get_phone_state\n");
  uint8_t addr = PHONY_EP_IN_ADDR;
  uint8_t len = 2;
  unsigned char data[len];
  memset(data, 0x0, len);

  // data[0] = addr; // first bit active indicates hostavail = true
  // data[1] = 0x1;
  int r = interrupt_transfer(ctx, addr, data, len);

  // const char *state = phony_hid_state_to_str(data[1]);
  // printf("PHONY STATe: %s\n", state);
  return r;
}

static int set_offhook(struct PhonyHidContext *ctx) {
  uint8_t addr = PHONY_EP_OUT_ADDR;
  uint8_t len = 2 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);
  data[2] = 0x3;

  int r = interrupt_transfer(ctx, addr, data, len);

  return r;
}

static int set_onhook(struct PhonyHidContext *ctx) {
  uint8_t addr = PHONY_EP_OUT_ADDR;
  uint8_t len = 2 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);
  data[2] = 0x1;

  int r = interrupt_transfer(ctx, addr, data, len);

  return r;
}

typedef struct VersionReport {
  uint8_t major;
  uint8_t minor;
  uint8_t rev;
}VersionReport;

static int get_version_report(struct PhonyHidContext *ctx) {
  VersionReport *ver = {0};
  uint8_t addr = PHONY_EP_OUT_ADDR;
  uint8_t len = 3 + 1; // 3 bytes + 1 address byte?
  unsigned char data[len];
  memset(data, 0x0, len);

  data[0] = addr; // first bit active indicates hostavail = true
  data[1] = 0x1;
  int r = interrupt_transfer(ctx, addr, data, len);

  return r;
}


/*
static int test_control_transfer(void)
{
  printf("libusb_control_transfer begin\n");
  int r,i;
  char answer[len];
  char question[len];
  for (i=0;i<len; i++) question[i]=0x20+i;

  r = libusb_control_transfer(dev_h,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00, 0,question, len,TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "Control Out error %d - %s\n", r, libusb_error_name(r));
    return r;
  }
  printf("libusb_control_transfer HID_SET_REPORT success\n");

  r = libusb_control_transfer(dev_h,CTRL_IN,HID_GET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,0, answer,len, TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "Control IN error %d - %s\n", r, libusb_error_name(r));
    return r;
  }
  printf("libusb_control_transfer HID_GET_REPORT success\n");

  for(i = 0;i < len; i++) {
    if(i%8 == 0)
      printf("\n");
    printf("%02x, %02x; ",question[i],answer[i]);
  }
  printf("\n");

  printf("libusb_control_transfer end\n");
  return 0;
}
*/

/*
static int test_control_transfer_in_out(void)
{
  int r,i;
  char answer[PACKET_IN_LEN];
  char question[PACKET_IN_LEN];
  for (i=0;i<PACKET_IN_LEN; i++) question[i]=0x30+i;
  for (i=1;i<PACKET_IN_LEN; i++) answer[i]=0;

  r = libusb_control_transfer(dev_h,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_OUTPUT<<8)|0x00, 0,question, PACKET_IN_LEN,TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "Control Out error %d - %s\n", r, libusb_error_name(r));
    return r;
  }
  r = libusb_control_transfer(dev_h,CTRL_IN,HID_GET_REPORT,(HID_REPORT_TYPE_INPUT<<8)|0x00, 0, answer,PACKET_IN_LEN, TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "Control IN error %d - %s\n", r, libusb_error_name(r));
    return r;
  }
  for(i = 0;i < PACKET_IN_LEN; i++) {
    if(i%8 == 0)
      printf("\n");
    printf("%02x, %02x; ",question[i],answer[i]);
  }
  printf("\n");

  return 0;
}
*/

static int auto_detach_kernel(struct PhonyHidContext *ctx, int enable) {
  libusb_device_handle *dev_h = ctx->device_handle;

  int r = libusb_set_auto_detach_kernel_driver(dev_h, enable);
  if (r != 0) {
    fprintf(stderr, "libusb_set_auto_detach_kernel_driver = 0 failed with: "
                    "%d\n", r);
  }
  printf("Successfully called libusb_set_auto_detach_kernel_driver\n");
  return r;
}

static int detach_kernel(libusb_device_handle *dev_h, int interface) {
// #ifdef LINUX
  int r = libusb_detach_kernel_driver(dev_h, interface);
  if (r != 0) {
    fprintf(stderr, "libusb_detach_kernel_driver failed with: %d %s\n",
            r, libusb_error_name(r));
  }
  printf("Successfully detached kernel driver\n");
  return r;
// #endif
}

static int get_configuration(libusb_device_handle *dev_h) {
  int cfg;
  int r = libusb_get_configuration(dev_h, &cfg);
  if (r != 0) {
    fprintf(stderr, "libusb_get_configuration error with %d\n", r);
  } else {
    printf("Successfully called libusb_get_configuration result: %d\n", cfg);
  }

  return r;
}

static int set_configuration(libusb_device_handle *dev_h, int config) {
  int r = libusb_set_configuration(dev_h, config);
  if (r < 0) {
    fprintf(stderr, "libusb_set_configuration error %s\n",
            libusb_error_name(r));
  }
  printf("Successfully set usb configuration %d\n", config);

  return r;
}

static int claim_interface(struct PhonyHidContext *ctx, int interface) {
  libusb_device_handle *dev_h = ctx->device_handle;
  int r = libusb_claim_interface(dev_h, MAPLE_PHONE_INTERFACE);
  if (r < 0) {
    fprintf(stderr, "libusb_claim_interface error %d %s\n", r,
            libusb_error_name(r));
  }
  is_interface_claimed = true;
  printf("Successfully claimed interface %d\n", MAPLE_PHONE_INTERFACE);
}

int hid_help_me(void) {
  int r = hid_init();
  if (r < 0) {
    fprintf(stderr, "failed to init HID\n");
    return r;
  }
  printf("Successfully initialized hid\n");

  hid_device *handle = hid_open(EIGHT_AMPS_VID, MAPLE_V3_PID, NULL);
  if (handle == NULL) {
    fprintf(stderr, "failed to open HID\n");
    return -1;
  }
  printf("Successfully opened hid\n");

  size_t size = 3 + 1; // 3 bytes version + 1 byte address
  unsigned char *data = malloc(size);
  memset(data, 0x0, size);
  data[0] = 0x82; // phony interface
  data[1] = 0x1;

  size_t bytes_sent = hid_send_feature_report(handle, data, size);

  // r = hid_get_feature_report(handle, data, size);
  if (bytes_sent <= 0) {
    fprintf(stderr, "hid_get_feature_report failed with: %zu %s\n",
           bytes_sent, (char *)hid_error(handle));

    return r;
  }

  printf("Successfully sent feature report with %zu bytes sent\n", bytes_sent);

  for (int i = 0; i < size; i++) {
    printf("i: %d q: 0x%02x\n", i, data[i]);
  }

out:
  sleep(10);
  printf("Exiting HID\n");
  if (handle != NULL) {
    hid_close(handle);
    printf("Closed HID handle\n");
  }

  return r;
}

int libusb_help_me(void) {
  int r = EXIT_SUCCESS;

  size_t size = sizeof(PhonyHidContext);
  struct PhonyHidContext *ctx = malloc(size);
  if (ctx == NULL) {
    fprintf(stderr, "Failed to allocate usb context\n");
    return ENOMEM;
  }
  memset(ctx, 0x0, size);

  libusb_context *lusb_ctx = NULL;

  r = libusb_init(&lusb_ctx);
  if (r < 0) {
    fprintf(stderr, "Failed to initialise libusb\n");
    goto out;
  }

  r = find_device(ctx, EIGHT_AMPS_VID, MAPLE_V3_PID);
  if (r < 0) {
    fprintf(stderr, "Could not find/open the HID device\n");
    goto out;
  }
  printf("Successfully found the expected HID device\n");

  r = get_config_descriptors(ctx);
  if (r < 0) {
    goto out;
  }

  r = auto_detach_kernel(ctx, 1); // enable auto-detach
  if (r < 0) {
    goto out;
  }

  r = claim_interface(ctx, 2);
  if (r < 0) {
    goto out;
  }

  // App features stitch_start here:
  r = set_hostavail(ctx, 1);
  if (r < 0) {
    goto out;
  }

  r = get_phone_state(ctx);
  if (r < 0) {
    goto out;
  }

  r = set_offhook(ctx);
  if (r < 0) {
    goto out;
  }

  r = get_phone_state(ctx);
  if (r < 0) {
    goto out;
  }

  sleep(3);

  /*
  r = set_onhook(ctx);
  if (r < 0) {
    goto out;
  }
  */

  /*
  r = get_version_report(ctx);
  if (r < 0) {
    goto out;
  }
   */

  sleep(10);

  // Remove the host from the firmware
  r = set_hostavail(ctx, 0);
  if (r < 0) {
    goto out;
  }

  out:
  printf("-------------------------------------\n");
  libusb_device_handle *dev_h = ctx->device_handle;
  if (dev_h != NULL) {
    if (is_interface_claimed) {
      r = libusb_release_interface(dev_h, MAPLE_PHONE_INTERFACE);
      if (r != 0) {
        fprintf(stderr, "libusb_release_interface error %d\n", r);
      }
    }

    r = libusb_reset_device(dev_h);
    if (r != 0) {
      fprintf(stderr, "libusb_reset_device error %d %s\n", r,
              libusb_error_name(r));
    } else {
      printf("Successfully reset_device\n");
    }
    libusb_attach_kernel_driver(dev_h, MAPLE_PHONE_INTERFACE);
    libusb_close(dev_h);
    libusb_exit(NULL);
  }
  printf("Exiting now\n");
  return r >= 0 ? r : -r;
}

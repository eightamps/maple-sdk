//
// Created by lukebayes on 4/30/21.
//

#ifndef MAPLE_LIBUSB_FAKE_H
#define MAPLE_LIBUSB_FAKE_H

typedef struct {
}libusb_context;

typedef struct {
}libusb_device_handle;

typedef struct {
}libusb_device;

void libusb_fake_set_next_result(int result);
int libusb_init(libusb_context *c);
void libusb_exit(libusb_context *c);

#endif //MAPLE_LIBUSB_FAKE_H

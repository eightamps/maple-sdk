//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include <stdint.h>
#include <hidapi/hidapi.h>

typedef enum {
  PhonyStatusSuccess = 0,
  PhonyStatusFailureToAlloc,
  PhonyStatusFailureHidInit,
  PhonyStatusFailureDeviceConnect,
  PhonyStatusFailure,
}PhonyStatus;

typedef struct Phony {
  int vid;
  int pid;
  hid_device *device;
}Phony;

PhonyStatus phony_init(Phony *phony);
PhonyStatus phony_info(Phony *phony);
PhonyStatus phony_free(Phony *phony);
Phony *phony_new(uint16_t vid, uint16_t pid);

#endif //MAPLE_PHONY_H

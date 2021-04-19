//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include <stdint.h>
#include <hidapi/hidapi.h>

/**
 * Telephone connection API return status.
 */
typedef enum {
  PhonyStatusSuccess = 0,
  PhonyStatusFailureToAlloc = 1,
  PhonyStatusFailureHidInit,
  PhonyStatusFailureDeviceConnect,
  PhonyStatusFailureCommunication,
}PhonyStatus;

/**
 * Represents a telephone connection.
 */
typedef struct Phony {
  int vid;
  int pid;
  hid_device *device;
}Phony;

/**
 * Create a new container for a telephone connection that is associated with
 * the provided USB-IF Vendor ID and Product ID.
 * @param vid
 * @param pid
 * @return Phony instance
 */
Phony *phony_new(uint16_t vid, uint16_t pid);

/**
 * Initialize a recently created telephone connection.
 * @param phony
 * @return PhonyStatus
 */
PhonyStatus phony_init(Phony *phony);

/**
 * Populate the provided Phony object with device and manufacturer details.
 * @param phony
 * @return PhonyStatus
 */
PhonyStatus phony_info(Phony *phony);

/**
 * Close down and free the provided telephone connection.
 * @param phony
 * @return PhonyStatus
 */
PhonyStatus phony_free(Phony *phony);

#endif //MAPLE_PHONY_H

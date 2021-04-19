//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include <stdint.h>
#include <hidapi/hidapi.h>
#include <errno.h>

/**
 * Represents a telephone connection.
 */
typedef struct PhonyContext {
  int vid;
  int pid;
  hid_device *device;
}PhonyContext;

/**
 * Create a new container for a telephone connection that is associated with
 * the provided USB-IF Vendor ID and Product ID.
 * @param vid
 * @param pid
 * @return PhonyContext instance
 */
PhonyContext *phony_new(uint16_t vid, uint16_t pid);

/**
 * Initialize a recently created telephone connection.
 * @param phony
 * @return int Status code
 */
int phony_init(PhonyContext *phony);

/**
 * Populate the provided PhonyContext object with device and manufacturer details.
 * @param phony
 * @return int Status code
 */
int phony_info(PhonyContext *phony);

/**
 * Close down and free the provided telephone connection.
 * @param phony
 * @return int Status code
 */
int phony_free(PhonyContext *phony);

#endif //MAPLE_PHONY_H

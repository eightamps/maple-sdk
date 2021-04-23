//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include <stdint.h>
#include <hidapi/hidapi.h>
#include <errno.h>

#define EIGHT_AMPS_VID 0x335e
#define MAPLE_V3_PID 0x8a01

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
 * @return PhonyContext*
 */
PhonyContext *phony_new_with_vid_and_pid(uint16_t vid, uint16_t pid);

/**
 * Create a new container for telephone connection using the default VID and
 * PID values for Maple devices (vid: 0x335e and pid: 0x8a01).
 * @return PhonyContext*
 */
PhonyContext *phony_new(void);
/**
 * Initialize a recently created telephone connection.
 * @param PhonyContext*
 * @return int Status code
 */
int phony_init(PhonyContext *phony);

/**
 * Populate the provided PhonyContext object with device and manufacturer details.
 * @param PhonyContext*
 * @return int Status code
 */
int phony_info(PhonyContext *phony);

/**
 * Dial the provided number using the provided phony context.
 * @param PhonyContext*
 * @param numbers to dial
 * @return int Status code
 */
int phony_dial(PhonyContext *phony, const char *numbers);

/**
 * Close down and free the provided telephone connection.
 * @param PhonyContext*
 * @return void
 */
void phony_free(PhonyContext *phony);

#endif //MAPLE_PHONY_H

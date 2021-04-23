//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include <stdint.h>
#include <hidapi/hidapi.h>
#include <errno.h>
#include <stdint.h>


// typedef struct HidFeatureReport {
  // bLength;
  //
// }HidFeatureReport;

/**
 typedef PACKED(struct) {
  PACKED(union) {
    PACKED(struct) {
      uint8_t loop : 1;
      uint8_t ring : 1;
      uint8_t ring2 : 1;
      uint8_t lineinuse : 1;
      uint8_t polarity : 1;
      uint8_t  : 3;
    };
    uint8_t b;
  };
  uint8_t state;
}InReportType;
*/

typedef struct PhonyInReport {
  uint8_t loop;
  uint8_t ring;
  uint8_t ring2;
  uint8_t line_in_use;
  uint8_t polarity;
  uint8_t unused__;
  uint8_t last_b__;
  uint8_t state;
}PhonyInReport;

/*
typedef PACKED(struct) {
  PACKED(union) {
    PACKED(struct) {
      uint8_t hostavail : 1;
      uint8_t offhook : 1;
      uint8_t  : 6;
    };
    uint8_t b;
  };
}OutReportType;
 */
typedef struct PhonyOutReport {
  uint8_t host_avail;
  uint8_t off_hook;
  uint8_t unused__;
  uint8_t last_b__;
}PhonyOutReport;

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
int phony_init(PhonyContext *c);

/**
 * Populate the provided PhonyContext object with device and manufacturer details.
 * @param PhonyContext*
 * @return int Status code
 */
int phony_info(PhonyContext *c);

/**
 * Dial the provided number using the provided phony context.
 * @param PhonyContext*
 * @param numbers to dial
 * @return int Status code
 */
int phony_dial(PhonyContext *phony, const char *numbers);

/**
 * Take the line off hook.
 * @param phony
 * @return
 */
int phony_take_off_hook(PhonyContext *phony);

/**
 * Close down the open line.
 * @param PhonyContext*
 * @return int Status code
 */
int phony_hang_up(PhonyContext *phony);

/**
 * Close down and free the provided telephone connection.
 * @param PhonyContext*
 * @return void
 */
void phony_free(PhonyContext *c);

#endif //MAPLE_PHONY_H

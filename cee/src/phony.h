//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include "phony_hid.h"
#include "stitcher.h"
#include "dtmf.h"

/**
 * Represents a telephone connection.
 */
typedef struct PhonyContext {
  PhonyHidContext *hid_context;
  DtmfContext  *dtmf_context;
  StitcherContext *stitcher_context;
}PhonyContext;

/**
 * Create a new context for a telephone connection.
 * @return PhonyContext*
 */
PhonyContext *phony_new(void);

/**
 * Initialize a telephone connection with the provided VendorId and ProductId
 * @param PhonyContext*
 * @return int Status code
 */
int phony_open(PhonyContext *c, int vid, int pid);

/**
 * Initialize a new telephone connection with the first Maple device found on
 * the USB bus.
 * @param c
 * @return
 */
int phony_open_maple(PhonyContext *c);

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

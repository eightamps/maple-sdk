//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include "phony_hid.h"
#include "stitch.h"
#include "dtmf.h"


typedef enum PhonyState {
  PHONY_NOT_READY = 0,
  PHONY_READY,
  PHONY_OFF_HOOK,
  PHONY_RINGING,
  PHONY_LINE_NOT_FOUND,
  PHONY_LINE_IN_USE,
}PhonyState;


typedef void (*phony_state_changed)(void *varg);

/**
 * Represents a telephone connection.
 */
typedef struct PhonyContext {
  PhonyState state;
  phony_state_changed state_changed;
  void *userdata;
  PhonyHidContext *hid_context;
  DtmfContext  *dtmf_context;
  StitchContext *to_phone;
  StitchContext *from_phone;


  //StitcherContext *stitcher_context;
  pthread_t thread_id;
  bool is_looping;
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
int phony_open_device(PhonyContext *c, int vid, int pid);

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
 * Set a callback that will be called whenever the PhonyContext state changes.
 * @param *PhonyContext
 * @param *phony_state_changed
 * @return int Status code
 */
int phony_set_state_changed(PhonyContext *c, phony_state_changed callback,
                            void *userdata);

/**
 * Close down and free the provided telephone connection.
 * @param PhonyContext*
 * @return void
 */
void phony_free(PhonyContext *c);

/**
 * Get a human readable label from an Phony state value.
 * @param state
 * @return const char *label
 */
const char *phony_state_to_str(int state);

#endif //MAPLE_PHONY_H

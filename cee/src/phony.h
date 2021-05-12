//
// Created by lukebayes on 4/17/21.
//

#ifndef MAPLE_PHONY_H
#define MAPLE_PHONY_H

#include "phony_hid.h"
#include "stitch.h"
#include "dtmf.h"
#include "shared.h"

#define PHONY_EIGHT_AMPS_VID 0x335e
#define PHONY_MAPLE_V3_PID 0x8a01

typedef enum phony_state {
  // Firmware provided states:
  PHONY_NOT_READY = 0,
  PHONY_READY,
  PHONY_OFF_HOOK,
  PHONY_RINGING,
  PHONY_LINE_NOT_FOUND,
  PHONY_LINE_IN_USE,
  // Host-only states:
  PHONY_DEVICE_NOT_FOUND,
  PHONY_CONNECTED,
  PHONY_EXITING,
}phony_state;

typedef void (*phony_state_changed)(void *varg);

/**
 * Represents a telephone connection.
 */
typedef struct {
  phony_state state;
  phony_state_changed state_changed;
  void *userdata;
  phony_hid_context_t *hid_context;
  dtmf_context_t  *dtmf_context;
  stitch_context_t *to_phone;
  stitch_context_t *from_phone;
  pthread_t thread_id;
  bool is_looping;
}phony_context_t;

/**
 * Create a new context for a telephone connection.
 * @return phony_context_t*
 */
phony_context_t *phony_new(void);

/**
 * Initialize a telephone connection with the provided VendorId and ProductId
 * @param PhonyContext*
 * @return int Status code
 */
int phony_open_device(phony_context_t *c, int vid, int pid);

/**
 * Initialize a new telephone connection with the first Maple device found on
 * the USB bus.
 * @param c
 * @return
 */
int phony_open_maple(phony_context_t *c);

/**
 * Dial the provided number using the provided phony context.
 * @param PhonyContext*
 * @param numbers to dial
 * @return int Status code
 */
int phony_dial(phony_context_t *phony, const char *numbers);

/**
 * Take the line off hook.
 * @param phony
 * @return
 */
int phony_take_off_hook(phony_context_t *phony);

/**
 * Close down the open line.
 * @param PhonyContext*
 * @return int Status code
 */
int phony_hang_up(phony_context_t *phony);

/**
 * Set a callback that will be called whenever the phony_context_t state changes.
 * @param *PhonyContext
 * @param *phony_state_changed
 * @return int Status code
 */
int phony_on_state_changed(phony_context_t *c, phony_state_changed callback,
                            void *userdata);

/**
 * Get the current phony state.
 * @param *PhonyContext
 * @return phony_state
 */
phony_state phony_get_state(phony_context_t *c);

/**
 * Close down and free the provided telephone connection.
 * @param PhonyContext*
 * @return void
 */
void phony_free(phony_context_t *c);

/**
 * Get a human readable label from an Phony state value.
 * @param state
 * @return const char *label
 */
const char *phony_state_to_str(int state);

#endif //MAPLE_PHONY_H

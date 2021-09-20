//
// Created by lukebayes on 9/20/2021
//

#ifndef MAPLE_INFRAREDDY_H
#define MAPLE_INFRAREDDY_H

#include "infrareddy_hid.h"
#include "shared.h"

typedef enum infrareddy_state {
  // Firmware provided states:
  INFRAREDDY_NOT_READY = 0,
  INFRAREDDY_READY,
  INFRAREDDY_RECEIVING,
  INFRAREDDY_SENDING,
  // Host-only states:
  INFRAREDDY_DEVICE_NOT_FOUND,
  INFRAREDDY_CONNECTED,
  INFRAREDDY_EXITING,
}infrareddy_state;

typedef void (*infrareddy_state_changed)(void *varg);

/**
 * Represents an infrared connection.
 */
typedef struct {
  infrareddy_state state;
  infrareddy_state_changed state_changed;
  void *userdata;
  infrareddy_hid_context_t *hid_context;
  pthread_t thread_id;
  bool is_looping;
}infrareddy_context_t;

/**
 * Create a new context for an infrared connection.
 * @return infrareddy_context_t*
 */
DLL_LINK infrareddy_context_t *infrareddy_new(void);

/**
 * Initialize an infrared connection with the provided VendorId and ProductId
 * @param *infrareddy_context_t
 * @return int Status code
 */
DLL_LINK int infrareddy_open_device(infrareddy_context_t *c, int vid, int pid);

/**
 * Initialize a new infrared connection with the first Maple device found on
 * the USB bus.
 * @param *infrareddy_context_t
 * @return
 */
DLL_LINK int infrareddy_open_maple(infrareddy_context_t *c);

/**
 * Decode incoming IR data
 * @param *infrareddy_context_t
 * @return
 */
DLL_LINK int infrareddy_decode(infrareddy_context_t *c);

/**
 * Encode outbound IR data
 * @param *infrareddy_context_t
 * @return
 */
DLL_LINK int infrareddy_encode(infrareddy_context_t *c, uint16_t len, unsigned char *data);

/**
 * Set a callback that will be called whenever the infrareddy_context_t state changes.
 * @param *infrareddy_context_t
 * @param *infrareddy_state_changed
 * @return int Status code
 */
DLL_LINK int infrareddy_on_state_changed(infrareddy_context_t *c, infrareddy_state_changed callback,
    void *userdata);

/**
 * Get the current state.
 * @param *infrareddy_context_t
 * @return infrareddy_state
 */
DLL_LINK infrareddy_state infrareddy_get_state(infrareddy_context_t *c);

/**
 * Close down and free the provided infrared connection.
 * @param *infrareddy_context_t
 * @return void
 */
DLL_LINK void infrareddy_free(infrareddy_context_t *c);

/**
 * Get a human readable label from an infrared state value.
 * @param state
 * @return const char *label
 */
DLL_LINK const char *infrareddy_state_to_str(int state);

#endif //MAPLE_INFRAREDDY_H


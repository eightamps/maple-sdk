//
// Created by lukebayes on 9/20/2021
//

#include "hid_status.h"
#include "infrareddy.h"
#include "infrareddy_hid.h"
#include "log.h"
#include "shared.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_VENDOR_ID 0x335e
#define DEFAULT_PRODUCT_ID 0x8a01

const char *infrareddy_state_to_str(int state) {
  switch (state) {
    case INFRAREDDY_NOT_READY:
      return "Not ready";
    case INFRAREDDY_READY:
      return "Ready";
    case INFRAREDDY_RECEIVING:
      return "Off hook";
    case INFRAREDDY_SENDING:
      return "Ringing";
    case INFRAREDDY_DEVICE_NOT_FOUND:
      return "Device not found";
    case INFRAREDDY_CONNECTED:
      return "Connected";
    case INFRAREDDY_EXITING:
      return "Exiting";
    default:
      return "Unknown state";
  }
}

static int set_state(infrareddy_context_t *c, infrareddy_state state) {
  infrareddy_state last_state = c->state;
  if (last_state != state) {
    c->state = state;
    log_info("infrareddy changed state to: %s", infrareddy_state_to_str(state));

    if (c->state_changed != NULL) {
      log_info("calling infrareddy state_changed handler now");
      c->state_changed(c->userdata);
    }
  }
  return EXIT_SUCCESS;
}

infrareddy_context_t *infrareddy_new(void) {
  // Initialize infrareddy context
  infrareddy_context_t *c = calloc(sizeof(infrareddy_context_t), 1);
  if (c == NULL) {
    return NULL;
  }

  // Initialize Hid context
  infrareddy_hid_context_t *hc = infrareddy_hid_new();
  if (hc == NULL) {
    infrareddy_free(c);
    return NULL;
  }
  c->hid_context = hc;
  set_state(c, INFRAREDDY_NOT_READY);

  /*
  // Initialize Stitch contexts
  stitch_context_t *to_phone = stitch_new_with_label("to_phone");
  if (to_phone == NULL) {
    infrareddy_free(c);
    return NULL;
  }
  c->to_phone = to_phone;
  stitch_init(to_phone);

  stitch_context_t *from_phone = stitch_new_with_label("from_phone");
  if (from_phone == NULL) {
    infrareddy_free(c);
    return NULL;
  }
  c->from_phone = from_phone;
  stitch_init(from_phone);

  dtmf_context_t *dc = dtmf_new();
  if (dc == NULL) {
    dtmf_free(dc);
    return NULL;
  }
  c->dtmf_context = dc;
  */
  return c;
}

static void *begin_polling(void *varg) {
  infrareddy_context_t *c = varg;
  infrareddy_hid_context_t *hc = c->hid_context;
  int status = EXIT_SUCCESS;
  unsigned long error_timeout = 50 * 1000;

  log_info("infrareddy begin polling with state: %s", infrareddy_state_to_str(c->state));
  while(c->state != INFRAREDDY_EXITING) {
    if (c->state == INFRAREDDY_NOT_READY ||
        c->state == INFRAREDDY_DEVICE_NOT_FOUND) {
      // Attempt to open the HID connection
      status = infrareddy_hid_open(hc);
      log_info("Infrareddy Connecting");
      if (status == HID_SUCCESS) {
        log_info("Infrareddy Connected");
        set_state(c, INFRAREDDY_CONNECTED);
      } else {
        // log_err("infrareddy unable to open HID client with status: %s",
        // hid_status_message(status));
        log_err("Infrareddy Device not found");
        set_state(c, INFRAREDDY_DEVICE_NOT_FOUND);
        usleep_shim(error_timeout);
        continue;
      }
    }

    log_info("----------------------------");
    log_info("infrareddy waiting for HID report");
    status = infrareddy_hid_get_report(hc);
    if (status != HID_SUCCESS) {
      log_err("Infrareddy Device not found");
      set_state(c, INFRAREDDY_DEVICE_NOT_FOUND);
      usleep_shim(error_timeout);
      continue;
    }

    /*
    infrareddy_hid_in_report_t *ir = hc->in_report;

    if (ir->ring) {
      set_state(c, INFRAREDDY_RINGING);
    } else if (ir->line_in_use) {
      set_state(c, INFRAREDDY_LINE_IN_USE);
    } else if (!ir->loop || ir->line_not_found) {
      set_state(c, INFRAREDDY_LINE_NOT_FOUND);
    } else if (ir->loop) {
      set_state(c, INFRAREDDY_READY);
    } else {
      set_state(c, INFRAREDDY_NOT_READY);
    }
    */
  }

  return NULL;
}

/*
   static int infrareddy_join(infrareddy_context_t *c) {
   return pthread_join(c->thread_id, NULL);
   }
   */

int infrareddy_open_device(infrareddy_context_t *c, int vid, int pid) {
  infrareddy_hid_context_t *hc = c->hid_context;
  infrareddy_hid_set_vendor_id(hc, vid);
  infrareddy_hid_set_product_id(hc, pid);
  return pthread_create(&c->thread_id, NULL, begin_polling, c);
}

int infrareddy_open_maple(infrareddy_context_t *c) {
  log_info("infrareddy_open_maple called");
  return infrareddy_open_device(c, EIGHT_AMPS_VID, EIGHT_AMPS_MAPLE_V3_PID);
}

int infrareddy_encode(infrareddy_context_t *c, uint16_t len, unsigned char *data) {
  log_info("infrareddy_encode called with: %s", data);
  int status = EXIT_SUCCESS;

  if (data == NULL || data[0] == '\0') {
    log_err("infrareddy_encode called with empty data");
    return -EINVAL; // Invalid argument
  }

  if (c->state == INFRAREDDY_NOT_READY) {
    log_err("infrareddy_encode cannot be called before opening a connection");
    return -EPERM; // Operation not permitted
  }

  if (c->state == INFRAREDDY_DEVICE_NOT_FOUND) {
    log_err("infrareddy_dial cannot proceed without a connected device");
    return -EPERM; // Operation not permitted
  }

  status = infrareddy_hid_encode(c->hid_context, len, data);
  if (status != EXIT_SUCCESS) {
    log_err("failed to encode with: %d", status);
    return status;
  }

  log_info("infrareddy_encode exited successfully");
  return status;
}

/*
int infrareddy_dial(infrareddy_context_t *c, const char *numbers) {
  log_info("infrareddy_dial called with %s", numbers);
  if (numbers == NULL || numbers[0] == '\0') {
    log_err("infrareddy_dial called with empty input");
    return -EINVAL; // Invalid argument
  }

  if (c->state == INFRAREDDY_NOT_READY) {
    log_err("infrareddy_dial cannot be called before opening a connection");
    return -EPERM; // Operation not permitted
  }

  if (c->state == INFRAREDDY_DEVICE_NOT_FOUND) {
    log_err("infrareddy_dial cannot proceed without a connected device");
    return -EPERM; // Operation not permitted
  }

  if (c->state == INFRAREDDY_LINE_NOT_FOUND) {
    log_err("infrareddy_dial cannot proceed without a usable line");
    return -EPERM; // Operation not permitted
  }

  // Send the numbers to the DTMF service.
  int status = dtmf_dial(c->dtmf_context, numbers);
  if (status != EXIT_SUCCESS) {
    log_err("infrareddy_dial failed to generate DTMF tones");
    return status;
  }

  status = infrareddy_take_off_hook(c);
  if (status != EXIT_SUCCESS) {
    log_err("failed to take off hook with: %d", status);
    return status;
  }
  log_info("infrareddy_dial exited successfully");

  return status;
}
*/

infrareddy_state infrareddy_get_state(infrareddy_context_t *c) {
  return c->state;
}

void infrareddy_free(infrareddy_context_t *c) {
  if (c == NULL) {
    return;
  }

  /*
  // Hang up if we're in a call.
  if (c->state == INFRAREDDY_LINE_IN_USE) {
    pthread_cancel(c->thread_id);
    infrareddy_hang_up(c);
  }
  */

  set_state(c, INFRAREDDY_EXITING);

  if (c->hid_context != NULL) {
    infrareddy_hid_free(c->hid_context);
  }
  /*
  if (c->to_phone != NULL) {
    stitch_free(c->to_phone);
  }
  if (c->from_phone != NULL) {
    stitch_free(c->from_phone);
  }
  if (c->dtmf_context != NULL) {
    dtmf_free(c->dtmf_context);
  }
  */
  free(c);
}

int infrareddy_on_state_changed(infrareddy_context_t *c, infrareddy_state_changed callback,
    void *userdata) {
  if (c->state_changed != NULL) {
    log_err("infrareddy_on_state_changed cannot accept a second "
        "callback");
    return -EPERM; // Operation not permitted
  }
  c->state_changed = callback;
  c->userdata = userdata;
  return EXIT_SUCCESS;
}


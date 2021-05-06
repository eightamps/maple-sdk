//
// Created by lukebayes on 4/17/21.
//

#include "dtmf.h"
#include "log.h"
#include "phony.h"
#include "phony_hid.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *phony_state_to_str(int state) {
  switch (state) {
  case PHONY_NOT_READY:
    return "Not ready";
  case PHONY_READY:
    return "Ready";
  case PHONY_OFF_HOOK:
    return "Off hook";
  case PHONY_RINGING:
    return "Ringing";
  case PHONY_LINE_NOT_FOUND:
    return "Line not found";
  case PHONY_LINE_IN_USE:
    return "Line in use";
  case PHONY_DEVICE_NOT_FOUND:
    return "Device not found";
  default:
    return "Unknown state";
  }
}

static int swap_audio(phony_context_t *c) {
  int in_index, out_index;
  stitch_context_t *sc;

  // Set up the FROM PHONE audio signals
  sc = c->from_phone;
  in_index = stitch_get_matching_input_device_index(sc, STITCH_ASI_TELEPHONE);
  if (in_index == -1) {
    log_err("phony unable to find valid telephone audio input device");
    return -ENODEV;
  }
  out_index = stitch_get_default_output_index(sc);
  if (out_index == -1) {
    log_err("phony unable to find valid default host audio output device");
    return -ENODEV;
  }
  stitch_start(sc, in_index, out_index);

  // Set up the TO PHONE audio signals
  sc = c->to_phone;
  stitch_set_dtmf(sc, c->dtmf_context);
  in_index = stitch_get_default_input_index(sc);
  if (in_index == -1) {
    log_err("phony unable to find valid default host audio input device");
    return -ENODEV;
  }

  out_index = stitch_get_matching_output_device_index(sc, STITCH_ASI_TELEPHONE);
  if (out_index == -1) {
    log_err("phony unable to find valid telephone audio output device");
    return -ENODEV;
  }
  stitch_start(sc, in_index, out_index);

  return EXIT_SUCCESS;
}

static int stop_audio(phony_context_t *c) {
  stitch_stop(c->from_phone);
  stitch_stop(c->to_phone);
  return EXIT_SUCCESS;
}

static int set_state(phony_context_t *c, phony_state state) {
  phony_state last_state = c->state;
  int status;
  if (last_state != state) {
    c->state = state;
    log_info("phony changed state to: %s", phony_state_to_str(state));

    if (PHONY_LINE_IN_USE == state) {
      log_info("phony transitioning to line in use");
      status = swap_audio(c);
      if (status != EXIT_SUCCESS) {
        log_err("phony failed to swap audio with: %d", status);
        // TODO(lbayes): Figure out how to signal this error externally, and
        //  reset our looping
      }
    } else if (PHONY_READY == state &&
    PHONY_LINE_IN_USE == last_state) {
      log_info("phony transitioning to ready, from line in use");
      stop_audio(c);
    }

    if (PHONY_EXITING != state && c->state_changed != NULL) {
      log_info("calling phony state_changed handler now");
      c->state_changed(c->userdata);
    }
  }
}

phony_context_t *phony_new(void) {
  // Initialize Phony context
  phony_context_t *c = calloc(sizeof(phony_context_t), 1);
  if (c == NULL) {
    return NULL;
  }

  // Initialize Hid context
  phony_hid_context_t *hc = phony_hid_new();
  if (hc == NULL) {
    phony_free(c);
    return NULL;
  }
  c->hid_context = hc;
  set_state(c, PHONY_NOT_READY);

  // Initialize Stitch contexts
  stitch_context_t *to_phone = stitch_new_with_label("to_phone");
  if (to_phone == NULL) {
    phony_free(c);
    return NULL;
  }
  c->to_phone = to_phone;
  stitch_init(to_phone);

  stitch_context_t *from_phone = stitch_new_with_label("from_phone");
  if (from_phone == NULL) {
    phony_free(c);
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
  return c;
}

static void *begin_polling(void *varg) {
  phony_context_t *c = varg;
  phony_hid_context_t *hc = c->hid_context;
  int status;

  log_info("Phony begin polling with state: %s", phony_state_to_str(c->state));
  while(c->state != PHONY_EXITING) {
    if (c->state == PHONY_NOT_READY ||
        c->state == PHONY_DEVICE_NOT_FOUND) {
      // Attempt to open the HID connection
      status = phony_hid_open(hc);
      if (status == PHONY_HID_SUCCESS) {
        set_state(c, PHONY_CONNECTED);
      } else {
        // log_err("phony unable to open HID client with status: %s",
            // phony_hid_status_message(status));
        set_state(c, PHONY_DEVICE_NOT_FOUND);
        usleep(500000);
        continue;
      }
    }

    log_info("----------------------------");
    log_info("phony waiting for HID report");
    status = phony_hid_get_report(hc);
    if (status != PHONY_HID_SUCCESS) {
      set_state(c, PHONY_DEVICE_NOT_FOUND);
      continue;
    }

    phony_hid_in_report_t *ir = hc->in_report;
    if (ir->ring) {
      set_state(c, PHONY_RINGING);
    } else if (ir->line_in_use) {
      set_state(c, PHONY_LINE_IN_USE);
    } else if (!ir->loop || ir->line_not_found) {
      set_state(c, PHONY_LINE_NOT_FOUND);
    } else if (ir->loop) {
      set_state(c, PHONY_READY);
    } else {
      set_state(c, PHONY_NOT_READY);
    }
  }

  return NULL;
}

static int phony_join(phony_context_t *c) {
  return pthread_join(c->thread_id, NULL);
}

int phony_open_device(phony_context_t *c, int vid, int pid) {
  phony_hid_context_t *hc = c->hid_context;
  phony_hid_set_vendor_id(hc, vid);
  phony_hid_set_product_id(hc, pid);
  return pthread_create(&c->thread_id, NULL, begin_polling, c);
}

int phony_open_maple(phony_context_t *c) {
  log_info("phony_open_maple called");
  return phony_open_device(c, PHONY_EIGHT_AMPS_VID, PHONY_MAPLE_V3_PID);
}

int phony_take_off_hook(phony_context_t *c) {
  log_info("phony_take_off_hook called");
  if (c->state == PHONY_READY) {
    int status = phony_hid_set_off_hook(c->hid_context, true);
    if (status != EXIT_SUCCESS) {
      log_err("phony_take_off_hook failed with status: %d", status);
    }
    return status;
  }

  return -EPERM;

  // if (c->state == PHONY_OFF_HOOK) {
    // return EXIT_SUCCESS;
  // }
  // log_err("phony_take_off_hook failed with unexpected state %s",
          // phony_state_to_str(c->state));
  // return -EINVAL;
}

int phony_hang_up(phony_context_t *c) {
  if (PHONY_LINE_IN_USE == c->state) {
    log_info("phony_hang_up called");
    int status = phony_hid_set_off_hook(c->hid_context, false);
    if (status != EXIT_SUCCESS) {
      log_err("phony_hang_up failed with status: %d", status);
    }
    return status;
  }
  return -EPERM;
}

int phony_dial(phony_context_t *c, const char *numbers) {
  log_info("phony_dial called with %s", numbers);
  if (numbers == NULL || numbers[0] == '\0') {
    log_err("phony_dial called with empty input");
    return -EINVAL; // Invalid argument
  }

  if (c->state == PHONY_NOT_READY) {
    log_err("phony_dial cannot be called before opening a connection");
    return -EPERM; // Operation not permitted
  }

  if (c->state == PHONY_DEVICE_NOT_FOUND) {
    log_err("phony_dial cannot proceed without a connected device");
    return -EPERM; // Operation not permitted
  }

  if (c->state == PHONY_LINE_NOT_FOUND) {
    log_err("phony_dial cannot proceed without a usable line");
    return -EPERM; // Operation not permitted
  }

  // Send the numbers to the DTMF service.
  int status = dtmf_dial(c->dtmf_context, numbers);
  if (status != EXIT_SUCCESS) {
    log_err("phony_dial failed to generate DTMF tones");
    return status;
  }

  status = phony_take_off_hook(c);
  if (status != EXIT_SUCCESS) {
    log_err("failed to take off hook with: %d", status);
    return status;
  }
  log_info("phony_dial exited successfully");
  return status;
}

phony_state phony_get_state(phony_context_t *c) {
  return c->state;
}

void phony_free(phony_context_t *c) {
  if (c == NULL) {
    return;
  }

  // Hang up if we're in a call.
  if (c->state == PHONY_LINE_IN_USE) {
    phony_hang_up(c);
  }

  set_state(c, PHONY_EXITING);

  if (c->thread_id) {
    pthread_cancel(c->thread_id);
    // phony_join(c);
  }
  if (c->hid_context != NULL) {
    phony_hid_free(c->hid_context);
  }
  if (c->to_phone != NULL) {
    stitch_free(c->to_phone);
  }
  if (c->from_phone != NULL) {
    stitch_free(c->from_phone);
  }
  if (c->dtmf_context != NULL) {
    dtmf_free(c->dtmf_context);
  }
  free(c);
}

int phony_on_state_changed(phony_context_t *c, phony_state_changed callback,
                            void *userdata) {
  if (c->state_changed != NULL) {
    log_err("phony_on_state_changed cannot accept a second "
                    "callback");
    return -EPERM; // Operation not permitted
  }
  c->state_changed = callback;
  c->userdata = userdata;
  return EXIT_SUCCESS;
}

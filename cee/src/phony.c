//log/
// Created by lukebayes on 4/17/21.
//

#include "dtmf.h"
#include "log.h"
#include "phony.h"
#include "phony_hid.h"
#include <stdlib.h>
#include <string.h>

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

PhonyContext *phony_new(void) {
  // Initialize Phony context
  PhonyContext *c = calloc(sizeof(PhonyContext), 1);
  if (c == NULL) {
    return NULL;
  }

  // Initialize Hid context
  PhonyHidContext *hc = phony_hid_new();
  if (hc == NULL) {
    phony_free(c);
    return NULL;
  }
  c->state = PHONY_NOT_READY;
  c->hid_context = hc;

  // Initialize Stitch contexts
  StitchContext *to_phone = stitch_new_with_label("to_phone");
  if (to_phone == NULL) {
    phony_free(c);
    return NULL;
  }
  stitch_init(to_phone);
  c->to_phone = to_phone;

  StitchContext *from_phone = stitch_new_with_label("from_phone");
  if (from_phone == NULL) {
    phony_free(c);
    return NULL;
  }
  stitch_init(from_phone);
  c->from_phone = from_phone;

  DtmfContext *dc = dtmf_new();
  if (dc == NULL) {
    dtmf_free(dc);
    return NULL;
  }
  c->dtmf_context = dc;
  return c;
}

static int phony_swap_audio(PhonyContext *c) {
  int in_index, out_index;
  StitchContext *sc;

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

static int phony_stop_audio(PhonyContext *c) {
  stitch_stop(c->from_phone);
  stitch_stop(c->to_phone);
  return EXIT_SUCCESS;
}

static void *phony_poll_for_updates(void *varg) {
  PhonyContext *c = varg;
  PhonyHidContext *hc = c->hid_context;
  log_info("Phony begin polling...");
  int status = phony_hid_open(hc);
  if (status != EXIT_SUCCESS) {
    log_err("phony unable to open HID client with status: %d", status);
    c->state = PHONY_DEVICE_NOT_FOUND;
    return NULL;
  }

  PhonyState last_state;
  c->is_looping = true;
  while (c->is_looping) {
    log_info("phony waiting for HID report");
    phony_hid_get_report(hc);
    PhonyHidInReport *ir = hc->in_report;
    if (ir->ring) {
      c->state = PHONY_RINGING;
    } else if (ir->line_in_use) {
      c->state = PHONY_LINE_IN_USE;
    } else if (!ir->loop || ir->line_not_found) {
      c->state = PHONY_LINE_NOT_FOUND;
    } else if (ir->loop) {
      c->state = PHONY_READY;
    } else {
      c->state = PHONY_NOT_READY;
    }

    if (c->state != last_state) {
      if (c->state_changed != NULL) {
        log_info("calling phony state_changed handler now");
        c->state_changed(c->userdata);
      }

      log_info("UPDATED PHONY STATE to: %s", phony_state_to_str(c->state));
      if (c->state == PHONY_LINE_IN_USE) {
        status = phony_swap_audio(c);
        if (status != EXIT_SUCCESS) {
          log_err("FAILED TO SWAP AUDIO with: %d", status);
        }
      } else if (last_state == PHONY_LINE_IN_USE &&
          c->state == PHONY_READY) {
        phony_stop_audio(c);
      }

      last_state = c->state;
    }
  }

  return NULL;
}

static int begin_polling(PhonyContext *c) {
  return pthread_create(&c->thread_id, NULL, phony_poll_for_updates, c);
}

static int phony_join(PhonyContext *c) {
  return pthread_join(c->thread_id, NULL);
}

int phony_open_device(PhonyContext *c, int vid, int pid) {
  PhonyHidContext *hc = c->hid_context;
  phony_hid_set_vendor_id(hc, vid);
  phony_hid_set_product_id(hc, pid);
  return begin_polling(c);
}

int phony_open_maple(PhonyContext *c) {
  log_info("phony_open_maple called");
  return phony_open_device(c, PHONY_EIGHT_AMPS_VID, PHONY_MAPLE_V3_PID);
}

int phony_take_off_hook(PhonyContext *c) {
  log_info("phony_take_off_hook called");
  // if (c->state == PHONY_READY) {
    int status = phony_hid_set_off_hook(c->hid_context, true);
    if (status != EXIT_SUCCESS) {
      log_err("phony_take_off_hook failed with status: %d", status);
    }
    return status;
  // if (c->state == PHONY_OFF_HOOK) {
    // return EXIT_SUCCESS;
  // }
  // log_err("phony_take_off_hook failed with unexpected state %s",
          // phony_state_to_str(c->state));
  // return -EINVAL;
}

int phony_hang_up(PhonyContext *c) {
    log_info("phony_hang_up called");
    int status = phony_hid_set_off_hook(c->hid_context, false);
    if (status != EXIT_SUCCESS) {
      log_err("phony_hang_up failed with status: %d", status);
    }
    return status;
}

int phony_dial(PhonyContext *c, const char *numbers) {
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

PhonyState phony_get_state(PhonyContext *c) {
  return c->state;
}

void phony_free(PhonyContext *c) {
  if (c != NULL) {
    // Hang up if we're in a call.
    if (c->state == PHONY_LINE_IN_USE) {
      phony_hang_up(c);
    }
    if (c->hid_context) {
      // Let the read thread exit after this call.
      c->is_looping = false;
      // Attempt to flip hostavail on device.
      phony_hid_set_hostavail(c->hid_context, false);
      pthread_cancel(c->thread_id);
      // pthread_join(c->thread_id, NULL);
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
}

int phony_set_state_changed(PhonyContext *c, phony_state_changed callback,
                            void *userdata) {
  if (c->state_changed != NULL) {
    log_err("phony_set_state_changed cannot accept a second "
                    "callback");
    return -EPERM; // Operation not permitted
  }
  c->state_changed = callback;
  c->userdata = userdata;
  return EXIT_SUCCESS;
}

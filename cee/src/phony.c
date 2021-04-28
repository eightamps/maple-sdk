//
// Created by lukebayes on 4/17/21.
//

#include "dtmf.h"
#include "log.h"
#include "phony.h"
#include "phony_hid.h"
#include <stdio.h>
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
  StitchContext *to_phone = stitch_new();
  if (to_phone == NULL) {
    phony_free(c);
    return NULL;
  }
  c->to_phone = to_phone;

  StitchContext *from_phone = stitch_new();
  if (from_phone == NULL) {
    phony_free(c);
    return NULL;
  }
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
  int status = EXIT_SUCCESS;
  StitchContext *from = c->from_phone;
  stitch_init(from);
  stitch_start(from);
  return status;
}

static int phony_stop_audio(PhonyContext *c) {
  return stitch_stop(c->from_phone);
}

static void *phony_poll_for_updates(void *varg) {
  PhonyContext *c = varg;
  PhonyHidContext *hc = c->hid_context;
  printf("Phony begin polling...\n");
  int status = phony_hid_open(hc);
  if (status != EXIT_SUCCESS) {
    log_err("phony unable to open HID client with status: %d\n", status);
    return NULL;
  }

  PhonyState last_state;
  c->is_looping = true;
  while (c->is_looping) {
    printf("phony waiting for HID report\n");
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

    int status;
    if (c->state != last_state) {
      if (c->state_changed != NULL) {
        printf("calling phony state_changed handler now\n");
        c->state_changed(c->userdata);
        // (phony_state_changed)c->state_changed(c->userdata);
      }

      printf("UPDATED PHONY STATE to: %s\n", phony_state_to_str(c->state));
      if (c->state == PHONY_LINE_IN_USE) {
        status = phony_swap_audio(c);
        if (status != EXIT_SUCCESS) {
          fprintf(stderr, "FAILED TO SWAP AUDIO with: %d\n", status);
        }
      } else if (c->state == PHONY_READY) {
        status = phony_stop_audio(c);
        if (status != EXIT_SUCCESS) {
          fprintf(stderr, "FAILED TO STOP AUDIO with: %d\n", status);
        }
      }

      last_state = c->state;
    }
  }
}

static int phony_begin_polling(PhonyContext *c) {
  return pthread_create(&c->thread_id, NULL, phony_poll_for_updates, c);
}

static int phony_join(PhonyContext *c) {
  return pthread_join(c->thread_id, NULL);
}

int phony_open_device(PhonyContext *c, int vid, int pid) {
  PhonyHidContext *hc = c->hid_context;
  phony_hid_set_vendor_id(hc, vid);
  phony_hid_set_product_id(hc, pid);
  return phony_begin_polling(c);
}

int phony_open_maple(PhonyContext *c) {
  // Default VID/PID are already set in phony_hid.h
  printf("phony_open_maple called\n");
  return phony_begin_polling(c);
}

int phony_take_off_hook(PhonyContext *c) {
  printf("phony_take_off_hook called\n");
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
  // if (c->state == PHONY_LINE_IN_USE) {
    printf("phony_hang_up called\n");
    int status = phony_hid_set_off_hook(c->hid_context, 0);
    if (status != EXIT_SUCCESS) {
      log_err("phony_hang_up failed with status: %d", status);
    }
    return status;
  // }

  return EXIT_SUCCESS;
}

int phony_dial(PhonyContext *c, const char *numbers) {
  printf("phony_dial called with %s\n", numbers);
  if (numbers == NULL || numbers[0] == '\0') {
    log_err("phony_dial called with empty input");
    return -EINVAL;
  }

  // if (c->state == PHONY_READY) {
    // Take the phone off hook!
    int status = phony_take_off_hook(c);
    if (status != EXIT_SUCCESS) {
      fprintf(stderr, "failed to take off hook with: %d\n", status);
    }
    printf("Successfully requested phony_take_off_hook\n");
    return status;
    // } else if (c->state == PHONY_LINE_IN_USE) {
    // TODO(lbayes): Update DTMF state and send dial tones to in-progress call.
    // log_err("NOT YET IMPLEMENTED");
  //}

  return EXIT_SUCCESS;
}

void phony_free(PhonyContext *c) {
  if (c != NULL) {

    // Hang up if we're in a call.
    if (c->state == PHONY_LINE_IN_USE) {
      phony_hang_up(c);
    }
    if (c->hid_context) {
      // Attempt to flip hostavail on device.
      phony_hid_set_hostavail(c->hid_context, false);
    }
    // Stop listening for state messages from device.
    if (c->is_looping) {
      c->is_looping = false;
      // Cancel the thread, because it's likely blocked on an HID read
      // operation.
      pthread_cancel(c->thread_id);
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
    fprintf(stderr, "phony_set_state_changed cannot accept a second "
                    "callback\n");
    return -EPERM; // Operation not permitted
  }
  c->state_changed = callback;
  c->userdata = userdata;
  return EXIT_SUCCESS;
}

//
// Created by lukebayes on 4/17/21.
//

#include "dtmf.h"
#include "phony.h"
#include "phony_hid.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

PhonyContext *phony_new(void) {
  // Initialize Phony context
  PhonyContext *c = calloc(1, sizeof(PhonyContext));
  if (c == NULL) {
    return NULL;
  }

  // Initialize Hid context
  PhonyHidContext *hc = phony_hid_new();
  if (hc == NULL) {
    phony_free(c);
    return NULL;
  }
  c->hid_context = hc;

  // Initialize Stitcher context
  StitcherContext *sc = stitcher_new();
  if (sc == NULL) {
    phony_free(c);
    return NULL;
  }
  c->stitcher_context = sc;

  DtmfContext *dc = dtmf_new();
  if (dc == NULL) {
    dtmf_free(dc);
    return NULL;
  }
  c->dtmf_context = dc;

  return c;
}

int phony_open(PhonyContext *c, int vid, int pid) {
  return phony_hid_open(c->hid_context, vid, pid);
}

int phony_open_maple(PhonyContext *c) {
  return phony_open(c, EIGHT_AMPS_VID, MAPLE_V3_PID);
}

int phony_take_off_hook(PhonyContext *c) {
  printf("phony_take_off_hook called\n");
  return phony_hid_set_offhook(c->hid_context, 1);
}

int phony_hang_up(PhonyContext *c) {
  printf("phony_hang_up called\n");
  return phony_hid_set_offhook(c->hid_context, 0);
}

int phony_dial(PhonyContext *c, const char *numbers) {
  printf("phony_dial called with %s\n", numbers);
  int status;

  StitcherContext *sc = c->stitcher_context;
  if (numbers != NULL && numbers[0] != '\0') {

    // Take the phone off hook!
    status = phony_take_off_hook(c);
    if (status != EXIT_SUCCESS) {
      fprintf(stderr, "failed to take off hook with: %d\n", status);
    }
    printf("Successfully completed phony_take_off_hook\n");

    sleep(1);

    /*
    // TODO(lbayes): We should instead get the sample_rate directly from the
    //  mic and use that to configure the speaker and DTMF tones.
    struct SoundIoSampleRateRange *range = sc->to_speaker->device->sample_rates;
    int sample_rate = 48000; // Picked 48kHz because that's what the stream
    // defaulted to on my computer.
    if (range->min > 48000 && range->max < 48000) {
      return EPERM; // Operation not permitted
    }

    // We have a potentially valid input string, send to DTMF for configuration.
    // DtmfContext *dc = dtmf_new(numbers, sample_rate);
    DtmfContext *dc = c->dtmf_context;
    status = dtmf_dial(dc, numbers, sample_rate);
    if (status < 0) {
      fprintf(stderr, "phony_dial failed in dtmf_dial\n");
      return status;
    }

    // Set up the DTMF stream for emission on the line...
    sc->to_phone->stream->userdata = (void *)dc;
     */

    // Start audio stitcher
    status = stitcher_start(sc);
    if (status < 0) {
      fprintf(stderr, "stitcher_start failed with %d\n", status);
      free(c);
    }
    printf("Successfully started and exited stitcher\n");

    phony_hang_up(c);
    stitcher_stop(c->stitcher_context);
  }

  return 0;
}

void phony_free(PhonyContext *c) {
  if (c != NULL) {
    if (c->hid_context != NULL) {
      phony_hid_free(c->hid_context);
    }
    if (c->stitcher_context != NULL) {
      stitcher_free(c->stitcher_context);
    }
    if (c->dtmf_context != NULL) {
      dtmf_free(c->dtmf_context);
    }
    free(c);
  }
}

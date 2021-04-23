#include "dtmf.h"
#include "phony.h"
#include "stitcher.h"
#include "libusb_helper.h"
#include <stdlib.h>
#include <unistd.h>

int phony_show_info(void) {
  // printf("Attempting to allocate PhonyContext\n");
  PhonyContext *c = phony_new_with_vid_and_pid(EIGHT_AMPS_VID, MAPLE_V3_PID);
  if (c == NULL) return ENOMEDIUM;

  int status;

  // printf("Attempting to initialize PhonyContext device\n");
  status = phony_init(c);
  if (status != EXIT_SUCCESS) return status;

  // printf("Attempting to gather PhonyContext device info\n");
  status = phony_info(c);
  if (status != EXIT_SUCCESS) return status;

  // printf("Attempting to free PhonyContext allocation\n");
  phony_free(c);
  return EXIT_SUCCESS;
}

int stitcher_exercise(void) {
  StitcherContext *c = stitcher_new();
  if (c == NULL) {
    // Failed to allocate stitcher
    return ENOMEM;
  }

  int status = stitcher_init(c);
  if (status != EXIT_SUCCESS) {
    return status;
  }

  // TODO(lbayes): We should instead get the sample_rate directly from the
  //  mic and use that to configure the speaker and DTMF tones.
  struct SoundIoSampleRateRange *range = c->to_speaker->device->sample_rates;
  int sample_rate = 48000; // Picked 48kHz because that's what the stream
  // defaulted to on my computer.
  if (range->min > 48000 && range->max < 48000) {
    return EPERM; // Operation not permitted
  }

  DtmfContext *dtmf_context = dtmf_new("5107271234", sample_rate);
  c->to_speaker->stream->userdata = (void *)dtmf_context;
  status = stitcher_start(c, dtmf_soundio_callback);
  dtmf_free(dtmf_context);

  stitcher_free(c);
  return status;
}

int main() {
  // return phony_show_info();
  // return stitcher_exercise();
  return libusb_help_me();
  // return hid_help_me();
}

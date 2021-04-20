#include "dtmf.h"
#include "phony.h"
#include "stitcher.h"
#include <stdlib.h>
#include <unistd.h>

#define MAPLE_VID 0x335e
#define MAPLE_PID 0x8a01

int phony_show_info(void) {
  // printf("Attempting to allocate PhonyContext\n");
  PhonyContext *c = phony_new(MAPLE_VID, MAPLE_PID);
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
  int status = stitcher_init(c);
  if (status != EXIT_SUCCESS) return status;

  // TODO(lbayes): We should instead get the sample_rate directly from the
  //  mic and use that to configure the speaker and DTMF tones.
  struct SoundIoSampleRateRange *range = c->to_speaker->sample_rates;
  int sample_rate = 48000; // Picked 48kHz because that's what the stream
                           // defaulted to on my computer.
  if (range->min > 48000 && range->max < 48000) {
    return EPERM; // Operation not permitted
  }

  DtmfContext *dtmf_context = dtmf_new("123", sample_rate);
  c->to_speaker_stream->userdata = (void *)dtmf_context;
  status = stitcher_start(c, dtmf_soundio_callback);
  //dtmf_free(dtmf_context);

  stitcher_free(c);
  return status;
}

int main() {
  // return phony_show_info();
  return stitcher_exercise();
}

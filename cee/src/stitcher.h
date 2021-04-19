//
// Created by lukebayes on 4/19/21.
//

#ifndef MAPLE_STITCHER_H
#define MAPLE_STITCHER_H

#include <soundio/soundio.h>

typedef struct StitcherContext {
  bool is_active;
  int sample_rate;
  struct SoundIo *soundio;
  struct SoundIoDevice *to_speaker;
  struct SoundIoOutStream *to_speaker_stream;
}StitcherContext;

typedef void (StitcherCallback)(struct SoundIoOutStream *out_stream,
    int frame_count_min, int frame_count_max);

StitcherContext *stitcher_new(void);
int stitcher_init(StitcherContext *c);
int stitcher_start(StitcherContext *c, StitcherCallback *cb);
void stitcher_free(StitcherContext *c);

/**
 * This is a specific implementation of a Soundio device stream callback that
 * will take the samples from an attached (at stream->userdata) DtmfContext
 * and apply those samples into the stream as needed.
 * @param out_stream
 * @param frame_count_min
 * @param frame_count_max
 * @return void
 */
void *dtmf_soundio_callback(struct SoundIoOutStream *out_stream,
    int frame_count_min, int frame_count_max);

#endif // MAPLE_STITCHER_H

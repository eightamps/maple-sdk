//
// Created by lukebayes on 4/19/21.
//

#ifndef MAPLE_STITCHER_H
#define MAPLE_STITCHER_H

#include <soundio/soundio.h>

typedef struct StitcherStreamContext {
  struct StitcherContext *context;
  struct SoundIoRingBuffer *buffer;
}StitcherStreamContext;

typedef struct StitcherOutDevice {
  char *name;
  struct SoundIoDevice *device;
  struct SoundIoOutStream *stream;
}StitcherOutDevice;

typedef struct StitcherInDevice {
  char *name;
  struct SoundIoDevice *device;
  struct SoundIoInStream *stream;
}StitcherInDevice;

typedef struct StitcherContext {
  bool is_active;
  struct SoundIo *soundio;

  // Out devices
  struct StitcherOutDevice *to_phone;
  struct StitcherOutDevice *to_speaker;

  // In Devices
  struct StitcherInDevice *from_phone;
  struct StitcherInDevice *from_mic;

  struct SoundIoRingBuffer *to_phone_buff;
  struct SoundIoRingBuffer *from_phone_buff;
}StitcherContext;

typedef void (StitcherCallback)(struct SoundIoOutStream *out_stream,
    int frame_count_min, int frame_count_max);

StitcherContext *stitcher_new(void);
int stitcher_init(StitcherContext *c);
int stitcher_start(StitcherContext *c);
int stitcher_stop(StitcherContext *c);
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
void stitcher_dtmf_callback(struct SoundIoOutStream *out_stream,
     __attribute__((unused)) int frame_count_min, int frame_count_max);

#endif // MAPLE_STITCHER_H

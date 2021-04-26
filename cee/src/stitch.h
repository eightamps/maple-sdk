//
// Created by lukebayes on 4/25/21.
//

#ifndef MAPLE_STITCH_H
#define MAPLE_STITCH_H

#include <pthread.h>
#include <stdbool.h>
#include <soundio/soundio.h>

static enum SoundIoFormat prioritized_formats[] = {
    SoundIoFormatFloat32NE,
    SoundIoFormatFloat32FE,
    SoundIoFormatS32NE,
    SoundIoFormatS32FE,
    SoundIoFormatS24NE,
    SoundIoFormatS24FE,
    SoundIoFormatS16NE,
    SoundIoFormatS16FE,
    SoundIoFormatFloat64NE,
    SoundIoFormatFloat64FE,
    SoundIoFormatU32NE,
    SoundIoFormatU32FE,
    SoundIoFormatU24NE,
    SoundIoFormatU24FE,
    SoundIoFormatU16NE,
    SoundIoFormatU16FE,
    SoundIoFormatS8,
    SoundIoFormatU8,
    SoundIoFormatInvalid,
};

static int prioritized_sample_rates[] = {
    48000,
    44100,
    24000,
};

typedef struct StitchDevice {
  int index;
  char *id;
  char *name;
}StitchDevice;

typedef struct StitchContext {
  struct SoundIo *soundio;
  pthread_t thread_id;
  int thread_exit_status;
  float input_latency;
  bool is_initialized;
  bool is_active;
  bool in_raw;
  bool out_raw;
  enum SoundIoBackend backend;
  char *in_device_id;
  char *out_device_id;
  char *in_device_blocklist;
  char *out_device_blocklist;
  struct SoundIoRingBuffer *ring_buffer;
}StitchContext;

StitchContext *stitch_new(void);
int stitch_init(StitchContext *c);
int stitch_start(StitchContext *c);
int stitch_stop(StitchContext *c);
int stitch_join(StitchContext *c);
void stitch_free(StitchContext *c);
enum SoundIoBackend stitch_get_backend_from_label(char *label);

#endif //MAPLE_STITCH_H

//
// Created by lukebayes on 4/25/21.
//

#ifndef MAPLE_STITCH_H
#define MAPLE_STITCH_H

#include "dtmf.h"
#include <pthread.h>
#include <soundio/soundio.h>
#include <stdbool.h>

#define STITCH_ASI_TELEPHONE "ASI Telephone"
#define STITCH_ASI_MICROPHONE "ASI Microphone"
#define STITCH_WAY_2_CALL "Way2Call"
#define STITCH_WAY_2_CALL_LOWER "way2call"
#define STITCH_SPDIF "PDIF"
#define STITCH_PULSE_AUDIO "PulseAudio sound server"
#define STITCH_BUFFER_CAPACITY_MULTIPLIER 40
#define STITCH_DEFAULT_LATENCY_MS (float)0.06f

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

typedef struct StitchContext {
  struct SoundIo *soundio;
  char *label;
  DtmfContext *dtmf_context;
  pthread_t thread_id;
  int thread_exit_status;
  float input_latency;
  int in_device_index;
  int out_device_index;
  bool is_initialized;
  bool is_active;
  // bool in_raw;
  // bool out_raw;
  enum SoundIoBackend backend;
  struct SoundIoRingBuffer *ring_buffer;
}StitchContext;

StitchContext *stitch_new_with_label(char *label);
StitchContext *stitch_new(void);
int stitch_init(StitchContext *c);

int stitch_set_dtmf(StitchContext *c, DtmfContext *d);
int stitch_get_default_input_index(StitchContext *c);
int stitch_get_default_output_index(StitchContext *c);
int stitch_get_matching_input_device_index(StitchContext *c, char *name);
int stitch_get_matching_output_device_index(StitchContext *c, char *name);

int stitch_start(StitchContext *c, int in_index, int out_index);
int stitch_stop(StitchContext *c);
int stitch_join(StitchContext *c);
void stitch_free(StitchContext *c);
// enum SoundIoBackend stitch_get_backend_from_label(char *label);

#endif //MAPLE_STITCH_H

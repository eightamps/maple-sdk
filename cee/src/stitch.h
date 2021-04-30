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
#define STITCH_BUFFER_CAPACITY_MULTIPLIER 30
#define STITCH_DEFAULT_LATENCY_MS (float)0.07f
#define STITCH_OUT_DELAY_SECONDS 2

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

typedef struct {
  struct SoundIo *soundio;
  char *label;
  dtmf_context_t *dtmf_context;
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
}stitch_context_t;

stitch_context_t *stitch_new_with_label(char *label);
stitch_context_t *stitch_new(void);
int stitch_init(stitch_context_t *c);

int stitch_set_dtmf(stitch_context_t *c, dtmf_context_t *d);
int stitch_get_default_input_index(stitch_context_t *c);
int stitch_get_default_output_index(stitch_context_t *c);
int stitch_get_matching_input_device_index(stitch_context_t *c, char *name);
int stitch_get_matching_output_device_index(stitch_context_t *c, char *name);

int stitch_start(stitch_context_t *c, int in_index, int out_index);
int stitch_stop(stitch_context_t *c);
int stitch_join(stitch_context_t *c);
void stitch_free(stitch_context_t *c);
// enum SoundIoBackend stitch_get_backend_from_label(char *label);

#endif //MAPLE_STITCH_H

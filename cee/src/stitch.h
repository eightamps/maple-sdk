//
// Created by lukebayes on 4/25/21.
//

#ifndef MAPLE_STITCH_H
#define MAPLE_STITCH_H

#include <pthread.h>
#include <stdbool.h>
#include <soundio/soundio.h>

#define STITCH_ASI_TELEPHONE "ASI Telephone"
#define STITCH_ASI_MICROPHONE "ASI Microphone"
#define STITCH_WAY_2_CALL "Way2Call"
#define STITCH_WAY_2_CALL_LOWER "way2call"
#define STITCH_SPDIF "PDIF"
#define STITCH_PULSE_AUDIO "PulseAudio sound server"

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
    24000,
    44100,
    48000,
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
  int in_device_index;
  int out_device_index;
  bool is_initialized;
  bool is_active;
  bool in_raw;
  bool out_raw;
  enum SoundIoBackend backend;
  struct SoundIoRingBuffer *ring_buffer;
}StitchContext;

StitchContext *stitch_new(void);
int stitch_init(StitchContext *c);

int stitch_get_default_input_index(StitchContext *c);
int stitch_get_default_output_index(StitchContext *c);
int stitch_get_matching_input_device_index(StitchContext *c, char *name);
int stitch_get_matching_output_device_index(StitchContext *c, char *name);

int stitch_start(StitchContext *c, int in_index, int out_index);
int stitch_stop(StitchContext *c);
int stitch_join(StitchContext *c);
void stitch_free(StitchContext *c);
enum SoundIoBackend stitch_get_backend_from_label(char *label);

#endif //MAPLE_STITCH_H

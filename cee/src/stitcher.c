//
// Created by lukebayes on 4/19/21.
//

#include "stitcher.h"
#include "dtmf.h"
#include <stdlib.h>
#include <stdio.h>
#include <soundio/soundio.h>
#include <errno.h>
#include <math.h>

static const float PI = 3.1415926535f;
static const int DTMF_TONE_SECONDS = 1;

/*
static void dtmf_callback(struct SoundIoOutStream *out_stream,
    int frame_count_min, int frame_count_max) {

  // DtmfContext *dtmf_context = (DtmfContext *)out_stream->userdata;

  float seconds_offset = 0.0f;
  const struct SoundIoChannelLayout *layout = &out_stream->layout;
  float float_sample_rate = out_stream->sample_rate;
  float seconds_per_frame = 1.0f / float_sample_rate;
  struct SoundIoChannelArea *areas;

  // Generate DTMF Tones
  int dtmf_samples_count = float_sample_rate * DTMF_TONE_SECONDS;
  float *dtmf_samples = malloc(sizeof(float) * dtmf_samples_count);
  if (dtmf_samples == NULL) exit(ENOMEM);
  dtmf_generate("9", dtmf_samples_count, dtmf_samples);
  // for (int i = 0; i < frame_count_max; i++) {
    // printf("YOOOOOOOOOOOO %f\n", dtmf_samples[i]);
  // }


  int frames_left = frame_count_max;
  int err;

  while (frames_left > 0) {
    int frame_count = frames_left;

    if ((err = soundio_outstream_begin_write(out_stream, &areas,
                                             &frame_count))) {
      fprintf(stderr, "ERROR: %s\n", soundio_strerror(err));
      exit(err);
    }

    if (!frame_count) {
      fprintf(stderr, "ERROR: Bad frame count\n");
      break;
    }

    // float pitch = 440.0f;
    // float radians_per_second = pitch * 2.0f * PI;
    for (int frame = 0; frame < frame_count; frame += 1) {
      float sample = dtmf_samples[frame];

      // float sample = sinf((seconds_offset * frame * seconds_per_frame) *
                          // radians_per_second);
      for (int channel = 0; channel < layout->channel_count; channel += 1) {
        float *ptr =
            (float *)(areas[channel].ptr + areas[channel].step * frame);
        *ptr = sample;
      }
    }
    seconds_offset = fmodf(seconds_offset + seconds_per_frame * frame_count,
                           1.0f);
    if (err = soundio_outstream_end_write(out_stream)) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(err);
    }

    frames_left -= frame_count;
  }
}

static void tone_callback(struct SoundIoOutStream *out_stream,
    int frame_count_min, int frame_count_max) {
  float seconds_offset = 0.0f;
  const struct SoundIoChannelLayout *layout = &out_stream->layout;
  float float_sample_rate = out_stream->sample_rate;
  float seconds_per_frame = 1.0f / float_sample_rate;
  struct SoundIoChannelArea *areas;
  int frames_left = frame_count_max;
  int err;

  while (frames_left > 0) {
    int frame_count = frames_left;

    if ((err = soundio_outstream_begin_write(out_stream, &areas,
        &frame_count))) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(err);
    }

    if (!frame_count) {
      break;
    }

    float pitch = 440.0f;
    float radians_per_second = pitch * 2.0f * PI;
    for (int frame = 0; frame < frame_count; frame += 1) {
      float sample = sinf((seconds_offset * frame * seconds_per_frame) *
                          radians_per_second);
      for (int channel = 0; channel < layout->channel_count; channel += 1) {
        float *ptr =
            (float *)(areas[channel].ptr + areas[channel].step * frame);
        *ptr = sample;
      }
    }
    seconds_offset = fmodf(seconds_offset + seconds_per_frame * frame_count,
        1.0f);
    if (err = soundio_outstream_end_write(out_stream)) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(err);
    }

    frames_left -= frame_count;
  }
}
*/

StitcherContext *stitcher_new(void) {
  StitcherContext *c = malloc(sizeof(StitcherContext));
  if (c == NULL) return NULL;
}

int stitcher_init(StitcherContext *c) {
  if (c->to_speaker != NULL) {
    fprintf(stderr, "ERROR: stitcher_init called with already-initialized "
                    "context");
    return EINVAL; // Invalid Argument
  }

  // Create the soundio client
  struct SoundIo *sio = soundio_create();
  if (sio == NULL) return ENOMEM;
  c->soundio = sio;

  // Connect the soundio client
  int conn_status = soundio_connect(sio);
  if (conn_status != EXIT_SUCCESS) return conn_status;
  soundio_flush_events(sio);

  // Get the default output device samples_index
  int index = soundio_default_output_device_index(sio);
  if (index < 0) return ENXIO; // No such device or address.

  // Get the default output device
  struct SoundIoDevice *device = soundio_get_output_device(sio, index);
  if (device == NULL) return ENOMEM;
  fprintf(stderr, "Output device: %s\n", device->name);
  c->to_speaker = device;

  struct SoundIoOutStream *to_speaker_stream = soundio_outstream_create(c->to_speaker);
  to_speaker_stream->format = SoundIoFormatFloat32NE;
  c->to_speaker_stream = to_speaker_stream;

  return EXIT_SUCCESS;
}

int stitcher_start(StitcherContext *c, StitcherCallback *cb) {
  if (cb == NULL) {
    return EINVAL; // Invalid Argument
  }

  if (c->soundio == NULL) {
    int init_status = stitcher_init(c);
    if (init_status != EXIT_SUCCESS) return init_status;
  }

  struct SoundIoOutStream *out_stream = c->to_speaker_stream;
  if (out_stream == NULL) {
    return EINVAL; // Invalid Argument
  }

  out_stream->write_callback = cb;

  int out_status = soundio_outstream_open(out_stream);
  if (out_status != EXIT_SUCCESS) return out_status;

  if (out_stream->layout_error) {
    fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror
        (out_stream->layout_error));
  }

  int out_start_status = soundio_outstream_start(out_stream);
  if (out_start_status != EXIT_SUCCESS) return out_start_status;

  c->is_active = true;
  soundio_wait_events(c->soundio);
  c->is_active = false;
  return EXIT_SUCCESS;
}

void stitcher_free(StitcherContext *c) {
  if (c == NULL) return;

  if (c->to_speaker != NULL) {
    soundio_device_unref(c->to_speaker);
  }

  if (c->soundio != NULL) {
    soundio_destroy(c->soundio);
  }

  free(c);
}

void *dtmf_soundio_callback(struct SoundIoOutStream *out_stream,
                            int frame_count_min, int frame_count_max) {
  DtmfContext *dtmf_context = (DtmfContext *)out_stream->userdata;
  float seconds_offset = 0.0f;
  const struct SoundIoChannelLayout *layout = &out_stream->layout;
  float float_sample_rate = out_stream->sample_rate;
  float seconds_per_frame = 1.0f / float_sample_rate;
  struct SoundIoChannelArea *areas;

  float *dtmf_samples = dtmf_context->samples;
  int frames_left = frame_count_max;
  int err;

  while (frames_left > 0) {
    int frame_count = frames_left;

    if ((err =
             soundio_outstream_begin_write(out_stream, &areas, &frame_count))) {
      fprintf(stderr, "ERROR: %s\n", soundio_strerror(err));
      exit(err);
    }

    if (!frame_count) {
      fprintf(stderr, "ERROR: Bad frame count\n");
      break;
    }

    int samples_index = dtmf_context->samples_index;
    int samples_count = dtmf_context->samples_count;
    // int frame_limit = frame_count;
    // if ((frame_count + samples_index) > samples_count) {
      // frame_limit = (frame_count + samples_index)
    // }

    int frame = 0;
    for (frame = 0; frame < frame_count; frame++) {
      int frame_index = frame + samples_index;
      float sample;
      if (frame_index < samples_count) {
        sample = dtmf_samples[frame + dtmf_context->samples_index];
      } else {
        sample = 0;
      }
      for (int channel = 0; channel < layout->channel_count; channel++) {
        float *ptr = (float *)(areas[channel].ptr + areas[channel].step *
                                                    frame);
        *ptr = sample;
      }
    }

    seconds_offset =
        fmodf(seconds_offset + seconds_per_frame * frame_count, 1.0f);
    frames_left -= frame_count;

    dtmf_context->samples_index = dtmf_context->samples_index + frame;

    if (err = soundio_outstream_end_write(out_stream)) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(err);
    }
  }
}

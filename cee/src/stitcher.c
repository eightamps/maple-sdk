//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "log.h"
#include "stitcher.h"
#include <errno.h>
#include <soundio/soundio.h>
#include <stdio.h>
#include <stdlib.h>

StitcherContext *stitcher_new(void) {
  StitcherContext *c = NULL;
  c = malloc(sizeof(StitcherContext));
  if (c == NULL) {
    log_err("stitcher_new unable to allocate memory");
    return NULL;
  }
  c->is_active = false;
  c->sample_rate = 0;
  c->soundio = NULL;

  c->from_phone_device = NULL;
  c->to_speaker_device = NULL;
  c->from_mic_device = NULL;
  c->to_phone_device = NULL;

  c->from_phone_stream = NULL;
  c->to_speaker_stream = NULL;
  c->from_mic_stream = NULL;
  c->to_phone_stream = NULL;

  c->to_speaker_name = NULL;
  c->from_mic_name = NULL;
  c->to_phone_name = NULL;
  c->from_phone_name = NULL;
  return c;
}

int stitcher_init(StitcherContext *c) {
  if (c->to_speaker_device != NULL) {
    log_err("stitcher_init called with already-initialized context");
    return EINVAL; // Invalid Argument
  }

  // Create the soundio client
  struct SoundIo *sio = soundio_create();
  if (sio == NULL) {
    log_err("stitcher_init unable to allocate for soundio");
    return ENOMEM;
  }
  c->soundio = sio;

  // Connect the soundio client
  int conn_status = soundio_connect(sio);
  if (conn_status != EXIT_SUCCESS) {
    log_err("stitcher_init unable to connect to soundio");
    return conn_status;
  }
  soundio_flush_events(sio);

  // Get the default output device samples_index
  int index = soundio_default_output_device_index(sio);
  if (index < EXIT_SUCCESS) {
    log_err("stitcher_init unable to get default output device index");
    return ENXIO; // No such device or address.
  }

  // Get the default output device
  struct SoundIoDevice *device = soundio_get_output_device(sio, index);
  if (device == NULL) {
    log_err("stitcher_init unable to get default output device");
    return ENOMEM;
  }
  log_info("stitcher_init default output device: %s", device->name)
  c->to_speaker_device = device;

  struct SoundIoOutStream *to_speaker_stream = soundio_outstream_create(c->to_speaker_device);
  to_speaker_stream->format = SoundIoFormatFloat32NE;
  c->to_speaker_stream = to_speaker_stream;

  return EXIT_SUCCESS;
}

int stitcher_start(StitcherContext *c, StitcherCallback *cb) {
  if (cb == NULL) {
    log_err("stitcher_start cannot start with a NULL callback");
    return EINVAL; // Invalid Argument
  }

  if (c->soundio == NULL) {
    int init_status = stitcher_init(c);
    if (init_status != EXIT_SUCCESS) {
      log_err("stitcher_start failed to automatically initialize");
      return init_status;
    }
  }

  struct SoundIoOutStream *out_stream = c->to_speaker_stream;
  if (out_stream == NULL) {
    log_err("stitcher_start unable to get to_speaker_device stream");
    return EINVAL; // Invalid Argument
  }

  out_stream->write_callback = cb;

  int out_status = soundio_outstream_open(out_stream);
  if (out_status != EXIT_SUCCESS) {
    log_err("stitcher_start unable to open output stream");
    return out_status;
  }

  if (out_stream->layout_error) {
    log_err("stitcher_start unable to set channel layout: %s",
        soundio_strerror(out_stream->layout_error));
  }

  int out_start_status = soundio_outstream_start(out_stream);
  if (out_start_status != EXIT_SUCCESS) {
    log_err("stitcher_start unable to start output stream");
    return out_start_status;
  }

  c->is_active = true;
  soundio_wait_events(c->soundio);
  c->is_active = false;

  log_info("stitcher_start success");
  return EXIT_SUCCESS;
}

void stitcher_free(StitcherContext *c) {
  if (c == NULL) return;

  if (c->to_speaker_device != NULL) {
    soundio_device_unref(c->to_speaker_device);
  }

  if (c->soundio != NULL) {
    soundio_destroy(c->soundio);
  }

  free(c);
}

void dtmf_soundio_callback(struct SoundIoOutStream *out_stream,
                           __attribute__((unused)) int frame_count_min, int frame_count_max) {
  DtmfContext *dtmf_context = (DtmfContext *)out_stream->userdata;
  int err;
  struct SoundIoChannelArea *areas;

  if (dtmf_context->is_complete) {
    return;
  }

  const struct SoundIoChannelLayout *layout = &out_stream->layout;

  if ((err = soundio_outstream_begin_write(out_stream, &areas,
      &frame_count_max))) {
    log_err("dtmf_soundio_callback unable to begin_write with: %s",
      soundio_strerror(err));
    return;
  }

  float sample;
  for (int i = 0; i < frame_count_max; i++) {
    sample = dtmf_next_sample(dtmf_context);
    // printf("%d - %f\n", i, sample);
    for (int channel = 0; channel < layout->channel_count; channel++) {
      float *ptr = (float *) (areas[channel].ptr + areas[channel].step * i);
      *ptr = sample;
    }
  }

  err = soundio_outstream_end_write(out_stream);
  if (err != EXIT_SUCCESS) {
    log_err("dtmf_soundio_callback unable to end_write with %s",
        soundio_strerror(err));
  }
}

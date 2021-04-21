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

#define DEFAULT_TELEPHONE_DEVICE_NAME "ASI Telephone"

StitcherContext *stitcher_new(void) {
  StitcherContext *c = NULL;
  c = malloc(sizeof(StitcherContext));
  if (c == NULL) {
    log_err("stitcher_new unable to allocate memory");
    return NULL;
  }
  c->is_active = false;
  c->soundio = NULL;

  c->to_phone = malloc(sizeof(StitcherOutDevice));
  if (c->to_phone == NULL) {
    log_err("stitcher unable to allocate to_phone");
    stitcher_free(c);
    return NULL;
  }
  c->to_phone->name = NULL;
  c->to_phone->device = NULL;
  c->to_phone->stream = NULL;

  c->to_speaker = malloc(sizeof(StitcherOutDevice));
  if (c->to_speaker == NULL) {
    log_err("stitcher unable to allocate to_speaker");
    stitcher_free(c);
    return NULL;
  }
  c->to_speaker->name = NULL;
  c->to_speaker->device = NULL;
  c->to_speaker->stream = NULL;

  c->from_phone = malloc(sizeof(StitcherInDevice));
  if (c->from_phone == NULL) {
    log_err("stitcher unable to allocate from_phone");
    stitcher_free(c);
    return NULL;
  }
  c->from_phone->name = NULL;
  c->from_phone->device = NULL;
  c->from_phone->stream = NULL;

  c->from_mic = malloc(sizeof(StitcherInDevice));
  if (c->from_mic == NULL) {
    log_err("stitcher unable to allocate from_mic");
    stitcher_free(c);
    return NULL;
  }
  c->from_mic->name = NULL;
  c->from_mic->device = NULL;
  c->from_mic->stream = NULL;

  return c;
}

static struct SoundIoDevice *get_input_device_matching(StitcherContext *c,
    char *matcher) {
  int count = soundio_input_device_count(c->soundio);
  for (int i = 0; i < count; i++) {
    struct SoundIoDevice *d = soundio_get_input_device(c->soundio, i);
    if (strstr(d->name, matcher) != NULL) {
      return d;
    }
  }

  return NULL;
}

static struct SoundIoDevice *get_output_device_matching(StitcherContext *c,
    char *matcher) {
  int count = soundio_output_device_count(c->soundio);
  for (int i = 0; i < count; i++) {
    struct SoundIoDevice *d = soundio_get_output_device(c->soundio, i);
    if (strstr(d->name, matcher) != NULL) {
      return d;
    }
  }

  return NULL;
}

static int init_from_device(StitcherContext *c, StitcherInDevice *sin,
    bool is_default) {
  // Get the output device samples_index
  struct SoundIoDevice *device;

  if (is_default) {
    // Get the default input device samples_index
    int index = soundio_default_input_device_index(c->soundio);
    if (index < EXIT_SUCCESS) {
      log_err("stitcher_init unable to get default input device index");
      return ENXIO; // No such device or address.
    }
    // Get the default input device
    device = soundio_get_input_device(c->soundio, index);
    if (strstr(device->name, DEFAULT_TELEPHONE_DEVICE_NAME) != NULL) {
      log_err("ASI Telephone must not be a default device");
      return ENXIO;
    }
  } else {
    device = get_input_device_matching(c, DEFAULT_TELEPHONE_DEVICE_NAME);
  }

  if (device == NULL) {
    log_err("Unable to initialize expected audio device");
    return ENXIO; // No such device or address;
  }

  sin->device = device;
  strcpy(sin->device->name, device->name);
  log_info("stitcher_init from_device: %s", sin->device->name);

  struct SoundIoInStream *stream = soundio_instream_create(device);
  stream->format = SoundIoFormatFloat32NE;
  sin->stream = stream;

  return EXIT_SUCCESS;
}

static int init_to_device(StitcherContext *c, StitcherOutDevice *sout,
    bool is_default) {
  // Get the output device samples_index
  struct SoundIoDevice *device;
  if (is_default) {
    int index = soundio_default_output_device_index(c->soundio);
    if (index < EXIT_SUCCESS) {
      log_err("stitcher_init unable to get default output device index");
      return ENXIO; // No such device or address.
    }
    device = soundio_get_output_device(c->soundio, index);
    if (strstr(device->name, DEFAULT_TELEPHONE_DEVICE_NAME) != NULL) {
      log_err("ASI Telephone must not be a default device");
      return ENXIO;
    }
  } else {
    device = get_output_device_matching(c, DEFAULT_TELEPHONE_DEVICE_NAME);
  }

  if (device == NULL) {
    log_err("Unable to initialize expected audio device");
    return ENXIO; // No such device or address;
  }

  sout->device = device;
  strcpy(sout->device->name, device->name);
  log_info("stitcher_init to_device: %s", sout->device->name);

  struct SoundIoOutStream *stream = soundio_outstream_create(device);
  stream->format = SoundIoFormatFloat32NE;
  sout->stream = stream;

  return EXIT_SUCCESS;
}

static int init_soundio(StitcherContext *c) {
  // Create the soundio client
  struct SoundIo *sio = soundio_create();
  if (sio == NULL) {
    log_err("stitcher_init unable to allocate for soundio");
    return ENOMEM;
  }

  // Assign to context so that it can be freed later.
  c->soundio = sio;

  // Connect the soundio client
  int status = soundio_connect(sio);
  if (status != EXIT_SUCCESS) {
    log_err("stitcher_init unable to connect to soundio");
    return status;
  }

  soundio_flush_events(sio);
  return EXIT_SUCCESS;
}

int stitcher_init(StitcherContext *c) {
  int status;

  status = init_soundio(c);
  if (status != EXIT_SUCCESS) {
    return status;
  }

  status = init_from_device(c, c->from_phone, false);
  if (status != EXIT_SUCCESS) {
    return status;
  }

  status = init_to_device(c, c->to_phone, false);
  if (status != EXIT_SUCCESS) {
    return status;
  }

  status = init_from_device(c, c->from_mic, true);
  if (status != EXIT_SUCCESS) {
    return status;
  }

  status = init_to_device(c, c->to_speaker, true);
  if (status != EXIT_SUCCESS) {
    return status;
  }

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

  struct SoundIoOutStream *out_stream = c->to_speaker->stream;
  if (out_stream == NULL) {
    log_err("stitcher_start unable to get to_speaker->device->stream");
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

static void out_device_free(StitcherOutDevice *d) {
  if (d != NULL) {
    if (d->device != NULL) {
      soundio_device_unref(d->device);
    }
    if (d->stream != NULL) {
      free(d->stream);
    }

    free(d);
  }
}

static void in_device_free(StitcherInDevice *d) {
  if (d != NULL) {
    if (d->stream != NULL) {
      free(d->stream);
    }
    if (d->device != NULL) {
      soundio_device_unref(d->device);
    }
    free(d);
  }
}

void stitcher_free(StitcherContext *c) {
  if (c == NULL) {
    return;
  }

  if (c->to_phone != NULL) {
    out_device_free(c->to_phone);
  }

  if (c->to_speaker != NULL) {
    out_device_free(c->to_speaker);
  }

  if (c->from_phone != NULL) {
    in_device_free(c->from_phone);
  }

  if (c->from_mic != NULL) {
    in_device_free(c->from_mic);
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

  err = soundio_outstream_begin_write(out_stream, &areas, &frame_count_max);
  if (err != EXIT_SUCCESS) {
    log_err("dtmf_soundio_callback unable to begin_write with: %s",
      soundio_strerror(err));
    return;
  }

  float sample;
  for (int i = 0; i < frame_count_max; i++) {
    sample = dtmf_next_sample(dtmf_context);
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

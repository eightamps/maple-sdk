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
#include <string.h>

#define DEFAULT_TELEPHONE_DEVICE_NAME "ASI Telephone"

static int prioritized_sample_rates[] = {
  48000,
  44100,
  96000,
  24000,
  0,
};

static int min_int(int a, int b) {
  return (a < b) ? a : b;
}

static int max_int(int a, int b) {
  return (a > b) ? a : b;
}

struct SoundIoRingBuffer *create_buffer_with(struct SoundIo *sio,
    struct SoundIoInStream *stream) {
  int latency = 40;
  int capacity = latency * 2 * stream->sample_rate * stream->bytes_per_frame;
  struct SoundIoRingBuffer *buffer = soundio_ring_buffer_create(sio, capacity);
  if (buffer == NULL) {
    return NULL;
  }
  return buffer;
}

static void read_callback(struct SoundIoInStream *in_stream,
    int frame_count_min, int frame_count_max) {
  struct StitcherStreamContext *sc = in_stream->userdata;
  struct SoundIoRingBuffer *buffer = sc->buffer;
  if (buffer == NULL) {
    buffer = create_buffer_with(sc->context->soundio, in_stream);
    sc->buffer = buffer;
  }
  struct SoundIoChannelArea *areas;
  int err;

  char *write_ptr = soundio_ring_buffer_write_ptr(buffer);
  int free_bytes = soundio_ring_buffer_free_count(buffer);
  int free_count = free_bytes / in_stream->bytes_per_frame;

  if (free_count < frame_count_min) {
    fprintf(stderr, "ring buffer overflow\n");
    exit(1);
  }

  int write_frames = min_int(free_count, frame_count_max);
  int frames_left = write_frames;
  for (;;) {
    int frame_count = frames_left;
    if ((err = soundio_instream_begin_read(in_stream, &areas, &frame_count))) {
      fprintf(stderr, "begin read error: %s", soundio_strerror(err));
      exit(1);
    }
    if (!frame_count) {
      break;
    }

    if (areas) {
      // Due to an overflow there is a hole. Fill the ring buffer with
      // silence for the size of the hole.
      // memset(write_ptr, 0, frame_count * in_stream->bytes_per_frame);
    // } else {
      for (int frame = 0; frame < frame_count; frame += 1) {
        for (int ch = 0; ch < in_stream->layout.channel_count; ch += 1) {
          memcpy(write_ptr, areas[ch].ptr, in_stream->bytes_per_sample);
          areas[ch].ptr += areas[ch].step;
          write_ptr += in_stream->bytes_per_sample;
        }
      }
    }
    if ((err = soundio_instream_end_read(in_stream))) {
      fprintf(stderr, "end read error: %s", soundio_strerror(err));
      exit(1);
    }
    frames_left -= frame_count;
    if (frames_left <= 0)
      break;
  }
  int advance_bytes = write_frames * in_stream->bytes_per_frame;
  if (advance_bytes > 0) {
    // printf(" read_callback wrote %d bytes\n", advance_bytes);
  }
  soundio_ring_buffer_advance_write_ptr(buffer, advance_bytes);
}

static void write_callback(struct SoundIoOutStream *out_stream,
    int frame_count_min, int frame_count_max) {
  struct StitcherStreamContext *sc = out_stream->userdata;
  struct SoundIoRingBuffer *buffer = sc->buffer;
  if (buffer == NULL) {
    // Wait for the read_callback to create and fill the buffer.
    return;
  }

  struct SoundIoChannelArea *areas;
  int err;

  char *read_ptr = soundio_ring_buffer_read_ptr(buffer);
  int fill_bytes = soundio_ring_buffer_fill_count(buffer);
  int fill_count = fill_bytes / out_stream->bytes_per_frame;

  if (fill_bytes == 0) {
    // Nothing to write into the device.
    return;
  }

  // printf("write_callback fill_bytes: %d\n", fill_bytes);

  if (fill_count > frame_count_max) {
    fprintf(stderr, "ring buffer write overflow\n");
    // TODO(lbayes): DO NOT EXIT FROM LIBRARY...
    exit(1);
  }

  int write_frames = min_int(fill_count, frame_count_max);
  int frames_left = write_frames;
  for (;;) {
    int frame_count = frames_left;
    err = soundio_outstream_begin_write(out_stream, &areas, &frame_count);
    if (err) {
      fprintf(stderr, "begin write error: %s", soundio_strerror(err));
      // TODO(lbayes): DO NOT EXIT FROM LIBRARY...
      exit(1);
    }

    if (!frame_count)
      break;
    if (areas) {
      // Due to an overflow there is a hole. Fill the ring buffer with
      // silence for the size of the hole.
      // memset(read_ptr, 0, frame_count * out_stream->bytes_per_frame);
    // } else {
      for (int frame = 0; frame < frame_count; frame += 1) {
        for (int ch = 0; ch < out_stream->layout.channel_count; ch += 1) {
          memcpy(read_ptr, areas[ch].ptr, out_stream->bytes_per_sample);
          areas[ch].ptr += areas[ch].step;
          read_ptr += out_stream->bytes_per_sample;
        }
      }
    }

    err = soundio_outstream_end_write(out_stream);
    if (err) {
      fprintf(stderr, "end read error: %s", soundio_strerror(err));
      // TODO(lbayes): DO NOT EXIT FROM LIBRARY...
      exit(1);
    }
    frames_left -= frame_count;
    if (frames_left <= 0) {
      break;
    }
  }
  int advance_bytes = write_frames * out_stream->bytes_per_frame;
  printf("write_callback wrote %d bytes\n", advance_bytes);
  soundio_ring_buffer_advance_read_ptr(buffer, advance_bytes);
}

StitcherContext *stitcher_new(void) {
  StitcherContext *c = NULL;
  c = calloc(1, sizeof(StitcherContext));
  if (c == NULL) {
    log_err("stitcher_new unable to allocate memory");
    return NULL;
  }

  c->to_phone = calloc(1, sizeof(StitcherOutDevice));
  if (c->to_phone == NULL) {
    log_err("stitcher unable to allocate to_phone");
    stitcher_free(c);
    return NULL;
  }

  c->to_speaker = calloc(1, sizeof(StitcherOutDevice));
  if (c->to_speaker == NULL) {
    log_err("stitcher unable to allocate to_speaker");
    stitcher_free(c);
    return NULL;
  }

  c->from_phone = calloc(1, sizeof(StitcherInDevice));
  if (c->from_phone == NULL) {
    log_err("stitcher unable to allocate from_phone");
    stitcher_free(c);
    return NULL;
  }

  c->from_mic = calloc(1, sizeof(StitcherInDevice));
  if (c->from_mic == NULL) {
    log_err("stitcher unable to allocate from_mic");
    stitcher_free(c);
    return NULL;
  }

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
  // stream->software_latency = 10;
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

  // struct SoundIoRingBuffer *buff;
  // int latency = 40;
  // int capacity;
  // struct SoundIoInStream *in_stream;
//
  // buff = soundio_ring_buffer_create(c->soundio, capacity);
  // if (buff == NULL) {
    // return ENOMEM;
  // }
  // c->from_phone_buff = buff;

  // in_stream = c->from_mic->stream;
  // capacity = latency * 2 * in_stream->sample_rate *
                 // in_stream->bytes_per_frame;
  // buff = soundio_ring_buffer_create(c->soundio, RING_BUFFER_CAPACITY);
  // if (buff == NULL) {
    // return ENOMEM;
  // }
  // c->to_phone_buff = buff;

  return EXIT_SUCCESS;
}

static int configure_out_stream(StitcherOutDevice *device,
    struct StitcherStreamContext *sc) {
  struct SoundIoOutStream *stream = device->stream;
  int status;

  if (stream == NULL) {
    log_err("stitcher_start unable to get to_speaker->device->stream");
    return EINVAL; // Invalid Argument
  }

  stream->userdata = sc;
  stream->write_callback = write_callback;

  status = soundio_outstream_open(stream);
  if (status != EXIT_SUCCESS) {
    log_err("stitcher_start unable to open stream");
    return status;
  }

  if (stream->layout_error) {
    log_err("stitcher_start unable to set channel layout: %s",
            soundio_strerror(stream->layout_error));
  }

  status = soundio_outstream_start(stream);
  if (status != EXIT_SUCCESS) {
    log_err("stitcher_start unable to start stream");
    return status;
  }

  return EXIT_SUCCESS;
}

static int configure_in_stream(StitcherInDevice *device,
  struct StitcherStreamContext *sc) {
  struct SoundIoInStream *stream = device->stream;
  int status;

  if (stream == NULL) {
    log_err("stitcher_start unable to get to_speaker->device->stream");
    return EINVAL; // Invalid Argument
  }

  stream->userdata = sc;
  stream->read_callback = read_callback;

  status = soundio_instream_open(stream);
  if (status != EXIT_SUCCESS) {
    log_err("stitcher_start unable to open stream");
    return status;
  }

  if (stream->layout_error) {
    log_err("stitcher_start unable to set channel layout: %s",
            soundio_strerror(stream->layout_error));
  }

  status = soundio_instream_start(stream);
  if (status != EXIT_SUCCESS) {
    log_err("stitcher_start unable to start stream");
    return status;
  }

  return EXIT_SUCCESS;
}

static int config_device_pair(StitcherOutDevice *from, StitcherInDevice *to,
    struct StitcherStreamContext *sc) {

  int status;
  status = configure_out_stream(from, sc);
  if (status != EXIT_SUCCESS) {
    fprintf(stderr, "configure_out_stream failed!\n");
    return status;
  }

  status = configure_in_stream(to, sc);
  if (status != EXIT_SUCCESS) {
    fprintf(stderr, "configure_in_stream failed!\n");
    return status;
  }

  return EXIT_SUCCESS;
}

int stitcher_start(StitcherContext *c) {
  int status;

  if (c->soundio == NULL) {
    status = stitcher_init(c);
    if (status != EXIT_SUCCESS) {
      log_err("stitcher_start failed to automatically initialize");
      return status;
    }
  }

  size_t size = sizeof(StitcherStreamContext);
  struct StitcherStreamContext *sc_to_phone = calloc(1, size);
  sc_to_phone->context = c;
  status = config_device_pair(c->to_phone, c->from_mic, sc_to_phone);
  if (status != EXIT_SUCCESS) {
    return status;
  }

  struct StitcherStreamContext *sc_from_phone = calloc(1, size);
  sc_from_phone->context = c;
  status = config_device_pair(c->to_speaker, c->from_phone, sc_from_phone);
  if (status != EXIT_SUCCESS) {
    return status;
  }

  c->is_active = true;
  soundio_wait_events(c->soundio);
  c->is_active = false;

  // TODO(lbayes): free all audio resources here.

  free(sc_to_phone);
  free(sc_from_phone);
  log_info("stitcher_start success");
  return EXIT_SUCCESS;
}

int stitcher_stop(StitcherContext *c) {
  if (c->is_active) {
    soundio_wakeup(c->soundio);
  }
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

  if (c->from_phone_buff != NULL) {
    soundio_ring_buffer_destroy(c->from_phone_buff);
  }

  if (c->to_phone_buff != NULL) {
    soundio_ring_buffer_destroy(c->to_phone_buff);
  }

  free(c);
}

/*
void stitcher_dtmf_callback(struct SoundIoOutStream *out_stream,
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
    log_err("stitcher_dtmf_callback unable to begin_write with: %s",
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
    log_err("stitcher_dtmf_callback unable to end_write with %s",
        soundio_strerror(err));
  }
}
*/

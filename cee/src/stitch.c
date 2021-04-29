//
// Created by lukebayes on 4/25/21.
//

#include "log.h"
#include "stitch.h"
#include <errno.h>
#include <soundio/soundio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int min_int(int a, int b) {
  return (a < b) ? a : b;
}

StitchContext *stitch_new_with_label(const char *label) {
  StitchContext *c = stitch_new();
  c->label = label;
  return c;
}

StitchContext *stitch_new(void) {
  StitchContext *c = calloc(sizeof(StitchContext), 1);
  if (c == NULL) {
    log_err("stitch_new failed to allocate memory");
    return NULL;
  }
  c->is_initialized = false;
  // c->backend = SoundIoBackendAlsa;
  // c->backend = SoundIoBackendPulseAudio;
  c->backend = SoundIoBackendNone;
  c->input_latency = 0.04; // ms
  return c;
}

static void read_callback(struct SoundIoInStream *instream, int frame_count_min, int frame_count_max) {
  StitchContext *c = instream->userdata;
  if (c == NULL) {
    log_err("read_callback unable to get StitchContext");
    return;
  }

  // log_info("<< read_callback with min: %d and max: %d", frame_count_min,
         // frame_count_max);

  struct SoundIoRingBuffer *ring_buffer = c->ring_buffer;
  struct SoundIoChannelArea *areas;
  int err;
  char *write_ptr = soundio_ring_buffer_write_ptr(ring_buffer);
  int free_bytes = soundio_ring_buffer_free_count(ring_buffer);
  int free_count = free_bytes / instream->bytes_per_frame;

  if (frame_count_min > free_count) {
    log_err("ring buffer overflow");
    return;
  }

  int write_frames = min_int(free_count, frame_count_max);
  int frames_left = write_frames;

  for (;;) {
    int frame_count = frames_left;

    if ((err = soundio_instream_begin_read(instream, &areas, &frame_count))) {
      log_err("begin read error: %s", soundio_strerror(err));
      return;
    }

    if (!frame_count) {
      break;
    }

    if (!areas) {
      // Due to an overflow there is a hole. Fill the ring buffer with
      // silence for the size of the hole.
      memset(write_ptr, 0, frame_count * instream->bytes_per_frame);
      log_err("Dropped %d frames due to internal overflow", frame_count);
    } else {
      for (int frame = 0; frame < frame_count; frame += 1) {
        for (int ch = 0; ch < instream->layout.channel_count; ch += 1) {
          memcpy(write_ptr, areas[ch].ptr, instream->bytes_per_sample);
          areas[ch].ptr += areas[ch].step;
          write_ptr += instream->bytes_per_sample;
        }
      }
    }

    if ((err = soundio_instream_end_read(instream))) {
      log_err("end read error: %s", soundio_strerror(err));
      return;
    }

    frames_left -= frame_count;
    if (frames_left <= 0) {
      break;
    }
  }

  int advance_bytes = write_frames * instream->bytes_per_frame;
  // log_info("read_callback advance_bytes: %d", advance_bytes);
  soundio_ring_buffer_advance_write_ptr(ring_buffer, advance_bytes);
}

static void write_callback(struct SoundIoOutStream *outstream,
    int frame_count_min, int frame_count_max) {
  StitchContext *c = outstream->userdata;
  if (c == NULL) {
    log_err("write_callback unable to get StitchContext");
    return;
  }

  // log_info(">> write_callback with min %d and max: %d",
          // frame_count_min, frame_count_max);

  struct SoundIoRingBuffer *ring_buffer = c->ring_buffer;
  struct SoundIoChannelArea *areas;
  int frames_left;
  int frame_count;
  int status;

  char *read_ptr = soundio_ring_buffer_read_ptr(ring_buffer);
  int fill_bytes = soundio_ring_buffer_fill_count(ring_buffer);
  int fill_count = fill_bytes / outstream->bytes_per_frame;

  if (frame_count_min > fill_count) {
    // Ring buffer does not have enough data, fill with zeroes.
    frames_left = frame_count_min;
    for (;;) {
      frame_count = frames_left;
      if (frame_count <= 0) {
        return;
      }
      status = soundio_outstream_begin_write(outstream, &areas, &frame_count);
      if (status) {
        log_err("begin write error: %s", soundio_strerror(status));
        return;
      }

      if (frame_count <= 0) {
        return;
      }

      for (int frame = 0; frame < frame_count; frame += 1) {
        for (int ch = 0; ch < outstream->layout.channel_count; ch += 1) {
          memset(areas[ch].ptr, 0, outstream->bytes_per_sample);
          areas[ch].ptr += areas[ch].step;
        }
      }

      status = soundio_outstream_end_write(outstream);
      if (status) {
        log_err("end write error: %s", soundio_strerror(status));
        return;
      }

      frames_left -= frame_count;
    }
  }

  int read_count = min_int(frame_count_max, fill_count);
  frames_left = read_count;

  while (frames_left > 0) {
    int fframe_count = frames_left;

    status = soundio_outstream_begin_write(outstream, &areas, &fframe_count);
    if (status) {
      log_err("begin write error: %s", soundio_strerror(status));
    }

    if (fframe_count <= 0) {
      break;
    }

    DtmfContext *d = c->dtmf_context;
    float *sample = calloc(sizeof(float), 1);

    for (int frame = 0; frame < fframe_count; frame += 1) {
      for (int ch = 0; ch < outstream->layout.channel_count; ch += 1) {
        if (d && d->is_active) {
          dtmf_next_sample(d, sample);
          memcpy(areas[ch].ptr, sample, outstream->bytes_per_sample);
        } else {
          memcpy(areas[ch].ptr, read_ptr, outstream->bytes_per_sample);
        }
        areas[ch].ptr += areas[ch].step;
        read_ptr += outstream->bytes_per_sample;
      }
    }

    free(sample);
    status = soundio_outstream_end_write(outstream);
    if (status) {
      log_err("end write error: %s", soundio_strerror(status));
      return;
    }

    frames_left -= fframe_count;
  }

  int advance_bytes = read_count * outstream->bytes_per_frame;
  // log_info("write_callback advance_bytes: %d", advance_bytes);
  soundio_ring_buffer_advance_read_ptr(c->ring_buffer, advance_bytes);
}

static void underflow_callback(struct SoundIoOutStream *outstream) {
  static int count = 0;
  StitchContext *c = outstream->userdata;
  log_err("stitch write_underflow count: %d on stream id: %ld", ++count,
          c->thread_id);
  if (count >= 10) {
    log_err("stitch killing thread now due to underflow conditions");
    c->is_active = false;
    c->thread_exit_status = -EIO; // I/O error
    count = 0;
  }
}

int stitch_init(StitchContext *c) {
  int status = EXIT_SUCCESS;
  if (c == NULL) {
    log_err("stitch_init requires a non-null context");
    return -EINVAL;
  }

  if (c->is_initialized) {
    return status;
  }

  struct SoundIo *soundio = soundio_create();

  if (!soundio) {
    log_err("out of memory");
    c->thread_exit_status = -ENOMEM;
    return -ENOMEM;
  }

  if (c->backend == SoundIoBackendNone) {
    status = soundio_connect(soundio);
  } else {
    status = soundio_connect_backend(soundio, c->backend);
  }
  if (status != EXIT_SUCCESS) {
    log_err("stitch_init failed to connect soundio with: %s",
            soundio_strerror(status));
  }

  soundio_flush_events(soundio);
  c->soundio = soundio;
  c->is_initialized = true;

  return status;
}

static void *stitch_start_thread(void *vargp) {
  StitchContext *c = vargp;
  int status;
  struct SoundIo *soundio = c->soundio;

  struct SoundIoDevice *in_device = soundio_get_input_device(soundio,
      c->in_device_index);
  struct SoundIoDevice *out_device = soundio_get_output_device(soundio,
      c->out_device_index);

  log_info("---------------------------------------------");
  log_info("stitch %s input device name: %s", c->label, in_device->name);
  log_info("stitch %s output device name: %s", c->label, out_device->name);

  soundio_device_sort_channel_layouts(out_device);
  const struct SoundIoChannelLayout *layout = soundio_best_matching_channel_layout(
      out_device->layouts, out_device->layout_count,
      in_device->layouts, in_device->layout_count);

  if (!layout) {
    log_err("channel layouts not compatible");
    c->thread_exit_status = -ENOMEM;
    return NULL;
  }

  int *sample_rate;
  for (sample_rate = prioritized_sample_rates; *sample_rate; sample_rate++) {
    if (soundio_device_supports_sample_rate(in_device, *sample_rate) &&
        soundio_device_supports_sample_rate(out_device, *sample_rate)) {
      break;
    }
  }

  if (!*sample_rate) {
    log_err("incompatible sample rates");
    // TODO(lbayes): Consider resampling input to a supported output in this
    //  case...
    c->thread_exit_status = -EINVAL;
    return NULL;
  }

  if (c->dtmf_context != NULL) {
    dtmf_set_sample_rate(c->dtmf_context, *sample_rate);
  }
  log_info("Starting with sample_rate: %d", *sample_rate);

  enum SoundIoFormat *fmt;
  for (fmt = prioritized_formats; *fmt != SoundIoFormatInvalid; fmt += 1) {
    if (soundio_device_supports_format(in_device, *fmt) &&
        soundio_device_supports_format(out_device, *fmt)) {
      break;
    }
  }
  if (*fmt == SoundIoFormatInvalid) {
    log_err("incompatible sample formats");
    c->thread_exit_status = -EINVAL;
    return NULL;
  }

  struct SoundIoInStream *instream = soundio_instream_create(in_device);
  if (!instream) {
    log_err("No instream found");
    c->thread_exit_status = -EINVAL;
    return NULL;
  }
  instream->userdata = c;
  instream->format = *fmt;
  instream->sample_rate = *sample_rate;
  instream->layout = *layout;
  instream->software_latency = c->input_latency;
  instream->read_callback = read_callback;

  if ((status = soundio_instream_open(instream))) {
    log_err("unable to open input stream: %s", soundio_strerror(status));
    c->thread_exit_status = -EINVAL;
    return NULL;
  }

  struct SoundIoOutStream *outstream = soundio_outstream_create(out_device);
  if (!outstream) {
    log_err("out of memory");
    c->thread_exit_status = -ENOMEM;
    return NULL;
  }
  outstream->userdata = c;
  outstream->format = *fmt;
  outstream->sample_rate = *sample_rate;
  outstream->layout = *layout;
  outstream->software_latency = c->input_latency / 2;
  outstream->write_callback = write_callback;
  outstream->underflow_callback = underflow_callback;

  status = soundio_outstream_open(outstream);
  if (status) {
    const char *msg = soundio_strerror(status);
    log_err("unable to open output stream: %s", msg);
    c->thread_exit_status = -EINVAL;
    return NULL;
  }

  int capacity = (c->input_latency * instream->sample_rate *
                 instream->bytes_per_frame) * 40;

  // int capacity = (c->input_latency * 2) * instream->sample_rate *
                 // instream->bytes_per_frame;
  c->ring_buffer = soundio_ring_buffer_create(soundio, capacity);

  log_info("Creating ring_buffer with latency %fs and capacity: %d and "
         " bytes_per_frame: %d",
         c->input_latency, capacity, instream->bytes_per_frame);

  log_info("outstream->bytes_per_frame: %d", outstream->bytes_per_frame);

  if (!c->ring_buffer) {
    log_err("unable to create ring buffer: out of memory");
    return NULL;
  }

  char *buf = soundio_ring_buffer_write_ptr(c->ring_buffer);
  int fill_count = soundio_ring_buffer_free_count(c->ring_buffer);
  // int fill_count = (int)c->input_latency * outstream->sample_rate *
                   // outstream->bytes_per_frame;
  memset(buf, 0x0, fill_count);
  // soundio_ring_buffer_advance_write_ptr(c->ring_buffer, fill_count);
  // soundio_ring_buffer_advance_write_ptr(c->ring_buffer, fill_count);

  if ((status = soundio_instream_start(instream))) {
    log_err("unable to stitch_start input device: %s",
       soundio_strerror(status));
    return NULL;
  }

  sleep(2);

  if ((status = soundio_outstream_start(outstream))) {
    log_err("unable to stitch_start output device: %s",
            soundio_strerror(status));
  }

  c->is_active = true;
  log_info("stitch starting now");

  while (c->is_active == true) {
    // NOTE(lbayes): DO NOT use soundio_wait_events here. It is a blocking call!
    // soundio_wait_events(soundio);
    sleep(0.04);
  }

  log_info("stitch thread finishing");
  soundio_outstream_destroy(outstream);
  soundio_instream_destroy(instream);
  soundio_device_unref(in_device);
  soundio_device_unref(out_device);
  return NULL;
}

int stitch_join(StitchContext *c) {
  pthread_join(c->thread_id, NULL);
  return c->thread_exit_status;
}

static int is_valid_host_device(struct SoundIoDevice *d) {
  return (strstr(d->name, STITCH_ASI_TELEPHONE) == NULL) &&
         (strstr(d->name, STITCH_ASI_MICROPHONE) == NULL) &&
         (strstr(d->name, STITCH_SPDIF) == NULL) &&
         (strstr(d->name, STITCH_WAY_2_CALL) == NULL) &&
         (strstr(d->name, STITCH_WAY_2_CALL_LOWER) == NULL) &&
         (strstr(d->name, STITCH_PULSE_AUDIO) == NULL);
}

typedef struct SoundIoDevice *(GetDeviceFunc)(struct SoundIo *s, int index);

static int get_default_device_index(StitchContext *c,
                                    GetDeviceFunc *get_device,
                                    int device_count,
                                    int default_index) {
  struct SoundIoDevice *d;
  // TODO(lbayes): Figure out how to get the default_communication device on
  //  Windows.
  d = get_device(c->soundio, default_index);
  if (is_valid_host_device(d)) {
    soundio_device_unref(d);
    return default_index;
  }

  int result = -1;
  int index = device_count - 1;
  while (index >= 0) {
    if (index == default_index) {
      index--;
      continue;
    }
    d = get_device(c->soundio, index);
    if (is_valid_host_device(d)) {
      result = index;
      soundio_device_unref(d);
      break;
    }
    soundio_device_unref(d);
    index--;
  }

  return result;
}

static int get_matching_device_index(StitchContext *c,
                                    GetDeviceFunc *get_device,
                                    int device_count,
                                    char *name) {
  struct SoundIoDevice *d;
  int result = -1;
  int index = device_count - 1;
  while (index >= 0) {
    d = get_device(c->soundio, index);
    if ((strstr(d->name, name) != NULL)) {
      result = index;
      soundio_device_unref(d);
      break;
    }
    soundio_device_unref(d);
    index--;
  }

  return result;
}

int stitch_set_dtmf(StitchContext *c, DtmfContext *d) {
  c->dtmf_context = d;
}

int stitch_get_default_input_index(StitchContext *c) {
  int count = soundio_input_device_count(c->soundio);
  int index = soundio_default_input_device_index(c->soundio);
  return get_default_device_index(c, &soundio_get_input_device, count, index);
}

int stitch_get_default_output_index(StitchContext *c) {
  int count = soundio_output_device_count(c->soundio);
  int index = soundio_default_output_device_index(c->soundio);
  return get_default_device_index(c, &soundio_get_output_device, count, index);
}

int stitch_get_matching_input_device_index(StitchContext *c, char *name) {
  int count = soundio_input_device_count(c->soundio);
  return get_matching_device_index(c, &soundio_get_input_device, count, name);
}

int stitch_get_matching_output_device_index(StitchContext *c, char *name) {
  int count = soundio_output_device_count(c->soundio);
  return get_matching_device_index(c, &soundio_get_output_device, count, name);
}

int stitch_start(StitchContext *c, int in_index, int out_index) {
  c->in_device_index = in_index;
  c->out_device_index = out_index;
  return pthread_create(&c->thread_id, NULL, stitch_start_thread, c);
}

int stitch_stop(StitchContext *c) {
  c->is_active = false;
  return stitch_join(c);
}

void stitch_free(StitchContext *c) {
  if (c != NULL) {
    if (c->is_active) {
      c->is_active = false;
      stitch_join(c);
    }
    if (c->ring_buffer) {
      soundio_ring_buffer_destroy(c->ring_buffer);
    }
    if (c->soundio != NULL) {
      soundio_destroy(c->soundio);
    }
    free(c);
  }
}

enum SoundIoBackend stitch_get_backend_from_label(char *label) {
  if (strcmp("none", label) == 0 ||
      strcmp("", label) == 0) {
    return SoundIoBackendNone;
  } else if (strcmp("dummy", label) == 0) {
    return SoundIoBackendDummy;
  } else if (strcmp("alsa", label) == 0) {
    return SoundIoBackendAlsa;
  } else if (strcmp("pulseaudio", label) == 0) {
    return SoundIoBackendPulseAudio;
  } else if (strcmp("jack", label) == 0) {
    return SoundIoBackendJack;
  } else if (strcmp("coreaudio", label) == 0) {
    return SoundIoBackendCoreAudio;
  } else if (strcmp("wasapi", label) == 0) {
    return SoundIoBackendWasapi;
  }

  log_err("Invalid backend: %s", label);
  return -EINVAL;
}

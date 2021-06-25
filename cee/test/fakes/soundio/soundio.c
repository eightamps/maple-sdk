#include "soundio.h"
#include <stdlib.h>

//---------------------------------------------------------
// soundio
//---------------------------------------------------------

static struct SoundIo *context;
static bool use_fake_context = false;
static int fake_connect_status = 0;

SOUNDIO_EXPORT struct SoundIo *soundio_create(void) {
  if (!use_fake_context) {
    struct SoundIo *s = malloc(sizeof(struct SoundIo));
    return s;
  }
  use_fake_context = false;
  return context;
}

void soundio_fake_create_returns(struct SoundIo *sio) {
  use_fake_context = true;
  context = sio;
}

SOUNDIO_EXPORT int soundio_connect(struct SoundIo *soundio) {
  int status = fake_connect_status;
  fake_connect_status = 0;
  return status; 
}

void soundio_fake_connect_returns(int status) {
  fake_connect_status = status;
}

SOUNDIO_EXPORT int soundio_connect_backend(struct SoundIo *soundio, enum SoundIoBackend backend) {
  return soundio_connect(soundio);
}

SOUNDIO_EXPORT void soundio_flush_events(struct SoundIo *soundio) {
}

SOUNDIO_EXPORT void soundio_disconnect(struct SoundIo *soundio) {
}

SOUNDIO_EXPORT void soundio_sort_channel_layouts(struct SoundIoChannelLayout *layouts, int layout_count) {
}

SOUNDIO_EXPORT const char *soundio_strerror(int error) {
  return "fake-error";
}

SOUNDIO_EXPORT void soundio_destroy(struct SoundIo *soundio) {
  free(soundio);
}

//---------------------------------------------------------
// devices
//---------------------------------------------------------

SOUNDIO_EXPORT bool soundio_device_supports_format(struct SoundIoDevice *device,
        enum SoundIoFormat format) {
  return false;
}

SOUNDIO_EXPORT bool soundio_device_supports_layout(struct SoundIoDevice *device,
        const struct SoundIoChannelLayout *layout) {
  return false;
}

SOUNDIO_EXPORT bool soundio_device_supports_sample_rate(struct SoundIoDevice *device,
        int sample_rate) {
  return false;
}

SOUNDIO_EXPORT void soundio_device_sort_channel_layouts(struct SoundIoDevice *device) {
}

SOUNDIO_EXPORT const struct SoundIoChannelLayout *soundio_best_matching_channel_layout(
        const struct SoundIoChannelLayout *preferred_layouts, int preferred_layout_count,
        const struct SoundIoChannelLayout *available_layouts, int available_layout_count) {
  return NULL;
}

SOUNDIO_EXPORT struct SoundIoDevice *soundio_get_input_device(struct SoundIo *soundio, int index) {
  return NULL;
}

SOUNDIO_EXPORT struct SoundIoDevice *soundio_get_output_device(struct SoundIo *soundio, int index) {
  return NULL;
}

SOUNDIO_EXPORT int soundio_default_input_device_index(struct SoundIo *soundio) {
  return -1;
}

SOUNDIO_EXPORT int soundio_default_output_device_index(struct SoundIo *soundio) {
  return -1;
}

SOUNDIO_EXPORT int soundio_input_device_count(struct SoundIo *soundio) {
  return 0;
}

SOUNDIO_EXPORT int soundio_output_device_count(struct SoundIo *soundio) {
  return 0;
}

SOUNDIO_EXPORT void soundio_device_unref(struct SoundIoDevice *device) {
}

//---------------------------------------------------------
// out_stream
//---------------------------------------------------------
SOUNDIO_EXPORT struct SoundIoOutStream *soundio_outstream_create(struct SoundIoDevice *device) {
  return NULL;
}

SOUNDIO_EXPORT int soundio_outstream_open(struct SoundIoOutStream *outstream) {
  return -1;
}

SOUNDIO_EXPORT int soundio_outstream_start(struct SoundIoOutStream *outstream) {
  return -1;
}

SOUNDIO_EXPORT int soundio_outstream_begin_write(struct SoundIoOutStream *outstream,
        struct SoundIoChannelArea **areas, int *frame_count) {
  return -1;
}

SOUNDIO_EXPORT int soundio_outstream_end_write(struct SoundIoOutStream *outstream) {
  return -1;
}

SOUNDIO_EXPORT void soundio_outstream_destroy(struct SoundIoOutStream *outstream) {
}

//---------------------------------------------------------
// in_stream
//---------------------------------------------------------
SOUNDIO_EXPORT struct SoundIoInStream *soundio_instream_create(struct SoundIoDevice *device) {
  return NULL;
}

SOUNDIO_EXPORT int soundio_instream_open(struct SoundIoInStream *instream) {
  return -1;
}

SOUNDIO_EXPORT int soundio_instream_start(struct SoundIoInStream *instream) {
  return -1;
}

SOUNDIO_EXPORT int soundio_instream_begin_read(struct SoundIoInStream *instream,
        struct SoundIoChannelArea **areas, int *frame_count) {
  return -1;
}

SOUNDIO_EXPORT int soundio_instream_end_read(struct SoundIoInStream *instream) {
  return -1;
}

SOUNDIO_EXPORT void soundio_instream_destroy(struct SoundIoInStream *instream) {
}



//---------------------------------------------------------
// ring_buffer
//---------------------------------------------------------
SOUNDIO_EXPORT struct SoundIoRingBuffer *soundio_ring_buffer_create(struct SoundIo *soundio, int requested_capacity) {
  return NULL;
}

SOUNDIO_EXPORT void soundio_ring_buffer_destroy(struct SoundIoRingBuffer *ring_buffer) {
}

SOUNDIO_EXPORT int soundio_ring_buffer_capacity(struct SoundIoRingBuffer *ring_buffer) {
  return 0;
}

SOUNDIO_EXPORT int soundio_ring_buffer_fill_count(struct SoundIoRingBuffer *ring_buffer) {
  return 0;
}

SOUNDIO_EXPORT int soundio_ring_buffer_free_count(struct SoundIoRingBuffer *ring_buffer) {
  return 0;
}

SOUNDIO_EXPORT char *soundio_ring_buffer_read_ptr(struct SoundIoRingBuffer *ring_buffer) {
  return "a";
}

SOUNDIO_EXPORT void soundio_ring_buffer_advance_read_ptr(struct SoundIoRingBuffer *ring_buffer, int count) {
}

SOUNDIO_EXPORT char *soundio_ring_buffer_write_ptr(struct SoundIoRingBuffer *ring_buffer) {
  return "a";
}

SOUNDIO_EXPORT void soundio_ring_buffer_advance_write_ptr(struct SoundIoRingBuffer *ring_buffer, int count) {
}



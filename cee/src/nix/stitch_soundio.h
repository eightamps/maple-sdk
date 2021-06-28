#ifndef __stitch_soundio_h__
#define __stitch_soundio_h__

#include <soundio/soundio.h>

typedef struct {
  struct SoundIo *soundio;
  enum SoundIoBackend backend;
  struct SoundIoRingBuffer *ring_buffer;
}soundio_platform_t;

#endif // __stitch_soundio_h__


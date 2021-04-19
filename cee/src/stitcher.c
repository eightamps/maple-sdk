//
// Created by lukebayes on 4/19/21.
//

#include "stitcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <soundio/soundio.h>
#include <errno.h>


StitcherContext *stitcher_new(void) {
  StitcherContext *c = malloc(sizeof(StitcherContext));
  if (c == NULL) return NULL;
}

int stitcher_init(StitcherContext *c) {
  // Create the soundio client
  struct SoundIo *sio = soundio_create();
  if (sio == NULL) return ENOMEM;

  // Connect the soundio client
  int conn_status = soundio_connect(sio);
  if (conn_status != EXIT_SUCCESS) return conn_status;
  soundio_flush_events(sio);

  // Get the default output device index
  int index = soundio_default_output_device_index(sio);
  if (index < 0) return ENXIO; // No such device or address.

  // Get the default output device
  struct SoundIoDevice *device = soundio_get_output_device(sio, index);
  if (device == NULL) return ENOMEM;

  c->to_speaker = device;

  fprintf(stderr, "Output device: %s\n", device->name);
  return EXIT_SUCCESS;
}

int stitcher_start(StitcherContext *c) {
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

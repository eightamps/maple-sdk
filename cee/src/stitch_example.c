/*
 * Copyright (c) 2015 Andrew Kelley
 *
 * This file is part of libsoundio, which is MIT licensed.
 * See http://opensource.org/licenses/MIT
 */

#include "stitch.h"
// #include <soundio/soundio.h>
// #include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
__attribute__ ((cold))
__attribute__ ((noreturn))
__attribute__ ((format (printf, 1, 2)))
static void panic(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  abort();
}
 */

static int usage(char *exe) {
  fprintf(stderr, "Usage: %s [options]\n"
                  "Options:\n"
                  "  [--backend dummy|alsa|pulseaudio|jack|coreaudio|wasapi]\n"
                  "  [--in-device id]\n"
                  "  [--in-raw]\n"
                  "  [--out-device id]\n"
                  "  [--out-raw]\n"
                  "  [--latency seconds]\n"
      , exe);
  return 1;
}

int main(int argc, char **argv) {
  StitchContext *c = stitch_new();
  char *exe = argv[0];

  for (int i = 1; i < argc; i++) {
    char *arg = argv[i];
    if (arg[0] == '-' && arg[1] == '-') {
      if (strcmp(arg, "--in-raw") == 0) {
        c->in_raw = true;
      } else if (strcmp(arg, "--out-raw") == 0) {
        c->out_raw = true;
      } else if (++i >= argc) {
        return usage(exe);
      } else if (strcmp(arg, "--backend") == 0) {
        c->backend = stitch_get_backend_from_label(argv[i]);
      } else if (strcmp(arg, "--in-device") == 0) {
        c->in_device_id = argv[i];
      } else if (strcmp(arg, "--out-device") == 0) {
        c->out_device_id = argv[i];
      } else if (strcmp(arg, "--latency") == 0) {
        c->input_latency = atof(argv[i]);
      } else {
        return usage(exe);
      }
    } else {
      return usage(exe);
    }
  }

  stitch_start(c);
  int incr = 0;
  while (incr++ < 5) {
    printf("main looping\n");
    sleep(1);
  }
  stitch_stop(c);
  stitch_join(c);
  stitch_free(c);
}

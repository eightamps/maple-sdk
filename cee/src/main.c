#include "phony.h"
#include <stdio.h>
#include <unistd.h>
#include "pthread_play.h"

#define DEFAULT_8A_PHONE_NUMBER "7273392258"

int phony_exercise(void) {
  PhonyContext *c = phony_new();
  int status = phony_open_maple(c);
  if (status < 0) {
    return status;
  }

  status = phony_dial(c, DEFAULT_8A_PHONE_NUMBER);
  printf("phony_dial complete\n");

  if (status < 0) {
    return status;
  }

  sleep(10);

  phony_hang_up(c);
  phony_free(c);
  printf("EXITING\n");
}

int stitcher_exercise(void) {
  int status = 0;
  StitcherContext *c = stitcher_new();
  if (c == NULL) {
    fprintf(stderr, "stitcher_new failed\n");
    return -1;
  }

  c->to_phone_device_name = "sof-hda";
  c->from_phone_device_name = "sof-hda";

  status = stitcher_init(c);
  if (status < 0) {
    fprintf(stderr, "stitcher_init failed with %d\n", status);
    return status;
  }

  status = stitcher_start(c);
  if (status < 0) {
    fprintf(stderr, "stitcher_init failed with %d\n", status);
    return status;
  }

  // Wait 5 seconds and then ask stitcher to exit gracefully
  sleep(5);
  stitcher_stop(c);

  status = stitcher_join(c);
  if (status < 0) {
    fprintf(stderr, "stitcher_join failed with %d\n", status);
    return status;
  }

  return status;
}

int main() {
  return stitcher_exercise();
  // return phony_exercise();
  // return thread_play_start();
}

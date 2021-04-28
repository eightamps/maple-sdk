#include "log.h"
#include "phony.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

  sleep(30);

  phony_hang_up(c);
  phony_free(c);
  printf("EXITING\n");
}

int stitcher_exercise(void) {
  /*
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
   */
  return 0;
}

int stitch_exercise(void) {
  StitchContext *c = stitch_new();
  if (c == NULL) {
    log_err("Failed creation");
    return -ENOMEM;
  }

  int status = stitch_init(c);
  if (status != EXIT_SUCCESS) {
    log_err("Failed init");
    return status;
  }
  log_info("init success");

  status = stitch_start(c);
  if (status != EXIT_SUCCESS) {
    log_err("Failed start");
    return status;
  }
  log_info("start success");

  sleep(10);

  log_info("sleep finished, stopping now");
  status = stitch_stop(c);
  if (status != EXIT_SUCCESS) {
    log_err("Failed stop");
    return status;
  }
  log_info("stop finished");

  return status;
}

int main() {
  return stitch_exercise();
  // return stitcher_exercise();
  // return phony_exercise();
  // return thread_play_start();
}

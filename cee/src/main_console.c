#include "log.h"
#include "phony.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_8A_PHONE_NUMBER "7273392258"

static phony_context_t *phony_context;
static stitch_context_t *stitch_context;

void sig_term_handler(int a, siginfo_t *t, void *b) {
  if (phony_context != NULL) {
    phony_free(phony_context);
  }
  if (stitch_context != NULL) {
    stitch_free(stitch_context);
  }
}

void catch_sigterm(void) {
  static struct sigaction _sigact;

  memset(&_sigact, 0, sizeof(_sigact));
  _sigact.sa_sigaction = sig_term_handler;
  _sigact.sa_flags = SA_SIGINFO;
  sigaction(SIGTERM, &_sigact, NULL);
}

int phony_example(void) {
  log_info("phony_example starting");

  phony_context_t *c = phony_new();
  if (c == NULL) {
    log_err("unable to instantiate phony context");
    return -ENOMEM; // No memory
  }

  phony_context = c;

  int status = phony_open_maple(c);
  if (status < 0) {
    log_err("phony_open_maple failed with status: %d", status);
    return status;
  }

  // Give the system some time to establish HID connections.
  sleep(1);

  status = phony_dial(c, DEFAULT_8A_PHONE_NUMBER);
  if (status < 0) {
    log_err("phony_dial failed with status: %d", status);
    return status;
  }

  sleep(30);

  status = phony_hang_up(c);
  if (status != EXIT_SUCCESS) {
    log_err("phony_hang_up failed with: %d", status);
  }

  phony_free(c);

  log_info("phony_example exiting now with 0 success");
  return EXIT_SUCCESS;
}

int stitch_example(void) {
  stitch_context_t *c = stitch_new_with_label("example");
  stitch_context = c;
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

  int in_index = stitch_get_default_input_index(c);
  int out_index = stitch_get_default_output_index(c);

  status = stitch_start(c, in_index, out_index);
  if (status != EXIT_SUCCESS) {
    log_err("Failed start");
    return status;
  }
  log_info("start success");

  log_info("sleeping for some seconds");
  sleep(10);
  log_info("sleep finished, ending now");

  status = stitch_stop(c);
  if (status != EXIT_SUCCESS) {
    log_err("Failed stop");
    return status;
  }
  log_info("stop finished");

  return status;
}

int main() {
  catch_sigterm();
  return phony_example();
  // return stitch_example();
}

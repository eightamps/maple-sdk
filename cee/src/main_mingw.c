#include "log.h"
#include "stitch.h"
#include <stdio.h>

int main(void) {
  log_info("Main started");
  stitch_context_t *c = stitch_new_with_label("from_phone");
  log_info("stitch_new returned context: %x", c);
  stitch_init(c);

  /*
  int in_index = stitch_get_default_input_index(c);
  if (in_index < 0) {
    log_err("stitch_get_default_input_index failed with: %d", in_index);
  } else {
    log_info("stitch_get_default_input_index returned: %d", in_index);
  }

  int out_index = stitch_get_default_output_index(c);
  if (out_index < 0) {
    log_err("stitch_get_default_input_index failed with: %d", out_index);
  } else {
    log_info("stitch_get_default_output_index returned: %d", out_index);
  }
  */

  int status = stitch_start(c, -1, -1);
  if (status != 0) {
    log_err("stitch_start FAILED: %d", status);
  } else {
    log_info("stitch_start returned: %d", status);
  }

  stitch_free(c);
  return 0;
}

/*
typedef int (__cdecl *stitch_new_with_label_t)(LPWSTR);

int main(void) {
  printf("MINGW Main Started\n");
  HINSTANCE hinstLib;
  stitch_new_with_label_t stitch_new_with_label;
  BOOL fFreeResult, fRunTimeLinkSuccess = FALSE;

  // Get a handle to the DLL Module.
  hinstLib = LoadLibrary(TEXT("maple-sdk.dll"));

  printf("LoadLibrary result: 0x%x\n", hinstLib);
  if (hinstLib != NULL) {
    stitch_new_with_label = (stitch_new_with_label_t) GetProcAddress(hinstLib,
                                                          "stitch_new_with_label");
    int result = stitch_new_with_label("from_phone");

    printf("stitch_new_with_label result: %d\n", result);
    // If the function address is valid, call the function.
    if (NULL != stitch_new_with_label) {
      printf("inside bee 0x%x\n", stitch_new_with_label);
      fRunTimeLinkSuccess = TRUE;
      (stitch_new_with_label)(L"Message sent to the DLL function\n");
    }
    printf("inside cee\n");
    // Free the DLL module.
    fFreeResult = FreeLibrary(hinstLib);
  }

  if (!fRunTimeLinkSuccess) {
    printf("Message printed from executable\n");
  }

  return 0;
}
*/


// #include "phony.h"
#include <stdlib.h>
#include <unistd.h>
// #include <signal.h>

// #define DEFAULT_8A_PHONE_NUMBER "7273392258"

// static phony_context_t *phony_context;
// static stitch_context_t *stitch_context;

/*
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

int main() {
  // catch_sigterm();
  // return phony_example();
  printf("YOOOOOOOOOOOOOOOOOO\n");
}
*/

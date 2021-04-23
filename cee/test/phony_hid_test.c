//
// Created by lukebayes on 4/23/21.
//

#include "phony_hid.h"
#include "phony_hid_test.h"
#include "minunit.h"
#include <string.h>

char *test_phony_hid_state(void) {
  const char *one = phony_hid_state_to_str(PHONY_NOT_READY);
  muAssert(strcmp("Not ready", one) == 0, "Expected Not ready");

  const char *two = phony_hid_state_to_str(PHONY_READY);
  muAssert(strcmp("Ready", two) == 0, "Expected Ready");

  const char *three = phony_hid_state_to_str(PHONY_OFF_HOOK);
  muAssert(strcmp("Off hook", three) == 0, "Expected Off hook");

  const char *four = phony_hid_state_to_str(PHONY_RINGING);
  muAssert(strcmp("Ringing", four) == 0, "Expected Ringing");

  const char *five = phony_hid_state_to_str(PHONY_LINE_NOT_FOUND);
  muAssert(strcmp("Line not found", five) == 0, "Expected Line not found");

  const char *six = phony_hid_state_to_str(PHONY_LINE_IN_USE);
  muAssert(strcmp("Line in use", six) == 0, "Expected Line in use");

  const char *seven = phony_hid_state_to_str(PHONY_HOST_NOT_FOUND);
  muAssert(strcmp("Host not found", seven) == 0, "Expected Host not found");
  return NULL;
}

char *test_phony_hid_new(void) {
  PhonyHidContext *c = phony_hid_new();
  muAssert(c != NULL, "Expected Not Null");
  phony_hid_free(c);
  return NULL;
}

// NOTE(lbayes): This test only passes if a Maple board is connect to the USB
// bus. It's not clear to me (yet) how best to stub libusb services in C.
char *test_phony_hid_open(void) {
  PhonyHidContext *c = phony_hid_new();
  int status = phony_hid_open(c, EIGHT_AMPS_VID, MAPLE_V3_PID);
  muAssert(status >= 0, "Expected valid status from phony_hid_open");
  muAssert(c->is_open, "Expected is_open");

  phony_hid_free(c);
  return NULL;
}

#include "minunit.h"
#include "phony.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char *test_phony_state(void) {
  const char *one = phony_state_to_str(PHONY_NOT_READY);
  muAssert(strcmp("Not ready", one) == 0, "Expected Not ready");

  const char *two = phony_state_to_str(PHONY_READY);
  muAssert(strcmp("Ready", two) == 0, "Expected Ready");

  const char *three = phony_state_to_str(PHONY_OFF_HOOK);
  muAssert(strcmp("Off hook", three) == 0, "Expected Off hook");

  const char *four = phony_state_to_str(PHONY_RINGING);
  muAssert(strcmp("Ringing", four) == 0, "Expected Ringing");

  const char *five = phony_state_to_str(PHONY_LINE_NOT_FOUND);
  muAssert(strcmp("Line not found", five) == 0, "Expected Line not found");

  const char *six = phony_state_to_str(PHONY_LINE_IN_USE);
  muAssert(strcmp("Line in use", six) == 0, "Expected Line in use");

  const char *seven = phony_state_to_str(PHONY_DEVICE_NOT_FOUND);
  muAssert(strcmp("Device not found", seven) == 0, "Expected Device not found");

  return NULL;
}

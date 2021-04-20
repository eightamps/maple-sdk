#include "minunit.h"
#include "phony_test.h"
#include "dtmf_test.h"
// #include "kissfft_test.h"

char *allTests(void) {
  // Begin the test suite
  muSuiteStart();
  // This is a dev-only test, does not need to re-run unless exploring fft.
  // muRunTest(test_kissfft);

  muRunTest(test_dtmf_longer_value);
  muRunTest(test_dtmf_no_value);
  muRunTest(test_dtmf_new_state);
  muRunTest(test_phony_callable);
  return NULL;
}

RUN_TESTS(allTests);

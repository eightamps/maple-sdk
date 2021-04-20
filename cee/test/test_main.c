#include "minunit.h"
#include "phony_test.h"
#include "dtmf_test.h"

char *allTests(void) {
  // Begin the test suite
  muSuiteStart();

  muRunTest(test_dtmf_longer_value);
  muRunTest(test_dtmf_no_value);
  muRunTest(test_dtmf_new_state);
  muRunTest(test_phony_callable);
  return NULL;
}

RUN_TESTS(allTests);

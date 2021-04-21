#include "minunit.h"
#include "phony_test.h"
#include "dtmf_test.h"
#include "kissfft_test.h"
#include "stitcher_test.h"

char *allTests(void) {
  // Begin the test suite
  muSuiteStart();

  // Stitcher tests
  muRunTest(test_stitcher_new);
  muRunTest(test_stitcher_init);

  // FFT tests
  muRunTest(test_fft_empty);
  muRunTest(test_fft_ones);
  muRunTest(test_fft_sine);

  // DTMF tests
  muRunTest(test_dtmf_null_numbers);
  muRunTest(test_dtmf_empty_numbers);
  muRunTest(test_dtmf_duration_single);
  muRunTest(test_dtmf_duration_multiple);
  muRunTest(test_dtmf_sample_count);
  muRunTest(test_dtmf_sample_multiple);
  muRunTest(test_dtmf_large_sample_rate);
  muRunTest(test_dtmf_large_sample_rate_multiple);
  muRunTest(test_dtmf_large_sample_rate_three);
  muRunTest(test_dtmf_sample_index);
  muRunTest(test_dtmf_entry_and_padding);

  // Phony tests
  muRunTest(test_phony_callable);

  return NULL;
}

RUN_TESTS(allTests);

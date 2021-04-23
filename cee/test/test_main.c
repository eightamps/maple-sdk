#include "dtmf_test.h"
#include "kissfft_test.h"
#include "minunit.h"
#include "phone_view_test.h"
#include "phony_hid_test.h"
#include "phony_test.h"
#include "stitcher_test.h"

char *allTests(void) {
  // Begin the test suite
  muSuiteStart();

  // Stitcher tests
  muRunTest(test_stitcher_new);
  // muRunTest(test_stitcher_init);

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
  // muRunTest(test_phony_callable);
  // muRunTest(test_phony_report);

  // PhoneView tests
  // TODO(lbayes): Running the following test throws errors in console, need
  //  to investigate instantiating GTK widgets without a window context.
  // muRunTest(test_phone_view_new);

  // PhonyHid tests
  muRunTest(test_phony_hid_state);
  muRunTest(test_phony_hid_new);
  muRunTest(test_phony_hid_open);

  return NULL;
}

RUN_TESTS(allTests);

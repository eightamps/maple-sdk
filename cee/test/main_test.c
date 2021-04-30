#include "dtmf_test.h"
#include "hid_client_test.h"
#include "kissfft_test.h"
#include "minunit.h"
#include "phony_hid_test.h"
#include "phony_test.h"
#include "phony_view_test.h"
#include "stitch_test.h"

char *allTests(void) {

  // Begin the test suite
  muSuiteStart();

  // HID Client tests
  muRunTest(test_hid_client_new);
  muRunTest(test_hid_client_set_vid);
  muRunTest(test_hid_client_set_pid);
  muRunTest(test_hid_client_open);

  // DTMF tests
  muRunTest(test_dtmf_duration_multiple);
  muRunTest(test_dtmf_duration_single);
  muRunTest(test_dtmf_next_sample_without_dial);
  muRunTest(test_dtmf_null_numbers);
  muRunTest(test_dtmf_empty_numbers);
  muRunTest(test_dtmf_sample_count);
  muRunTest(test_dtmf_sample_multiple);
  muRunTest(test_dtmf_large_sample_rate);
  muRunTest(test_dtmf_large_sample_rate_multiple);
  muRunTest(test_dtmf_large_sample_rate_three);
  muRunTest(test_dtmf_sample_index);
  muRunTest(test_dtmf_entry_and_padding);
  muRunTest(test_dtmf_double_dial);

  // PhonyHid tests
  // muRunTest(test_phony_hid_open); // TODO(lbayes): mock the service
  muRunTest(test_phony_hid_new);
  muRunTest(test_hid_in_report_to_struct);

  // Phony tests
  muRunTest(test_phony_state);

  // Stitch tests
  muRunTest(test_stitch_new);
  muRunTest(test_stitch_init_null);
  muRunTest(test_stitch_init);

  // FFT tests
  muRunTest(test_fft_empty);
  muRunTest(test_fft_ones);
  muRunTest(test_fft_sine);

  // PhoneView tests
  // TODO(lbayes): Running the following test throws errors in console, need
  //  to investigate instantiating GTK widgets without a window context.
  // muRunTest(test_phone_view_new);

  return NULL;
}

RUN_TESTS(allTests);

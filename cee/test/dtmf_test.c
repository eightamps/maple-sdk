//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "dtmf_test.h"
#include "kiss_fft.h"

#include <minunit.h>
#include <stdio.h>
#include <string.h>

/**
 * Super basic, super brittle float comparison for testing.
 * @param a float
 * @param b float
 * @return 0 for equals and 1 for not equals
 */
int float_compare(float af, float bf) {
  int ai = (int)(af * 1000.0f);
  int bi = (int)(bf * 1000.0f);
  // printf("af: %f == bf: %f\n", af, bf);
  if (ai != bi) {
    printf("float_compare failed with ai: %d == bi: %d\n", ai, bi);
    return 1;
  }
  return 0;
}

static const float expected_samples_one[25] = {
  0.000000f,
  0.174223f,
  0.268352f,
  0.228147f,
  0.042991f,
  -0.250005f,
  -0.576754f,
  -0.848776f,
  -0.990155f,
  -0.960954f,
  -0.769449f,
  -0.469593f,
  -0.144423f,
  0.119412f,
  0.258141f,
  0.249909f,
  0.121400f,
  -0.062317f,
  -0.218109,
  -0.271456f,
  -0.181674f,
  0.045767f,
  0.359528f,
  0.677828f,
  0.913353f
};

char *test_dtmf_new_state(void) {
  DtmfContext *c = dtmf_new("1", 100);
  int count = c->samples_count;
  muAssert(c->sample_rate == 100, "Expected sample_rate");
  muAssert(c->samples_index == 0, "Expected samples_index");
  muAssert(count == 25, "Expected samples_count at 25");
  muAssert(strcmp(c->values, "1") == 0, "Expected values");

  kiss_fft_cfg fft = kiss_fft_alloc(1024, 0, NULL, NULL);
  muAssert(fft != NULL, "Expected FFT configuration object");
  kiss_fft_cpx fft_in[1024];
  kiss_fft_cpx fft_out[1024];

  for (int i = 0; i < c->samples_count; i++) {
    fft_in[i].r = c->samples[i];
    fft_in[i].i = c->samples[i];
  }

  kiss_fft(fft, fft_in, fft_out);

  printf("FINISHED FFT with: %f\n", fft_out[1]);

  /*
  for (int i = 0; i < count; i++) {
    // printf("sample %d a: %f vs b: %f\n", i, c->samples[i],
           // expected_samples_one[i]);
    muAssert(float_compare(c->samples[i], expected_samples_one[i]) == 0,
        "Expected sample");
  }
   */

  kiss_fft_free(fft);
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_no_value(void) {
  DtmfContext *c = dtmf_new(NULL, 100);
  muAssert(c == NULL, "Expected null result");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_longer_value(void) {
  DtmfContext *c = dtmf_new("5104590393", 100);

  muAssert(c != NULL, "Expected non-null result");
  muAssert(c->samples_count == 430, "Expected samples_count");
  // TODO(lbayes): Multiple characters aren't performing as expected.
  //  Uncomment the following to see an odd display of values
  // for (int i = 0; i < c->samples_count; i++) {
    // printf("YYY: %d %f\n", i, c->samples[i]);
  // }
  // muAssert(c->samples[400], 0.500f, "Expected sample entry");
  dtmf_free(c);
  return NULL;
}

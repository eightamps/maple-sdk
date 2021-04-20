//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "dtmf_test.h"
#include <minunit.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

/**
 * Super basic, super brittle float comparison for testing.
 * @param a float
 * @param b float
 * @return 0 for equals and 1 for not equals
 */
int float_compare(float af, float bf) {
  int ai = (int)(af * 100000.0f);
  int bi = (int)(bf * 100000.0f);
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
  -0.249991f,
  -0.576754f,
  -0.848776f,
  -0.990155f,
  -0.960954f,
  -0.769449f,
  -0.469593f,
  -0.144423f,
  0.119412f,
  0.258141f,
  0.250017f,
  0.121400f,
  -0.062317f,
  -0.217975f,
  -0.271456f,
  -0.181674f,
  0.045767f,
  0.359528f,
  0.677828f,
  0.913353f
};

char *test_dtmf_new_state(void) {
  DtmfContext *c = dtmf_new("1", 100);
  muAssert(c->sample_rate == 100, "Expected sample_rate");
  muAssert(c->samples_index == 0, "Expected samples_index");
  muAssert(c->samples_count == 25, "Expected samples_count at .25");
  muAssert(strcmp(c->values, "1") == 0, "Expected values");
  for (int i = 0; i < c->samples_count; i++) {
    muAssert(float_compare(c->samples[i], expected_samples_one[i]) == 0,
        "Expected sample");
  }
  return NULL;
}

//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include <math.h>

void dtmf_generate(int tone1, int tone2, int rate, float *out) {
  float increment = (2.0f * M_PI) / ((float)rate / tone1);
  float sampleValue = 0;
  int i;

  // Add first tone at 1/2 volume
  for (i = 0; i < rate; i++) {
    out[i] = sinf(sampleValue) * 0.5;
    sampleValue += increment;
  }

  // Add second tone at 1/2 volume
  increment = (2.0f * M_PI) / ((float)rate / tone2);
  sampleValue = 0;
  for (i = 0; i < rate; i++) {
    out[i] += sinf(sampleValue) * 0.5;
    sampleValue += increment;
  }
}

//
// Created by lukebayes on 4/20/21.
//

#include "minunit.h"
#include "kissfft_test.h"
#include <kiss_fft.h>
#include <math.h>

#define TWO_PI (2.0f * (float)M_PI)
#define N 16
#define NF (N / 2 - 1)

static char *exec_fft(const kiss_fft_cpx in[N],
                     kiss_fft_cpx out[NF]) {
  kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, NULL, NULL);
  muAssert(cfg != NULL, "Expected malloc");
  kiss_fft(cfg, in, out);
  free(cfg);
}

char *test_fft_empty(void) {
  kiss_fft_cpx in[N];
  kiss_fft_cpx out[NF];

  for (int i = 0; i < N; i++) {
    in[i].r = 0.0f;
    in[i].i = 0.0f;
  }

  exec_fft(in, out);
  muAssert(out[0].r == 0.0f, "Expected zero");
  muAssert(out[NF].r == 0.0f, "Expected zero");

  return NULL;
}

char *test_fft_ones(void) {
  kiss_fft_cpx in[N];
  kiss_fft_cpx out[NF];

  for (int i = 0; i < N; i++) {
    in[i].r = 1.0f;
    in[i].i = 0.0f;
  }
  exec_fft(in, out);

  int abs = sqrt((out[0].r * out[0].r) + (out[0].i * out[0].i));
  muAssert(abs == 15, "Expected sum for first entry");
  muAssert(0 == (int)out[1].r, "Expected zeros");
  muAssert(0 == (int)out[NF].r, "Expected zeros");
  return NULL;
}

char *test_fft_sine(void) {
  kiss_fft_cpx in[N];
  kiss_fft_cpx out[NF];

  // float freq = 0.08f;
  for (int i = 0; i < N; i++) {
    // in[i] = sin(TWO_PI * freq * (i / N*1.00));
    in[i].r = sin(TWO_PI * 4 * i / N);
    in[i].i = 0.0f;
  }
  exec_fft(in, out);

  int abs = sqrt((out[4].r * out[4].r) + (out[4].i * out[4].i));
  muAssert(abs == 6, "Expected 6 on 4");
  return NULL;
}

/*
 * Get the absolute value for an fft result.
for (int i = 0; i < NF; i++) {
  abs = sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i));
  printf("%d: abs: %d,  %04f\n", i, abs, out[i].r);
}
 */

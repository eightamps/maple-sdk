//
// Created by lukebayes on 4/20/21.
//

#include "minunit.h"
#include "kissfft_test.h"
#include <kiss_fft.h>
#include <math.h>

#define N 16

static void exec_fft(const char *title, const kiss_fft_cpx in[N],
                     kiss_fft_cpx out[N / 2 + 1]) {
  kiss_fft_cfg cfg;
  printf("%s\n", title);
  if ((cfg = kiss_fft_alloc(N, 0 /* is_inverse_fft */, NULL, NULL)) != NULL) {
    kiss_fft(cfg, in, out);
    free(cfg);
    for (int i = 0; i < N; i++) {
      printf(" in[%02d] r=%+f,  out[%02d] r=%+f,  i=%+f M[%02d]=%+f\n",
             i, in[i].r, i, out[i].r, out[i].i, i,
             sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i)));
    }
  } else {
    printf("Not enough memory");
    exit(-1);
  }
}

/**
 * Verify the kiff_fft shared library and ensure it is being integrated
 * properly.
 * @return NULL
 */
char *test_kissfft(void) {
  kiss_fft_cpx in[N];
  kiss_fft_cpx out[N / 2 + 1];
  int i;

  for (i = 0; i < N; i++) {
    in[i].r = 0.0f;
    in[i].i = 0.0f;
  }
  exec_fft("Zeroes (real)", in, out);

  for (i = 0; i < N; i++) {
    in[i].r = 1.0f;
    in[i].i = 0.0f;
  }
  exec_fft("Ones (real)", in, out);

  // float freq = 0.08f;
  for (i = 0; i < N; i++) {
    // in[i] = sin(2 * M_PI * freq * (i / N*1.00));
    in[i].r = sin(2 * M_PI * 4 * i / N);
    in[i].i = 0.0f;
  }
  exec_fft("SineWave (real)", in, out);

  return NULL;
}

//
// Created by lukebayes on 4/21/21.
//
#include "minunit.h"
#include "stitcher.h"
#include "stitcher_test.h"
#include <stdbool.h>

char *test_stitcher_new(void) {
  StitcherContext *c = stitcher_new();
  muAssert(c != NULL, "Expected context object");
  muAssert(c->is_active == false, "Expected non-active");
  muAssert(c->sample_rate == 0, "Expected default sample rate");
  muAssert(c->soundio == NULL, "Expected NULL soundio");
  muAssert(c->to_speaker == NULL, "Expected NULL to_speaker");
  muAssert(c->to_speaker_stream == NULL, "Expected NULL to_speaker_stream");
  stitcher_free(c);
  return NULL;
}

char *test_stitcher_init(void) {
  // StitcherContext *c = stitcher_new();
  // stitcher_init(c);
  // muAssert(1, "SDF");
  return NULL;
}

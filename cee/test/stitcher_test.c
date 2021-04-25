//
// Created by lukebayes on 4/21/21.
//
#include "minunit.h"
#include "audio_stitcher.h"
#include "stitcher_test.h"
#include <stdbool.h>

char *test_stitcher_new(void) {
  StitcherContext *c = stitcher_new();
  muAssert(c != NULL, "Expected context object");
  muAssert(c->is_active == false, "Expected non-active");
  muAssert(c->soundio == NULL, "Expected NULL soundio");

  muAssert(c->from_phone->device == NULL, "Expected NULL from_phone->device");
  muAssert(c->to_speaker->device == NULL, "Expected NULL to_speaker->device");
  muAssert(c->from_mic->device == NULL, "Expected NULL from_mic->device");
  muAssert(c->to_phone->device == NULL, "Expected NULL to_phone->device");

  muAssert(c->from_phone->stream == NULL, "Expected NULL from_phone->stream");
  muAssert(c->to_speaker->stream == NULL, "Expected NULL to_speaker->stream");
  muAssert(c->from_mic->stream == NULL, "Expected NULL from_mic->stream");
  muAssert(c->to_phone->stream == NULL, "Expected NULL to_phone->stream");

  muAssert(c->to_speaker->name == NULL, "Expected NULL to_speaker->name");
  muAssert(c->from_mic->name == NULL, "Expected NULL from_mic->name");
  muAssert(c->to_phone->name == NULL, "Expected NULL to_phone->name");
  muAssert(c->from_phone->name == NULL, "Expected NULL from_phone->name");

  stitcher_free(c);
  return NULL;
}

char *test_stitcher_init(void) {
  StitcherContext *c = stitcher_new();
  stitcher_init(c);
  stitcher_free(c);
  return NULL;
}

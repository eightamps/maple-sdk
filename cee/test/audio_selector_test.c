//
// Created by lukebayes on 4/25/21.
//

#include "audio_selector_test.h"
#include "audio_selector.h"
#include "minunit.h"

char *test_audio_selector_new(void) {
  AudioSelectorContext *c = audio_selector_new();
  muAssert(NULL != c, "Expect context");
  return NULL;
}

//
// Created by lukebayes on 4/25/21.
//

#include "audio_selector.h"
#include "stdlib.h"
#include "stdio.h"

struct AudioSelectorContext *audio_selector_new(void) {
  AudioSelectorContext *c = calloc(sizeof(AudioSelectorContext), 1);
  if (c == NULL) {
    fprintf(stderr, "audio_selector_new failed to allocate\n");
  }

  return c;
}

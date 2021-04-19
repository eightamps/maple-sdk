//
// Created by lukebayes on 4/19/21.
//

#include "stitcher.h"
#include <stdlib.h>

StitcherContext *stitcher_new(void) {
  StitcherContext *c = malloc(sizeof(StitcherContext));
  if (c == NULL) return NULL;
}

int stitcher_connect(StitcherContext *c) {
  return EXIT_SUCCESS;
}

int stitcher_start(StitcherContext *c) {
  return EXIT_SUCCESS;
}

void stitcher_free(StitcherContext *c) {
  free(c);
}

//
// Created by lukebayes on 4/26/21.
//

#include "stitch_test.h"
#include "minunit.h"
#include "stitch.h"

char *test_stitch_new(void) {
  StitchContext *c = stitch_new();
  muAssert(c != NULL, "Expected context");
  stitch_free(c);
  return NULL;
}

char *test_stitch_init_null(void) {
  int status = stitch_init(NULL);
  muAssert(status == -EINVAL, "Expected invalid arguments status");
  return NULL;
}

char *test_stitch_init(void) {
  StitchContext *c = stitch_new();
  int status = stitch_init(c);

  muAssert(status == EXIT_SUCCESS, "Expected success init");
  stitch_free(c);
  return NULL;
}

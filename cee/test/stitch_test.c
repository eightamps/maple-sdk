//
// Created by lukebayes on 4/26/21.
//

#include "../src/stitch.h"
#include "fakes/soundio/soundio.h"
#include "minunit.h"
#include "stitch_test.h"

char *test_stitch_new(void) {
  stitch_context_t *c = stitch_new();
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
  stitch_context_t *c = stitch_new();
  int status = stitch_init(c);

  muAssert(status == EXIT_SUCCESS, "Expected success init");
  stitch_free(c);
  return NULL;
}

char *test_stitch_init_sio_create_failed(void) {
  stitch_context_t *c = stitch_new();

  soundio_fake_create_returns(NULL);
  int status = stitch_init(c);
  muAssert(status == -ENOMEM, "Expected failure");
  muAssert(c->is_initialized == false, "Should not initialize");

  stitch_free(c);
  return NULL;
}

char *test_stitch_init_sio_connect_failed(void) {
  stitch_context_t *c = stitch_new();

  soundio_fake_connect_returns(-128);
  int status = stitch_init(c);

  muAssert(status == -128, "Expected connect failure");
  muAssert(c->is_initialized, "Expected initialization");
  muAssert(c->platform != NULL, "Expected context");

  stitch_free(c);
  return NULL;
}

//
// Created by lukebayes on 6/25/2021
//

#include "soundio_fake_test.h"
#include "minunit.h"
#include <stdbool.h>
#include <soundio/soundio.h>

char *test_soundio_fake_create(void) {
  struct SoundIo *sio = soundio_create();
  muAssert(sio != NULL, "Expected context");

  soundio_destroy(sio);
  return NULL;
}

char *test_soundio_fake_create_failure(void) {
  soundio_fake_create_returns(NULL);
  struct SoundIo *sio = soundio_create();
  muAssert(sio == NULL, "Expected null result");

  sio = soundio_create();
  muAssert(sio != NULL, "Only fakes one call");
  soundio_destroy(sio);
  return NULL;
}

char *test_soundio_fake_connect(void) {
  int status = 0;

  // Fake impl doesn't require a valid context
  status = soundio_connect(NULL);
  muAssert(0 == status, "Expected success");

  soundio_fake_connect_returns(-1);
  status = soundio_connect(NULL);
  muAssert(-1 == status, "Expected -1 failure");

  // Expected status to be reset after result
  status = soundio_connect(NULL);
  muAssert(0 == status, "Expected success");
  return NULL;
}

char *test_soundio_fake_connect_backend(void) {
  int status = 0;

  // Fake impl doesn't require a valid context
  status = soundio_connect_backend(NULL, SoundIoBackendNone);
  muAssert(0 == status, "Expected success");

  soundio_fake_connect_returns(-1);
  status = soundio_connect_backend(NULL, SoundIoBackendNone);
  muAssert(-1 == status, "Expected -1 failure");

  // Expected status to be reset after result
  status = soundio_connect_backend(NULL, SoundIoBackendNone);
  muAssert(0 == status, "Expected success");
}

char *test_soundio_fake_destroy(void) {
  soundio_destroy(NULL);
  return NULL;
}

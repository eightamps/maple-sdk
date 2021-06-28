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
  return NULL;
}

char *test_soundio_fake_destroy(void) {
  soundio_destroy(NULL);
  return NULL;
}

char *test_default_fake_input_devices(void) {
  struct SoundIo *sio = soundio_create();

  // Create a set of fake devices
  add_input_devices(
    "abcd",
    "efgh",
    "ijkl"
  );

  // Get the current state of the context
  int count = soundio_input_device_count(sio);
  muAssertIntEq(count, 3, "Expected Devices");

  struct SoundIoDevice *one = soundio_get_input_device(sio, 0);
  muAssertStrCmp(one->id, "abcd-id", "Expected id");
  muAssertStrCmp(one->name, "abcd-name", "Expected name");

  struct SoundIoDevice *two = soundio_get_input_device(sio, 1);
  muAssertStrCmp(two->id, "efgh-id", "Expected id");
  muAssertStrCmp(two->name, "efgh-name", "Expected name");

  struct SoundIoDevice *three = soundio_get_input_device(sio, 2);
  muAssertStrCmp(three->id, "ijkl-id", "Expected id");
  muAssertStrCmp(three->name, "ijkl-name", "Expected name");

  soundio_destroy(sio);
  return NULL;
}

char *test_default_fake_output_devices(void) {
  struct SoundIo *sio = soundio_create();

  // Create a set of fake devices
  add_output_devices(
    "abcd",
    "efgh",
    "ijkl"
  );

  // Get the current state of the context
  int count = soundio_output_device_count(sio);
  muAssertIntEq(count, 3, "Expected Devices");

  struct SoundIoDevice *one = soundio_get_output_device(sio, 0);
  muAssertStrCmp(one->id, "abcd-id", "Expected id");
  muAssertStrCmp(one->name, "abcd-name", "Expected name");

  struct SoundIoDevice *two = soundio_get_output_device(sio, 1);
  muAssertStrCmp(two->id, "efgh-id", "Expected id");
  muAssertStrCmp(two->name, "efgh-name", "Expected name");

  struct SoundIoDevice *three = soundio_get_output_device(sio, 2);
  muAssertStrCmp(three->id, "ijkl-id", "Expected id");
  muAssertStrCmp(three->name, "ijkl-name", "Expected name");

  soundio_destroy(sio);
  return NULL;
}

#include "../src/stitch.h"
#include "../src/nix/stitch_soundio.h"
#include "fakes/soundio/soundio.h"
#include "minunit.h"
#include "stitch_test.h"

char *test_stitch_new(void) {
  stitch_context_t *c = stitch_new_with_label("abcd");
  muAssert(c != NULL, "Expected context");
  muAssertStrCmp(c->label, "abcd", "Expected label");
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

char *test_stitch_connect(void) {
  stitch_context_t *c = stitch_new();
  // Do not configure a custom backend on the c->platform object.
  int status = stitch_init(c);
  muAssertIntEq(status, 0, "Expected success");
  int last_function = soundio_fake_get_last_function_name();
  muAssertIntEq(last_function, FunctionNameConnect, "Expected connect (no backend)");

  stitch_free(c);
  return NULL;
}

char *test_stitch_init_custom_backend(void) {
  stitch_context_t *c = stitch_new();
  // Configure a custom backend on the c->platform object.
  soundio_platform_t *p = (soundio_platform_t*)c->platform;
  p->backend = SoundIoBackendAlsa;

  int status = stitch_init(c);
  muAssertIntEq(status, 0, "Expected success");
  int last_function = soundio_fake_get_last_function_name();
  muAssertIntEq(last_function, FunctionNameConnectBackend, "Expected connect_backend");

  stitch_free(c);
  return NULL;
}

char *test_get_default_input(void) {
  struct SoundIo *sio = soundio_create();

  // Create a set of fake devices
  set_fake_input_devices(
    sio,
    "Aukey Microphone",
    "Built-in Array Mic"
  );

  // Get the current state of the context
  int index = soundio_default_input_device_index(sio);
  muAssertIntEq(index, 0, "Expected zero");

  struct SoundIoDevice *device = soundio_get_input_device(sio, index);
  muAssert(device != NULL, "Expected device");

  soundio_destroy(sio);
  return NULL;
}


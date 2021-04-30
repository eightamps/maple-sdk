//
// Created by lukebayes on 4/30/21.
//

#include "hid_client.h"
#include "hid_client_test.h"
#include "minunit.h"
#include "errno.h"

#define EIGHT_AMPS_VID 0x335e
#define MAPLE_PID 0x8a01

char *test_hid_client_new(void) {
  hid_client_context_t *c = hid_client_new();
  muAssert(c != NULL, "Expected context");

  hid_client_free(c);
  return NULL;
}

char *test_hid_client_set_vid(void) {
  int status = hid_client_set_vid(NULL, EIGHT_AMPS_VID);
  muAssert(status == -EINVAL, "Expected Invalid argument");

  hid_client_context_t *c = hid_client_new();
  int vid = hid_client_get_vid(c);
  muAssert(vid == 0x0, "Expected default vid");

  status = hid_client_set_vid(c, EIGHT_AMPS_VID);
  muAssert(status == EXIT_SUCCESS, "Expected success response");

  vid = hid_client_get_vid(c);
  muAssert(vid == EIGHT_AMPS_VID, "Expected configured vid");

  hid_client_free(c);
  return NULL;
}

char *test_hid_client_set_pid(void) {
  int status = hid_client_set_pid(NULL, MAPLE_PID);
  muAssert(status == -EINVAL, "Expected Invalid argument");

  hid_client_context_t *c = hid_client_new();
  int pid = hid_client_get_pid(c);
  muAssert(pid == 0x0, "Expected default pid");

  status = hid_client_set_pid(c, MAPLE_PID);
  muAssert(status == EXIT_SUCCESS, "Expected success response");

  pid = hid_client_get_pid(c);
  muAssert(pid == MAPLE_PID, "Expected configured pid");

  hid_client_free(c);
  return NULL;
}

char *test_hid_client_open(void) {
  int status = hid_client_open(NULL);
  muAssert(status == -EINVAL, "Expected Invalid argument");

  hid_client_context_t *c = hid_client_new();
  status = hid_client_open(c);
  muAssert(status == EXIT_SUCCESS, "Expected success");

  hid_client_free(c);
  return NULL;
}

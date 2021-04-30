//
// Created by lukebayes on 4/30/21.
//

#include "hid_client.h"
#include "hid_client_test.h"
#include "minunit.h"

char *test_hid_client_new(void) {
  HidClientContext *c = hid_client_new();
  muAssert(c != NULL, "Expected context");
  return NULL;
}

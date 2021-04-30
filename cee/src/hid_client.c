//
// Created by lukebayes on 4/30/21.
//

#include "log.h"
#include "hid_client.h"
#include <stdlib.h>

struct HidClientContext *hid_client_new(void) {
  HidClientContext *c = calloc(sizeof(HidClientContext), 1);
  if (c == NULL) {
    log_err("hid_client_new failed to allocated");
    return NULL;
  }

  return c;
}

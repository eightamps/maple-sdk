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

int hid_client_open(struct HidClientContext *c) {
  if (c == NULL) {
    log_err("hid_client_open requires valid context");
    return -EINVAL;
  }

  return EXIT_SUCCESS;
}

int hid_client_set_vid(struct HidClientContext *c, int vid) {
  if (c == NULL) {
    log_err("hid_client_set_vid requires valid context");
    return -EINVAL;
  }
  c->vid = vid;
  return EXIT_SUCCESS;
}

int hid_client_get_vid(struct HidClientContext *c) {
  if (c == NULL) {
    log_err("hid_client_get_vid requires valid context");
    return -EINVAL;
  }
  return c->vid;
}

int hid_client_set_pid(struct HidClientContext *c, int pid) {
  if (c == NULL) {
    log_err("hid_client_set_pid requires valid context");
    return -EINVAL;
  }
  c->pid = pid;
  return EXIT_SUCCESS;
}

int hid_client_get_pid(struct HidClientContext *c) {
  if (c == NULL) {
    log_err("hid_client_get_pid requires valid context");
    return -EINVAL;
  }
  return c->pid;
}

void hid_client_free(struct HidClientContext *c) {
  if (c != NULL) {
    free(c);
  }

}

//
// Created by lukebayes on 4/25/21.
//
#include "log.h"
#include "stitch.h"
#include <stdlib.h>
#include <stdio.h>

DLL_LINK stitch_context_t *stitch_new_with_label(char *label) {
  log_info("stitch_new_with_label called with: %s", label);
  stitch_context_t *c = stitch_new();
  if (c != NULL) {
    c->label = label;
  }
  return c;
}

DLL_LINK stitch_context_t *stitch_new(void) {
  printf("CORRECT STICH_NEW\n");
  stitch_context_t *c = calloc(sizeof(stitch_context_t), 1);
  if (c == NULL) {
    log_err("stitch_new failed to allocate memory");
    return NULL;
  }
  return c;
}

DLL_LINK int stitch_init(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_set_dtmf(stitch_context_t *c, dtmf_context_t *d) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_default_input_index(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_default_output_index(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_matching_input_device_index(stitch_context_t *c, char *name) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_matching_output_device_index(stitch_context_t *c, char *name) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_start(stitch_context_t *c, int in_index, int out_index) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_stop(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_join(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK void stitch_free(stitch_context_t *c) {
}

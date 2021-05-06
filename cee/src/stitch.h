//
// Created by lukebayes on 4/25/21.
//

#ifndef MAPLE_STITCH_H
#define MAPLE_STITCH_H

#include "dtmf.h"
#include <stdlib.h>
#include <stdbool.h>

#define STITCH_ASI_TELEPHONE "ASI Telephone"
#define STITCH_ASI_MICROPHONE "ASI Microphone"
#define STITCH_WAY_2_CALL "Way2Call"
#define STITCH_WAY_2_CALL_LOWER "way2call"

typedef struct {
  char *label;
  dtmf_context_t *dtmf_context;
  pthread_t thread_id;
  int thread_exit_status;
  float input_latency;
  int in_device_index;
  int out_device_index;
  bool is_initialized;
  bool is_active;
  // bool in_raw;
  // bool out_raw;
  // platform specific audio context
  void *platform;
}stitch_context_t;

stitch_context_t *stitch_new_with_label(char *label);
stitch_context_t *stitch_new(void);
int stitch_init(stitch_context_t *c);

int stitch_set_dtmf(stitch_context_t *c, dtmf_context_t *d);
int stitch_get_default_input_index(stitch_context_t *c);
int stitch_get_default_output_index(stitch_context_t *c);
int stitch_get_matching_input_device_index(stitch_context_t *c, char *name);
int stitch_get_matching_output_device_index(stitch_context_t *c, char *name);

int stitch_start(stitch_context_t *c, int in_index, int out_index);
int stitch_stop(stitch_context_t *c);
int stitch_join(stitch_context_t *c);
void stitch_free(stitch_context_t *c);

#endif //MAPLE_STITCH_H

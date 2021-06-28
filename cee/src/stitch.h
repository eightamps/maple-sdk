#ifndef MAPLE_STITCH_H
#define MAPLE_STITCH_H

#include "dtmf.h"
#include "shared.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

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

DLL_LINK stitch_context_t *stitch_new_with_label(char *label);
DLL_LINK stitch_context_t *stitch_new(void);
DLL_LINK int stitch_init(stitch_context_t *c);

DLL_LINK int stitch_set_dtmf(stitch_context_t *c, dtmf_context_t *d);
DLL_LINK int stitch_get_default_input_index(stitch_context_t *c);
DLL_LINK int stitch_get_default_output_index(stitch_context_t *c);
DLL_LINK int stitch_get_matching_input_device_index(stitch_context_t *c, char *name);
DLL_LINK int stitch_get_matching_output_device_index(stitch_context_t *c, char *name);

DLL_LINK int stitch_start(stitch_context_t *c, int in_index, int out_index);
DLL_LINK int stitch_stop(stitch_context_t *c);
DLL_LINK int stitch_join(stitch_context_t *c);
DLL_LINK void stitch_free(stitch_context_t *c);

#endif // MAPLE_STITCH_H

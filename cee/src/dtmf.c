//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "log.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MS_PER_ENTRY 250
#define MS_PER_SPACE 100
#define TWO_PI ((float)2 * (float)M_PI)

typedef struct DtmfToneInfo {
  const int char_code;
  int tone1;
  int tone2;
}DtmfToneInfo;

static DtmfToneInfo DtmfTones[] = {
    {'0', 1336, 941}, // underscore-ish?
    {'1', 1209, 697}, // recording icon?
    {'2', 1336, 697}, // abcd
    {'3', 1477, 697}, // def
    {'4', 1209, 770}, // ghi
    {'5', 1336, 770}, // jkl
    {'6', 1477, 770}, // mno
    {'7', 1209, 852}, // pqrs
    {'8', 1336, 852}, // tuv
    {'9', 1477, 852}, // wxyz
    {'*', 1209, 941}, // plus sign?
    {'#', 1477, 941}, // up arrow?
    {'A', 1633, 697},
    {'B', 1633, 770},
    {'C', 1633, 852},
    {'D', 1633, 941},
};

static const int dtmf_tones_count = sizeof(DtmfTones) / sizeof(DtmfToneInfo);

static DtmfToneInfo *get_tone_info(int char_code) {
  for (int i = 0; i < dtmf_tones_count; i++) {
    DtmfToneInfo *tone_info = &DtmfTones[i];
    if (char_code == tone_info->char_code) {
      return tone_info;
    }
  }
  return NULL;
}

int dtmf_dial(DtmfContext *c, const char *values, int sample_rate) {
  if (values == NULL || strlen(values) < 1) {
    log_err("dtmf_new values must not be empty");
    return -EINVAL;
  }

  int values_count = (int)strlen(values);
  c->sample_rate = sample_rate;
  c->entries = malloc(values_count + 1);
  strcpy(c->entries, values);
  c->is_active = true;

  return EXIT_SUCCESS;
}

struct DtmfContext *dtmf_new() {
  DtmfContext *c = calloc(sizeof(DtmfContext), 1);
  if (c == NULL) {
    fprintf(stderr, "dtmf_new cannot allocate\n");
    return NULL;
  }
  c->entry_ms = MS_PER_ENTRY;
  c->padding_ms = MS_PER_SPACE;

  return c;
}

static void configure_dtmf(DtmfContext *c) {
  int values_count = (int)strlen(c->entries);
  c->duration_ms = values_count * c->entry_ms +
      (values_count - 1) * c->padding_ms;

  float sample_rate = (float)c->sample_rate;
  float duration_ms = (float)c->duration_ms;

  c->sample_count = (int)(sample_rate * (duration_ms * 0.001f));
  c->entry_sample_count = (int)(sample_rate * ((float)c->entry_ms * 0.001f));
  c->padding_sample_count = (int)(sample_rate *
      ((float)c->padding_ms * 0.001f));
}

static float get_sample_for(float freq_1, float freq_2, float sample_rate,
    float sample_index) {
  // Apply the first tone
  float incr1 = TWO_PI / (sample_rate / freq_1);
  float sample = (sinf(incr1 * sample_index) * 0.5f);

  // Apply the second tone
  float incr2 = TWO_PI / (sample_rate / freq_2);
  sample += (sinf(incr2 * sample_index) * 0.5f);

  return sample;
}

int dtmf_next_sample(DtmfContext *c, float **sample) {
  if (!c->is_active) {
    log_err("dtmf_next_sample must follow dtmf_dial call");
    return -EINVAL;
  }

  if (c->duration_ms == 0) {
    configure_dtmf(c);
  }

  // We've exceeded our window, return empty samples.
  // TODO(lbayes): Should return something else to end signal.
  if (c->sample_index >= c->sample_count) {
    c->is_complete = true;
    **sample = 0.0f;
    return 0;
  }

  int entry_index = c->sample_index / (c->entry_sample_count +
                                       c->padding_sample_count);
  int entry_location = c->sample_index % (c->entry_sample_count +
                                          c->padding_sample_count);

  // We're inside of a padding block
  if (entry_location > c->entry_sample_count) {
    c->sample_index++;
    **sample = 0.0f;
    return 0;
  }

  // We're inside of an entry, get the DTMF signal sample
  int entry = (int)c->entries[entry_index];
  DtmfToneInfo *tones = get_tone_info(entry);
  float result = get_sample_for(
      (float)tones->tone1,
      (float)tones->tone2,
      (float)c->sample_rate,
      (float)c->sample_index);
  *sample = &result;
  // printf("RESULT: %.6f\n", result);
  // printf("SAMPLE: %.6f\n", **sample);
  c->sample_index++;
  return EXIT_SUCCESS;
}

void dtmf_free(DtmfContext *c) {
  if (c != NULL) {
    if (c->entries != NULL) {
      free(c->entries);
    }
    free(c);
  }
}

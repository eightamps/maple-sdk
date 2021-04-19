//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const float DTMF_ENTRY_MS = 250.0f;
static const float DTMF_SILENCE_MS = 200.0f;

typedef struct DtmfToneInfo {
  const int char_code;
  int tone1;
  int tone2;
}DtmfToneInfo;

static DtmfToneInfo DtmfTones[] = {
    {'0', 1336, 941},
    {'1', 1209, 697},
    {'2', 1336, 697},
    {'3', 1477, 697},
    {'4', 1209, 770},
    {'5', 1336, 770},
    {'6', 1477, 770},
    {'7', 1209, 852},
    {'8', 1336, 852},
    {'9', 1477, 852},
    {'A', 1633, 697},
    {'B', 1633, 770},
    {'C', 1633, 852},
    {'D', 1633, 941},
    {'*', 1209, 941},
    {'#', 1477, 941}
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

DtmfContext *dtmf_new(char *values, int sample_rate) {
  int value_count = strlen(values);
  float seconds_of_sound = (float)(DTMF_ENTRY_MS * value_count) / 1000;
  float seconds_of_silence = (float)(DTMF_SILENCE_MS * (value_count - 1)) /
                                  1000;
  size_t struct_size = (sizeof(int) * 5); // 3x int fields + 2x pointers
  DtmfContext *context = malloc(struct_size);
  context->sample_rate = sample_rate;
  context->samples_index = 0;

  size_t values_size = sizeof(char) * (strlen(values) + 1);
  context->values = malloc(values_size);
  if (context->values == NULL) {
    fprintf(stderr, "ERROR: DTMF failed to allocate values\n");
    return NULL;
  }
  // Copy provided values into context container.
  strcpy(context->values, values);

  size_t samples_count = sample_rate * (seconds_of_sound + seconds_of_silence);
  size_t samples_memory_size = sizeof(float) * samples_count;
  context->samples = malloc(samples_memory_size);
  context->samples_count = samples_count;
  if (context->samples == NULL) {
    fprintf(stderr, "ERROR: DTMF failed to allocate samples\n");
    return NULL;
  }

  for (int i = 0; i < value_count; i++) {
    int char_code = values[i];

    DtmfToneInfo *tone_info = get_tone_info(char_code);
    if (tone_info == NULL) {
      fprintf(stderr, "ERROR: Cannot get tone_info info for %c", char_code);
      // TODO(lbayes): Return an error code instead!
      return NULL;
    }

    float increment = (2.0f * M_PI) / ((float)sample_rate / tone_info->tone1);
    float sample_value = 0;
    int k;

    // Add first tone_info at 1/2 volume
    for (k = 0; k < sample_rate; k++) {
      context->samples[k] = sinf(sample_value) * 0.5;
      sample_value += increment;
    }

    // Add second tone_info at 1/2 volume
    increment = (2.0f * M_PI) / ((float)sample_rate / tone_info->tone2);
    sample_value = 0;
    for (k = 0; k < sample_rate; k++) {
      context->samples[k] += sinf(sample_value) * 0.5;
      sample_value += increment;
    }
  }

  return context;
}

void dtmf_free(DtmfContext *context) {
  free(context->values);
  free(context->samples);
  free(context);
}

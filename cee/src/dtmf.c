//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "log.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// #define sinf(x) (float)sin((double)(x))

static const float TWO_PI = M_PI * 2.0f;

typedef struct {
  const int char_code;
  int tone1;
  int tone2;
}dtmf_tone_info_t;

static dtmf_tone_info_t DtmfTones[] = {
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

static const int dtmf_tones_count = sizeof(DtmfTones) / sizeof(dtmf_tone_info_t);

static dtmf_tone_info_t *get_tone_info(int char_code) {
  for (int i = 0; i < dtmf_tones_count; i++) {
    dtmf_tone_info_t *tone_info = &DtmfTones[i];
    if (char_code == tone_info->char_code) {
      return tone_info;
    }
  }
  return NULL;
}

int dtmf_dial(dtmf_context_t *c, const char *values) {
  if (values == NULL || strlen(values) < 1) {
    log_err("dtmf_new values must not be empty");
    return -EINVAL;
  }

  char *existing = (c->entries != NULL) ? c->entries : "";

  int count = (int)strlen(values);
  count += (int)strlen(existing);

  char *new_entries = calloc(count + 1, 1);
  strcat(new_entries, existing);
  strcat(new_entries, values);

  if (c->entries != NULL) {
    free(c->entries);
  }
  c->entries = new_entries;
  c->is_active = true;
  c->duration_ms = 0; // Reset duration so we reconfigure tone timing

  return EXIT_SUCCESS;
}

int dtmf_set_sample_rate(dtmf_context_t *c, int sample_rate) {
  c->sample_rate = sample_rate;
  log_info("dtmf_set_sample_rate with: %d", c->sample_rate);
  return EXIT_SUCCESS;
}

dtmf_context_t *dtmf_new() {
  dtmf_context_t *c = calloc(sizeof(dtmf_context_t), 1);
  if (c == NULL) {
    log_err("dtmf_new cannot allocate");
    return NULL;
  }
  c->sample_rate = DTMF_DEFAULT_SAMPLE_RATE;
  c->entry_ms = DTMF_MS_PER_ENTRY;
  c->padding_ms = DTMF_MS_PER_SPACE;

  return c;
}

static void configure_dtmf(dtmf_context_t *c) {
  int values_count = (int)strlen(c->entries);
  float sample_rate = (float)c->sample_rate;
  float duration_ms = (float)((values_count * c->entry_ms) +
      ((values_count - 1) * c->padding_ms));


  c->duration_ms = (int)duration_ms;
  c->sample_count = (int)(sample_rate * (duration_ms * 0.001f));
  c->entry_sample_count = (int)(sample_rate * ((float)c->entry_ms * 0.001f));
  c->padding_sample_count = (int)(sample_rate *
      ((float)c->padding_ms * 0.001f));
}

static float get_sample_for(float freq_1, float freq_2, float sample_rate,
    float sample_index) {
  // Apply the first tone
  sample_rate = sample_rate * DTMF_SAMPLE_RATE_MULTIPLIER;
  float incr1 = TWO_PI / (sample_rate / freq_1);
  float sample = (sinf(incr1 * sample_index) * 0.7f);

  // Apply the second tone
  float incr2 = TWO_PI / (sample_rate / freq_2);
  sample += (sinf(incr2 * sample_index) * 0.7f);

  return sample;
}

int dtmf_next_sample(dtmf_context_t *c, float *sample) {
  if (!c->is_active) {
    log_err("dtmf_next_sample must follow dtmf_dial call");
    return -EINVAL;
  }

  if (c->entries == NULL) {
    log_err("Cannot configure DTMF unless dtmf_dial has been called first.");
    return -EINVAL;
  }

  if (c->duration_ms == 0) {
    configure_dtmf(c);
  }

  // We've exceeded our window, return empty samples.
  // TODO(lbayes): Should return something else to end signal.
  if (c->sample_index >= c->sample_count) {
    c->is_active = false;
    // *sample = 0x0f;
    return 0;
  }

  int entry_index = c->sample_index / (c->entry_sample_count +
      c->padding_sample_count);
  int entry_location = c->sample_index % (c->entry_sample_count +
      c->padding_sample_count);

  // We're inside of a padding block
  if (entry_location > c->entry_sample_count) {
    c->sample_index++;
    // *sample = 0.0f;
    return 0;
  }

  // We're inside of an entry, get the DTMF signal sample
  int entry = (int)c->entries[entry_index];
  dtmf_tone_info_t *tones = get_tone_info(entry);
  float result = get_sample_for(
      (float)tones->tone1,
      (float)tones->tone2,
      (float)c->sample_rate,
      (float)c->sample_index);
  *sample = result;
  // log_info("RESULT: %.6f", result);
  // log_info("SAMPLE: %.6f", **sample);
  c->sample_index++;
  return EXIT_SUCCESS;
}

void dtmf_free(dtmf_context_t *c) {
  if (c != NULL) {
    if (c->entries != NULL) {
      free(c->entries);
    }
    free(c);
  }
}

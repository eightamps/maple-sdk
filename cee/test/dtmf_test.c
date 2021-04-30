//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "dtmf_test.h"
#include "test_helper.h"
#include <minunit.h>
#include <stdio.h>
#include <string.h>

/**
 * Helper method to get a sample count for a provided rate and num chars.
 *
 * @param sample_rate
 * @param char_count
 * @return int sample_count
 */
static int get_sample_count_for(int sample_rate, int char_count) {
  float duration_ms = (float)((char_count * DTMF_MS_PER_ENTRY) +
                       ((char_count - 1) * DTMF_MS_PER_SPACE));
  return (int)((float)sample_rate * (duration_ms * 0.001f));
}

char *test_dtmf_null_numbers(void) {
  dtmf_context_t *c = dtmf_new();
  int status = dtmf_dial(c, NULL);
  muAssert(status == -EINVAL, "Expected invalid argument");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_empty_numbers(void) {
  // dtmf_context_t *c = dtmf_new("", 1000);
  dtmf_context_t *c = dtmf_new();
  int status = dtmf_dial(c, "");
  muAssert(status == -EINVAL, "Expected invalid argument");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_next_sample_without_dial(void) {
  dtmf_context_t *c = dtmf_new();
  float sample = 0.0f;
  int status = dtmf_next_sample(c, &sample);
  muAssert(status == -EINVAL, "Expected failure status");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_duration_single(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "5");
  float sample = 0.0f;
  int status = dtmf_next_sample(c, &sample);

  int floats_match = floats_match_as_str(sample, 0.0f);
  muAssert(0 == status, "Expected success");
  muAssert(0 == floats_match, "Expected zero sample");
  muAssert(DTMF_MS_PER_ENTRY == c->duration_ms, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_duration_multiple(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "510");
  float sample = 0.0f;
  int status = dtmf_next_sample(c, &sample);
  muAssert(0 == status, "Expected status");
  muAssert(c->duration_ms == 2500, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_count(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "5");
  dtmf_set_sample_rate(c, 1000);
  float sample = 0.0f;
  dtmf_next_sample(c, &sample);
  dtmf_next_sample(c, &sample);
  dtmf_next_sample(c, &sample);
  dtmf_next_sample(c, &sample);
  dtmf_next_sample(c, &sample);
  int status = dtmf_next_sample(c, &sample);
  int matched = floats_match_as_str(-0.77f, sample);
  muAssert(0 == status, "Expected status");
  muAssert(0 == matched, "Expected match");
  muAssert(c->sample_count == DTMF_MS_PER_ENTRY, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_multiple(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "51");
  dtmf_set_sample_rate(c, 1000);
  float sample = 0.0f;
  dtmf_next_sample(c, &sample);
  int sample_count = get_sample_count_for(1000, 2);
  muAssert(c->sample_count == sample_count, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "5");
  dtmf_set_sample_rate(c, 44100);
  float sample = 0.0f;
  dtmf_next_sample(c, &sample);
  muAssert(0 == floats_match_as_str(0.0f, sample), "Expected sample");
  int sample_count = get_sample_count_for(44100, 1);
  muAssert(c->sample_count == sample_count, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_multiple(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "51");
  dtmf_set_sample_rate(c, 44100);
  float sample = 0.0f;
  dtmf_next_sample(c, &sample);
  muAssert(0 == floats_match_as_str(0.0f, sample), "Expected sample");
  int sample_count = get_sample_count_for(44100, 2);
  muAssert(c->sample_count == sample_count, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_three(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 44100);
  float sample = 0.0f;
  dtmf_next_sample(c, &sample);
  int sample_count = get_sample_count_for(44100, 3);
  muAssert(c->sample_count == sample_count, "Expected duration");

  // Move the cursor to the middle of the range.
  c->sample_index = 20000;
  dtmf_next_sample(c, &sample);
  int matched = floats_match_as_str(sample, 0.778f);
  muAssert(0 == matched, "Expected sample");

  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_index(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 1000);
  c->sample_index = 974;
  float sample = 0.0f;
  int status = dtmf_next_sample(c, &sample);
  int matched = floats_match_as_str(sample, 0.0f);
  muAssert(0 == matched, "Expected sample");
  muAssert(0 == status, "Expected status");
  muAssert(0 == floats_match_as_str(0.0f, sample), "Expected sample");
  muAssert(c->is_active == true, "Expected active");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_entry_and_padding(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 2000);
  c->sample_index = 100;
  float sample = 0.0f;
  dtmf_next_sample(c, &sample);
  int matched = floats_match_as_str(sample, -1.16f);
  muAssert(0 == matched, "Expected sample");
  muAssert(c->entry_sample_count == 1000, "Expected complete");
  muAssert(c->padding_sample_count == 1000, "Expected complete");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_double_dial(void) {
  dtmf_context_t *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 1000);
  dtmf_dial(c, "954");
  dtmf_dial(c, "8");
  muAssert(strcmp(c->entries, "5109548") == 0, "Expected concat");
  dtmf_free(c);
  return NULL;
}

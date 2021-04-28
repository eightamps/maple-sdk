//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "dtmf_test.h"
#include "test_helper.h"
#include <minunit.h>
#include <stdio.h>
#include <string.h>

#define DTMF_TEST_SAMPLE_RATE 1000

char *test_dtmf_null_numbers(void) {
  DtmfContext *c = dtmf_new();
  int status = dtmf_dial(c, NULL);
  muAssert(status == -EINVAL, "Expected invalid argument");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_empty_numbers(void) {
  // DtmfContext *c = dtmf_new("", 1000);
  DtmfContext *c = dtmf_new();
  int status = dtmf_dial(c, "");
  muAssert(status == -EINVAL, "Expected invalid argument");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_next_sample_without_dial(void) {
  DtmfContext *c = dtmf_new();
  float *sample;
  int status = dtmf_next_sample(c, &sample);
  muAssert(status == -EINVAL, "Expected failure status");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_duration_single(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "5");
  float *sample;
  int status = dtmf_next_sample(c, &sample);

  int floats_match = floats_match_as_str(*sample, 0.0f);
  muAssert(0 == status, "Expected success");
  muAssert(0 == floats_match, "Expected zero sample");
  muAssert(250 == c->duration_ms, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_duration_multiple(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510");
  float *result;
  int status = dtmf_next_sample(c, &result);
  muAssert(0 == status, "Expected status");
  muAssert(c->duration_ms == 950, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_count(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "5");
  dtmf_set_sample_rate(c, 1000);
  float *result;
  dtmf_next_sample(c, &result);
  dtmf_next_sample(c, &result);
  dtmf_next_sample(c, &result);
  dtmf_next_sample(c, &result);
  dtmf_next_sample(c, &result);
  int status = dtmf_next_sample(c, &result);
  int matched = floats_match_as_str(-0.85f, *result);
  muAssert(0 == status, "Expected status");
  muAssert(0 == matched, "Expected match");
  muAssert(c->sample_count == 250, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_multiple(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "51");
  dtmf_set_sample_rate(c, 1000);
  float *result;
  dtmf_next_sample(c, &result);
  muAssert(c->sample_count == 600, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "5");
  dtmf_set_sample_rate(c, 44100);
  float *result;
  dtmf_next_sample(c, &result);
  muAssert(0.0f == *result, "Expected result");
  muAssert(c->sample_count == 11025, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_multiple(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "51");
  dtmf_set_sample_rate(c, 44100);
  float *result;
  dtmf_next_sample(c, &result);
  muAssert(0.0f == *result, "Expected result");
  muAssert(c->sample_count == 26460, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_three(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 44100);
  float *result;
  dtmf_next_sample(c, &result);
  muAssert(c->sample_count == 41895, "Expected duration");

  // Move the cursor to the middle of the range.
  c->sample_index = 20000;
  dtmf_next_sample(c, &result);
  int matched = floats_match_as_str(*result, 0.770f);
  muAssert(0 == matched, "Expected result");

  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_index(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 1000);
  c->sample_index = 974;
  float *result;
  int status = dtmf_next_sample(c, &result);
  int matched = floats_match_as_str(*result, 0.0f);
  muAssert(0 == matched, "Expected result");
  muAssert(0 == status, "Expected status");
  muAssert(0.0f == *result, "Expected result");
  muAssert(c->is_active == false, "Expected complete");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_entry_and_padding(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 2000);
  c->sample_index = 100;
  float *result;
  dtmf_next_sample(c, &result);
  int matched = floats_match_as_str(*result, -0.476f);
  muAssert(0 == matched, "Expected result");
  muAssert(c->entry_sample_count == 500, "Expected complete");
  muAssert(c->padding_sample_count == 200, "Expected complete");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_double_dial(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510");
  dtmf_set_sample_rate(c, 1000);
  dtmf_dial(c, "954");
  dtmf_dial(c, "8");

  printf("YOO: %s\n", c->entries);
  muAssert(strcmp(c->entries, "5109548") == 0, "Expected concat");
}

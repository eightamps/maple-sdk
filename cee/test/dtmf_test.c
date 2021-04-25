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
  int status = dtmf_dial(c, NULL, DTMF_TEST_SAMPLE_RATE);
  muAssert(status == -EINVAL, "Expected invalid argument");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_empty_numbers(void) {
  // DtmfContext *c = dtmf_new("", 1000);
  DtmfContext *c = dtmf_new();
  int status = dtmf_dial(c, "", DTMF_TEST_SAMPLE_RATE);
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
  dtmf_dial(c, "5", DTMF_TEST_SAMPLE_RATE);
  float *sample;
  int status = dtmf_next_sample(c, &sample);
  muAssert(status == 0, "Expected success");
  muAssert(*sample == 0.0f, "Expected zero sample");
  muAssert(c->duration_ms == 250, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_duration_multiple(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510", DTMF_TEST_SAMPLE_RATE);
  float *result;
  int status = dtmf_next_sample(c, &result);
  muAssert(0 == status, "Expected status");
  muAssert(c->duration_ms == 950, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_count(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "5", DTMF_TEST_SAMPLE_RATE);
  float *result;
  int status = dtmf_next_sample(c, &result);
  muAssert(0 == status, "Expected status");
  muAssert(c->sample_count == 250, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_multiple(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "51", DTMF_TEST_SAMPLE_RATE);
  float *result;
  dtmf_next_sample(c, &result);
  muAssert(c->sample_count == 600, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "5", 44100);
  float *result;
  dtmf_next_sample(c, &result);
  muAssert(0.0f == *result, "Expected result");
  muAssert(c->sample_count == 11025, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_multiple(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "51", 44100);
  float *result;
  dtmf_next_sample(c, &result);
  muAssert(0.0f == *result, "Expected result");
  muAssert(c->sample_count == 26460, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_three(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510", 44100);
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
  dtmf_dial(c, "510", DTMF_TEST_SAMPLE_RATE);
  c->sample_index = 974;
  float *result;
  int status = dtmf_next_sample(c, &result);
  int matched = floats_match_as_str(*result, 0.0f);
  muAssert(0 == matched, "Expected result");
  muAssert(0 == status, "Expected status");
  muAssert(0.0f == *result, "Expected result");
  muAssert(c->is_complete == true, "Expected complete");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_entry_and_padding(void) {
  DtmfContext *c = dtmf_new();
  dtmf_dial(c, "510", 2000);
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

//
// Created by lukebayes on 4/19/21.
//

#include "dtmf.h"
#include "dtmf_test.h"
#include <minunit.h>
#include <stdio.h>
#include <string.h>

char *test_dtmf_null_numbers(void) {
  DtmfContext *c = dtmf_new(NULL, 1000);
  muAssert(c == NULL, "Expected NULL");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_empty_numbers(void) {
  DtmfContext *c = dtmf_new("", 1000);
  muAssert(c == NULL, "Expected NULL for empty");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_duration_single(void) {
  DtmfContext *c = dtmf_new("5", 1000);
  float result = dtmf_next_sample(c);
  muAssert(c->duration_ms == 250, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_duration_multiple(void) {
  DtmfContext *c = dtmf_new("510", 1000);
  float result = dtmf_next_sample(c);
  muAssert(c->duration_ms == 950, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_count(void) {
  DtmfContext *c = dtmf_new("5", 1000);
  float result = dtmf_next_sample(c);
  muAssert(c->sample_count == 250, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_multiple(void) {
  DtmfContext *c = dtmf_new("51", 1000);
  float result = dtmf_next_sample(c);
  muAssert(c->sample_count == 600, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate(void) {
  DtmfContext *c = dtmf_new("5", 44100);
  float result = dtmf_next_sample(c);
  muAssert(c->sample_count == 11025, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_multiple(void) {
  DtmfContext *c = dtmf_new("51", 44100);
  float result = dtmf_next_sample(c);
  muAssert(c->sample_count == 26460, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_large_sample_rate_three(void) {
  DtmfContext *c = dtmf_new("510", 44100);
  float result = dtmf_next_sample(c);
  muAssert(c->sample_count == 41895, "Expected duration");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_sample_index(void) {
  DtmfContext *c = dtmf_new("510", 1000);
  c->sample_index = 974;
  dtmf_next_sample(c);
  muAssert(c->is_complete == true, "Expected complete");
  dtmf_free(c);
  return NULL;
}

char *test_dtmf_entry_and_padding(void) {
  DtmfContext *c = dtmf_new("510", 2000);
  c->sample_index = 100;
  dtmf_next_sample(c);
  muAssert(c->entry_sample_count == 500, "Expected complete");
  muAssert(c->padding_sample_count == 200, "Expected complete");
  dtmf_free(c);
  return NULL;
}

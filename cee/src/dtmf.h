//
// This code was originally created by https://github.com/jamesu and found in
// this Gist: https://gist.github.com/jamesu/3296747
//
// Lightly modified by lukebayes
//

#ifndef MAPLE_DTMF_H
#define MAPLE_DTMF_H

#include <stdbool.h>

#define DTMF_DEFAULT_SAMPLE_RATE 44100
#define DTMF_MS_PER_ENTRY 500
#define DTMF_MS_PER_SPACE 500
#define DTMF_SAMPLE_RATE_MULTIPLIER 4

typedef struct {
  int duration_ms;
  int entry_ms;
  int entry_sample_count;
  int padding_ms;
  int padding_sample_count;
  int sample_count;
  int sample_index;
  int sample_rate;
  bool is_active;
  char *entries;
}dtmf_context_t;

/**
 * Create a new DTMF context object.
 *
 * @return dtmf_context_t*: A context object for getting samples.
 */
dtmf_context_t *dtmf_new(void);

/**
 * Generate tones for the provided values at the provided sample_rate.
 * @param values: A char array of PSTN "numbers" (e.g., "727555555#"
 * @param sample_rate: The sample rate in Hz (e.g., 44100)
 * @return
 */
int dtmf_dial(dtmf_context_t *c, const char *values);

/**
 * Set the DTMF sample rate.
 *
 * This should be done before calling next_sample.
 * @param *DtmfContext
 * @param int sample_rate
 * @return Status code
 */
int dtmf_set_sample_rate(dtmf_context_t *c, int sample_rate);
/**
 * Get the next DTMF sample for the configured context.
 *
 * Calling this function will increment an internal iterator and return the
 * current (float) sample value.
 *
 * @param DtmfContext*: The context for samples.
 * @return float: The current sample value.
 */
int dtmf_next_sample(dtmf_context_t *c, float *sample);

/**
 * Free the provided context object.
 * @param DtmfContext*
 */
void dtmf_free(dtmf_context_t *c);

#endif // MAPLE_DTMF_H

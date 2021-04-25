//
// This code was originally created by https://github.com/jamesu and found in
// this Gist: https://gist.github.com/jamesu/3296747
//
// Lightly modified by lukebayes
//

#ifndef MAPLE_DTMF_H
#define MAPLE_DTMF_H

#include <stdbool.h>

typedef struct DtmfContext {
  int duration_ms;
  int entry_ms;
  int entry_sample_count;
  int padding_ms;
  int padding_sample_count;
  int sample_count;
  int sample_index;
  int sample_rate;
  bool is_active;
  bool is_complete;
  char *entries;
}DtmfContext;

/**
 * Create a new DTMF context object.
 *
 * @return DtmfContext*: A context object for getting samples.
 */
DtmfContext *dtmf_new(void);

/**
 * Generate tones for the provided values at the provided sample_rate.
 * @param values: A char array of PSTN "numbers" (e.g., "727555555#"
 * @param sample_rate: The sample rate in Hz (e.g., 44100)
 * @return
 */
int dtmf_dial(DtmfContext *c, const char *values, int sample_rate);

/**
 * Get the next DTMF sample for the configured context.
 *
 * Calling this function will increment an internal iterator and return the
 * current (float) sample value.
 *
 * @param DtmfContext*: The context for samples.
 * @return float: The current sample value.
 */
int dtmf_next_sample(DtmfContext *c, float **sample);

/**
 * Free the provided context object.
 * @param DtmfContext*
 */
void dtmf_free(DtmfContext *c);

#endif // MAPLE_DTMF_H

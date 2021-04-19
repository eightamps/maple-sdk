//
// This code was originally created by https://github.com/jamesu and found in
// this Gist: https://gist.github.com/jamesu/3296747
//
// Lightly modified by lukebayes
//

#ifndef MAPLE_DTMF_H
#define MAPLE_DTMF_H

typedef struct DtmfContext {
  int sample_rate;
  int samples_index;
  int samples_count;
  char *values;
  float *samples;
}DtmfContext;

DtmfContext *dtmf_new(char *values, int sample_rate);
void dtmf_free(DtmfContext *context);

#endif // MAPLE_DTMF_H

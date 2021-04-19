//
// This code was originally created by https://github.com/jamesu and found in
// this Gist: https://gist.github.com/jamesu/3296747
//
// Lightly modified by lukebayes
//

#ifndef MAPLE_DTMF_H
#define MAPLE_DTMF_H

typedef struct DtmfToneInfo {
  const char *name;
  int tone1;
  int tone2;
}DtmfToneInfo;

DtmfToneInfo DtmfTones[] = {
  {"1", 1209, 697},
  {"2", 1336, 697},
  {"3", 1477, 697},
  {"A", 1633, 697},
  {"4", 1209, 770},
  {"5", 1336, 770},
  {"6", 1477, 770},
  {"B", 1633, 770},
  {"7", 1209, 852},
  {"8", 1336, 852},
  {"9", 1477, 852},
  {"C", 1633, 852},
  {"*", 1209, 941},
  {"0", 1336, 941},
  {"#", 1477, 941},
  {"D", 1633, 941}
};

void dtmf_generate(int tone1, int tone2, int rate, float *out);

#endif // MAPLE_DTMF_H

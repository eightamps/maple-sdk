//
// Created by lukebayes on 4/25/21.
//

#include "test_helper.h"
#include "stdio.h"
#include "string.h"

int floats_match_as_str(float a, float b) {
  char a_str[10];
  char b_str[10];

  sprintf(a_str, "%.3f", a);
  sprintf(b_str, "%.3f", b);

  int result = strcmp(a_str, b_str);
  if (0 != result) {
    fprintf(stderr, "floats_match_as_str failed with a: %s vs b: %s\n", a_str,
            b_str);
  }
  return result;
}

//
// Created by lukebayes on 4/25/21.
//

#include "test_helper.h"
#include "stdio.h"
#include "string.h"

/**
 * Returns 0 if both floats convert to the same 3 digit string value using
 * snprintf.
 *
 * @param a (float)
 * @param b (float)
 * @return 0 if matching
 */
int floats_match_as_str(float a, float b) {
  const size_t len = 6;
  char a_str[len];
  char b_str[len];

  snprintf(a_str, len, "%.3f", a);
  snprintf(b_str, len, "%.3f", b);

  int result = strncmp(a_str, b_str, len);
  if (0 != result) {
    fprintf(stderr, "floats_match_as_str failed with a: %s vs b: %s\n", a_str,
            b_str);
  }

  return result;
}

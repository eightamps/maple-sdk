#include "phony.h"
#include <stdio.h>
#include <unistd.h>

#define DEFAULT_8A_PHONE_NUMBER "7273392258"

int phony_exercise(void) {
  PhonyContext *c = phony_new();
  int status = phony_open_maple(c);
  if (status < 0) {
    return status;
  }

  status = phony_dial(c, DEFAULT_8A_PHONE_NUMBER);
  printf("phony_dial complete\n");

  if (status < 0) {
    return status;
  }

  sleep(10);

  phony_hang_up(c);
  phony_free(c);
  printf("EXITING\n");
}


int main() {
  // return stitcher_exercise();
  return phony_exercise();
}

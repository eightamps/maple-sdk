#include <phony.h>
#include <stitcher.h>
#include <stdio.h>
#include <stdlib.h>

#define MAPLE_VID 0x335e
#define MAPLE_PID 0x8a01

int phony_show_info(void) {
  // printf("Attempting to allocate PhonyContext\n");
  PhonyContext *c = phony_new(MAPLE_VID, MAPLE_PID);
  if (c == NULL) return ENOMEDIUM;

  int status;

  // printf("Attempting to initialize PhonyContext device\n");
  status = phony_init(c);
  if (status != EXIT_SUCCESS) return status;

  // printf("Attempting to gather PhonyContext device info\n");
  status = phony_info(c);
  if (status != EXIT_SUCCESS) return status;

  // printf("Attempting to free PhonyContext allocation\n");
  phony_free(c);
  return EXIT_SUCCESS;
}

int stitcher_exercise(void) {
  StitcherContext *c = stitcher_new();
  stitcher_init(c);
  stitcher_free(c);
  return EXIT_SUCCESS;
}

int main() {
  // return phony_show_info();
  return stitcher_exercise();
}

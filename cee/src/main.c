#include <stdio.h>
#include <stdlib.h>
#include <phony.h>

#define MAPLE_VID 0x335e
#define MAPLE_PID 0x8a01

int phony_show_info(void) {
  // printf("Attempting to allocate PhonyContext\n");
  PhonyContext *phony = phony_new(MAPLE_VID, MAPLE_PID);
  if (phony == NULL) return ENOMEDIUM;

  int status;

  // printf("Attempting to initialize PhonyContext device\n");
  status = phony_init(phony);
  if (status != EXIT_SUCCESS) return status;

  // printf("Attempting to gather PhonyContext device info\n");
  status = phony_info(phony);
  if (status != EXIT_SUCCESS) return status;

  // printf("Attempting to free PhonyContext allocation\n");
  return phony_free(phony);
}

int main() {
  return phony_show_info();
}

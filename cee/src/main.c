#include <stdio.h>
#include <phony.h>

#define MAPLE_VID 0x335e
#define MAPLE_PID 0x8a01

PhonyStatus phony_show_info(void) {
  // printf("Attempting to allocate Phony\n");
  Phony *phony = phony_new(MAPLE_VID, MAPLE_PID);
  if (phony == NULL) {
    return PhonyStatusFailureToAlloc;
  }

  PhonyStatus status;

  // printf("Attempting to initialize Phony device\n");
  status = phony_init(phony);
  if (status != PhonyStatusSuccess) {
    return status;
  }

  // printf("Attempting to gather Phony device info\n");
  status = phony_info(phony);
  if (status != PhonyStatusSuccess) {
    return status;
  }

  // printf("Attempting to free Phony allocation\n");
  return phony_free(phony);
}

int main() {
  return phony_show_info();
}

#include <stdio.h>
#include <phony.h>

PhonyStatus execute_phony(void) {
  printf("Attempting to allocate Phony\n");
  Phony *phony = phony_new(0x01, 0x02);
  if (phony == NULL) {
    return PhonyStatusFailureToAlloc;
  }

  PhonyStatus status;

  printf("Attempting to initialize Phony device\n");
  status = phony_init(phony);
  if (status != PhonyStatusSuccess) {
    return status;
  }

  printf("Attempting to gather Phony device info\n");
  status = phony_info(phony);
  if (status != PhonyStatusSuccess) {
    return status;
  }

  printf("Attempting to free Phony allocation\n");
  return phony_free(phony);
}

int main() {
  return execute_phony();
}

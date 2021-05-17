#include <stdio.h>
#include <stdint.h>
#include "maple-sdk.h"

int main(int argc, char **argv) {
  printf("Loaded maple-sdk from C\n");
  int result = add(39, 3);
  printf("DLL result: %d\n", result);
  return 0;
}


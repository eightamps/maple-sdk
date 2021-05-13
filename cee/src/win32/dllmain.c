// dllmain.cpp : Defines the entry point for the DLL application.
#include "log.h"
#include "shared.h"
// #include "stitch.h"
#include <mmdeviceapi.h>

void stitch_clean_up(void) {
  log_info("stitch_clean_up (on DLL exit)");
  CoUninitialize();
  /*
  std::cout << "stitch_clean_up with: " << stitch_index << std::endl;
  for (int i = 0; i < stitch_index; i++) {
      if (stitches[i] != NULL) {
    StitchImpl** s = stitches[i];
          std::cout << "stitch_clean_up freeing: " << s << std::endl;
          delete s;
      }
  }
  */
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  log_info(">>>>>>>>>> DLLMAIN CALLED WITH: %d", ul_reason_for_call);
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      stitch_clean_up();
      break;
  }
  return TRUE;
}

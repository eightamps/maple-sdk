//
// Created by lukebayes on 5/12/21.
//

#ifdef PHONY_PLATFORM_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_EXPORT
#define DLL_IMPORT
#endif

DLL_EXPORT int stitch_new(char* label) {
  return 0;
}

DLL_EXPORT int stitch_init(int ptr) {
  return 0;
}

DLL_EXPORT int stitch_start(int ptr) {
  return 0;
}

DLL_EXPORT int stitch_stop(int ptr) {
  return 0;
}

DLL_EXPORT void stitch_free(int ptr) {
}


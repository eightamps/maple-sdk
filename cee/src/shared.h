#ifndef __shared_h__
#define __shared_h__

#include "log.h"
#include "time.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#define CINTERFACE
#define COBJMACROS
#define CONST_VTABLE
#define INITGUID

#include <sdkddkver.h>
#include <windows.h>

#ifndef E_NOTFOUND
#define E_NOTFOUND 0x80070490
#endif //E_NOTFOUND

#define EXIT_ON_ERROR(hres, message)  \
  if (FAILED(hres)) { log_err("ERROR[0x%x]: %s", (int)hres, message); \
    goto ExitWithError; }
#define SAFE_FREE(punk)  \
  if ((punk) != NULL)  \
{ free(punk); (punk) = NULL; }
#endif // _WIN32


#ifdef MAPLE_EXPORT_DLL
#define DLL_LINK __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif MAPLE_IMPORT_DLL
#define DLL_LINK __declspec(dllimport)
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_LINK
#define DLL_IMPORT
#endif // MAPLE_EXPORT_DLL

#define EIGHT_AMPS_VID 0x335e
#define EIGHT_AMPS_MAPLE_V3_PID 0x8a01

// Bullshit time.h header requires me to declare this built-in signature for
// some ridiculous reason.
// https://cs50.stackexchange.com/questions/22521/nanosleep-function-implicit-deceleration
int nanosleep(const struct timespec *req, struct timespec *rem);

// Apparently, I'm the only person who has needed to idle C since 2006?!
// usleep is deprecated and nanosleep has a horrible signature. Of course,
// there's also clock_nanosleep, which is even crazier.
//
// Looks like I might also need to figure something else out for Win32 timers
// https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
//
// TODO(lbayes): Verify that this shim works.
#define usleep_shim(usec) {\
  struct timespec a = { .tv_nsec = usec * 1000 };\
  struct timespec b = { .tv_nsec = usec * 1000 };\
  nanosleep(&a, &b); } \

// Some SO posts indicate we should also place this call into a (barf) while loop.
// while (nanosleep(&a, &b) && errno == EINTR); }

#endif // __shared_h__


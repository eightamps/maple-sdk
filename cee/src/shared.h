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


// Bullshit header requires me to declare the built-in signature.
int nanosleep(const struct timespec *req, struct timespec *rem);

#define usleep_shim(usec) {\
  struct timespec a = { .tv_nsec = usec * 1000 };\
  struct timespec b = { .tv_nsec = usec * 1000 };\
  nanosleep(&a, &b); }

  // while (nanosleep(&a, &b) && errno == EINTR);}


#endif // __shared_h__


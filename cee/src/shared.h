#ifndef __shared_h__
#define __shared_h__

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define INITGUID
#define CINTERFACE
#define COBJMACROS
#define CONST_VTABLE

#include <sdkddkver.h>
#include <windows.h>

#ifndef E_NOTFOUND
#define E_NOTFOUND 0x80070490
#endif //E_NOTFOUND

#define EXIT_ON_ERROR(hres, message)  \
              if (FAILED(hres)) { log_err("ERROR[0x%x]: %s", hres, message); \
              goto ExitWithError; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }
#endif

#ifdef MAPLE_EXPORT_DLL
#define DLL_LINK __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif MAPLE_IMPORT_DLL
#define DLL_LINK __declspec(dllimport)
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_LINK
#define DLL_IMPORT
#endif

#endif // __shared_h__
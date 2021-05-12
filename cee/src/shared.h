#ifndef __shared_h__
#define __shared_h__

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


#endif // __shard_h__
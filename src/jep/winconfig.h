
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// If we are on MSVC, disable some stupid MSVC warnings
#ifdef _MSC_VER
# pragma warning (disable:4100)

// MSVC 6 doesn't have
# if _MSC_VER < 1310
    typedef jlong intptr_t;
# endif
#endif

#define R_OK 4
#define F_OK 0

/* Define if you want to deallocate objects. */
#define USE_DEALLOC 1

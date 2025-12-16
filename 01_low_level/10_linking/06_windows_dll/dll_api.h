#pragma once
#if defined(_WIN32) || defined(__CYGWIN__)
  #if defined(BUILDING_DEMO_DLL)
    #define DLL_API __declspec(dllexport)
  #else
    #define DLL_API __declspec(dllimport)
  #endif
#else
  #define DLL_API
#endif

extern "C" {
    DLL_API int dll_add(int a, int b);
}

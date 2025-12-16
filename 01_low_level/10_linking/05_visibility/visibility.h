#pragma once

#if defined(__GNUC__) || defined(__clang__)
  #define VIS_DEFAULT __attribute__((visibility("default")))
  #define VIS_HIDDEN  __attribute__((visibility("hidden")))
#else
  #define VIS_DEFAULT
  #define VIS_HIDDEN
#endif

VIS_DEFAULT int exported_add(int a, int b);
VIS_HIDDEN  int hidden_add(int a, int b);

#pragma once

#if __cplusplus >= 202002L
#include <format>
#endif

// Disable unknown pragma warnings

#if defined(_MSC_VER)
#define SUPPRESS_PRAGMA_WARNINGS() \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4068))
#else
#define SUPPRESS_PRAGMA_WARNINGS()
#endif


// Suppress counter-examples
// warning C4018: '<': signed/unsigned mismatch as a counter-example
// warning C4244: 'initializing' : conversion from '__int64' to 'long', possible loss of data
#if defined(_MSC_VER)
#define SUPPRESS_UNSIGNED_COUNTEREXAMPLE_WARNINGS() \
    __pragma(warning(push)) \
    __pragma(warning(disable:4018 4244))
#else
#define SUPPRESS_UNSIGNED_COUNTEREXAMPLE_WARNINGS()
#endif

#pragma once

#if __cplusplus >= 202002L
#include <format>
#endif

// Disable unknown pragma warnings

#if defined(_MSC_VER)
#define SUPPRESS_PRAGMA_WARNINGS() \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4068))

#define SUPPRESS_UNINITIALIZED_WARNINGS() \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4700))
#elif defined(__clang__)
#define SUPPRESS_PRAGMA_WARNINGS()
#define SUPPRESS_UNINITIALIZED_WARNINGS() \
    __pragma("clang diagnostic ignored \"-Wuninitialized\"")
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

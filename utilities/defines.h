#pragma once

#if __cplusplus >= 202002L
#include <format>
#endif

// Suppress counter-examples
// warning C4018: '<': signed/unsigned mismatch as a counter-example
// warning C4244: 'initializing' : conversion from '__int64' to 'long', possible loss of data
#if defined(_MSC_VER) and defined(MSVC_UNSIGNED_COUNTEREXAMPLE)
#pragma warning( disable : 4018 4244)
#endif

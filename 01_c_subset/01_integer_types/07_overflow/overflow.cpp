#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <utilities/bitwise.h>
#include <utilities/defines.h>
SUPPRESS_UNSIGNED_COUNTEREXAMPLE_WARNINGS()

#include <iostream>
#include <cstdint>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types


/// Arithmetic operations in C/C++ are NOT guaranteed to yield a correct mathematical result.
/// Integer overflow results in undefined behavior

int safe_addition(int x, int y)
{
  if (((y > 0) && (x > (INT_MAX - y))) || ((y < 0) && (x < (INT_MIN - y)))) {
      // handle error
      return INT_MAX;
  }
  return x + y;
}

void integer_overflow()
{
    // Is there overflow?
    int16_t x = 30000;
    int16_t y = x * 2 / 4;
}

// Solution could be a library like boost::safe_numerics
// It checks for overflow and underflow in compile time if possible,
// and in runtime if required

// Undefined Behavior in LLVM
// LLVM-compiled code sometimes generates SIGTRAP signals when the optimizer is turned on
// http://blog.llvm.org/2011/05/what-every-c-programmer-should-know.html

int main(int argc, char* argv[])
{
    integer_overflow();
    return 0;
}

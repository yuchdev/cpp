## Types and Definitions

---

### Char types

* `char`, `signed char`, and `unsigned char` are three distinct types
  Even though `char` is often implemented as either signed or unsigned, it is a *separate type* in the language.

* The signedness of plain `char` is implementation-defined.
  On some platforms `char` behaves like `signed char`, on others like `unsigned char`. This affects comparisons, promotions, and overload resolution.

* `CHAR_BIT` defines the number of bits in a byte.
  Contrary to common assumptions, a byte is *not guaranteed to be 8 bits*.
  Historical and embedded systems used many exotic sizes:

  * CDC 6x00: `char == 6` bits
  * PDP-8: `char == 12` bits
  * PDP-11, Windows CE: `char == 16` bits
  * Motorola DSP56k: `char == 24` bits
    The exact number is exposed via the macro `CHAR_BIT`.

* C++ supports multiple character types for different encodings

  ```cpp
  char, signed char, unsigned char,
  wchar_t, char16_t, char32_t
  ```

  These represent *code units*, not characters or glyphs.

* `wchar_t` is implementation-defined
  Its size and encoding vary:

  * Windows: UTF-16 (16-bit)
  * Linux/Unix: UTF-32 (32-bit)

* `char16_t` and `char32_t` were added in C++11.
  They are intended for UTF-16 and UTF-32 code units respectively, and have fixed sizes.

* Character literals can be prefixed

  ```cpp
  'a'    // char
  L'a'   // wchar_t
  u'a'   // char16_t
  U'a'   // char32_t
  ```

  *(Unicode literal prefixes added in C++11)*

* The notation `int{c}` yields the integer value of a character
  This is a safe, explicit way to convert characters to integers without relying on promotions.

* Some characters have standardized escape names.
  For example: `\n`, `\t`, `\0`, `\\`, etc.
  The actual numeric values of many control characters remain implementation-defined.

* Only `char`, `unsigned char`, and `std::byte` may alias arbitrary memory
  This is a special rule in the object model and is why serializers and memory inspectors rely on them.

---

### Integer types

* C++ provides multiple integer types to match hardware capabilities.
  The purpose is *not semantic clarity*, but efficient mapping to registers and instructions.

* Guaranteed ordering of integer sizes

  ```cpp
  sizeof(char) <= sizeof(short) <= sizeof(int)
               <= sizeof(long) <= sizeof(long long)
  ```

* Signedness does not affect size

  ```cpp
  sizeof(int) == sizeof(unsigned int)
  sizeof(long) == sizeof(unsigned long)
  ```

* `long long` was *added in C++11, inherited from C99*.
  It guarantees at least 64 bits.

* Exact-width and fast integer types live in `<cstdint>`
  *(added in C++11)*. Examples:

  ```cpp
  int16_t, uint32_t, int_fast64_t
  ```

  * Exact-width types may not exist on all platforms, but will be substituted with the next-larger type if needed.
  * `*_fast*` types trade size for speed

* The type of integer literal depends on form, value, and suffix

  ```cpp
  42        // int
  42u       // unsigned int
  42L       // long
  42LL      // long long
  0xFF      // could be int, unsigned, or long
  ```

* Unsigned integer overflow is well-defined.
  It wraps modulo 2ⁿ.

* Signed integer overflow is undefined behavior.
  Compilers aggressively optimize assuming it *never happens*.

* Integer alignment matters for performance.
  Misaligned integer access may be slower or even illegal on some architectures.

* `alignof(T)` yields alignment requirements.
  Use `alignas()` when alignment must be enforced in declarations.

* Floating-point multiplication can be faster than integer multiplication.
  On many modern CPUs:

  * FP units are deeply pipelined and always present
  * Integer multiplication may have higher latency or contention
  * FP units often handle more parallel operations
    This reverses the intuition inherited from older hardware.

---

### Other fundamental & type-system facts

* Many behaviors are implementation-defined or unspecified

  * Implementation-defined: documented per compiler/ABI
  * Unspecified: allowed to vary per evaluation
  * Undefined behavior: *no guarantees at all*

* The value returned by `operator new` is unspecified (but aligned).
  Only alignment and non-nullness (unless throwing) are guaranteed.

* C++ has hosted and freestanding implementations

  * Hosted: full standard library
  * Freestanding: minimal runtime for embedded, kernels, etc.

* Guaranteed size relationships

  ```cpp
  1 <= sizeof(bool) <= sizeof(long)
  sizeof(float) <= sizeof(double) <= sizeof(long double)
  sizeof(char) <= sizeof(wchar_t) <= sizeof(long)
  ```

* `sizeof` and `<limits>` expose implementation-defined properties.
  Many portability assumptions should be checked via `std::numeric_limits`.

* Some types require specific alignment for efficient access.
  Misaligned access can cause traps or slow paths on real hardware.

* `decltype(expr)` names types derived from expressions.
  It preserves reference-ness and value category, unlike `auto`.

* Four initialization syntaxes exist

  ```cpp
  X a1{v};
  X a2 = {v};
  X a3 = v;
  X a4(v);
  ```

* Brace-initialization prevents narrowing.
  This is one of the most important safety improvements of modern C++.

* `auto` with `{}` may deduce `std::initializer_list`

  ```cpp
  auto v{12345}; // NOT int -- initializer_list<int>
  ```

  Prefer `=` with `auto` unless list semantics are intended.

* Type aliases (`using`) are superior to `typedef`

  * Can be templated *(added in C++11)*
  * Cleaner function pointer syntax
  * No redeclaration ambiguities
  * Works better with dependent types

* Type specifiers cannot be applied to aliases

  ```cpp
  using Uchar = unsigned char; // OK
  using Uchar = unsigned Char; // error
  ```

* `std::byte` is not an integer type *(added in C++17)*.
  It exists to represent raw memory without arithmetic.

* `sizeof(char)` is always `1` by definition, even if `CHAR_BIT != 8`.

* Type names are part of the ABI.
  Changing `int` to `long` in public interfaces can silently break binaries.

* Alignment can affect object size
  `sizeof(T)` may be larger than the sum of its fields due to padding.

* The order of calculations inside subexpressions is not defined 

```cpp
int x = f() + g() * h(); // f(), g(), h() can be called in any order
v[i] = i++ // UB
```
 
---

### Scopes (quick facts)

* Statement scope: limited to control statements
* Local scope: function body
* Class scope: member declarations
* Namespace scope: namespace members
* Global scope: outside all named scopes

---

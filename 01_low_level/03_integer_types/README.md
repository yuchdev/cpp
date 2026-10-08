# 03. Integer Types in Modern C++

Integer arithmetic in C++ looks simple until the program crosses a boundary: a different ABI, a different integer rank, a signed/unsigned comparison, a narrowing conversion, a serialization format, or an optimizer that takes undefined behavior seriously.

For experienced C++ developers, the difficult part is rarely remembering that `int` is usually 32 bits. The difficult part is knowing **which properties are guaranteed by the language, which are ABI choices, which conversions happen before an operator is evaluated, and which apparently harmless expressions change meaning when a type changes**.

This chapter uses the eight accompanying example programs as its structure:

1. `character_types.cpp` -- character types, bytes, object representation, and Unicode code units.
2. `integer_types.cpp` -- fundamental integer types, data models, endianness, and ABI consequences.
3. `universal_init.cpp` -- initialization syntax, narrowing, and `auto` deduction.
4. `unsigned_types.cpp` -- modular arithmetic, mixed signedness, and shifts.
5. `promotions.cpp` -- integral promotions, ranks, enums, and overload resolution.
6. `conversions.cpp` -- value-changing numeric conversions and range safety.
7. `fixed_size.cpp` -- exact-, least-, fast-, pointer-, and maximum-width integer types.
8. `random_int.cpp` -- integer random-number generation, range mapping, reproducibility, and statistical traps.

The emphasis is on the cases that still surprise people who write C++ professionally: promotion before arithmetic, literal-type selection, ABI data models, two's-complement guarantees, inactive union members, modulo semantics, `uint8_t` behaving like a character type, and reproducibility traps in `<random>`.

---

## 1. Character types are integer types, but they are not interchangeable

The character types sit at the intersection of three different concepts that C++ deliberately keeps separate:

* textual code units;
* small integer values;
* raw bytes used to inspect object representation.

Treating those as the same abstraction is the source of many low-level bugs.

### `char`, `signed char`, and `unsigned char` are three distinct types

Plain `char` is not an alias for either signed character type. It is a distinct type whose representation, size, and alignment match one of them. Whether plain `char` behaves as signed or unsigned is implementation-defined.

```cpp
static_assert(!std::is_same_v<char, signed char>);
static_assert(!std::is_same_v<char, unsigned char>);
```

This distinction matters in overload resolution and templates:

```cpp
void f(char);
void f(signed char);
void f(unsigned char);

f('A'); // calls f(char)
```

The signedness of plain `char` is an ABI/compiler choice. GCC and Clang can even change it with options such as `-fsigned-char` and `-funsigned-char`, which means code that treats plain `char` as a tiny numeric type can change behavior without changing source.

A classic failure mode appears when bytes above `0x7f` are promoted:

```cpp
char c = '\xff';
auto x = c;       // still char
auto y = +c;      // integral promotion: usually int
```

If plain `char` is signed and 8 bits wide, `y` is commonly `-1`. If it is unsigned, `y` is commonly `255`.

That is one reason binary data should normally be represented as `std::byte` or `unsigned char`, not plain `char` used as a number.

### A C++ byte is `sizeof(char)`, not necessarily eight bits

The language defines:

```cpp
sizeof(char) == 1
```

but it does **not** define a byte as eight bits. `CHAR_BIT` in `<climits>` gives the number of bits in one C++ byte.

```cpp
static_assert(sizeof(char) == 1);
std::cout << CHAR_BIT;
```

The standard requires `CHAR_BIT >= 8` in modern C++, but unusual machines have existed with 9-, 12-, 16-, 24-, and other-width addressable units. This matters to code that equates "byte count" with "octet count."

For example:

```cpp
std::array<std::byte, 4> payload;
```

contains four C++ bytes. It is a 32-bit payload only on an implementation with `CHAR_BIT == 8`.

Protocol, file-format, cryptography, compression, and network code usually specifies sizes in **octets**. If octets are required, it is reasonable to make that platform requirement explicit:

```cpp
static_assert(CHAR_BIT == 8, "This protocol implementation requires 8-bit bytes");
```

That is better than silently assuming it throughout the implementation.

### Character types are integral types, so promotions apply to them

Most arithmetic does not happen in `char`, `signed char`, or `unsigned char` at all. Integral promotions occur first:

```cpp
unsigned char a = 200;
unsigned char b = 100;
auto sum = a + b;
```

On ordinary implementations where `int` can represent all values of `unsigned char`, `sum` is an `int` with value `300`, not an `unsigned char` containing `44`.

This is an important theme for the rest of the chapter: **the declared type of operand is not necessarily the type in which an expression is evaluated**.

### `char8_t`, `char16_t`, and `char32_t` are code-unit types

`char8_t` was added in C++20 as a distinct type for UTF-8 code units. `char16_t` and `char32_t` arrived in C++11 for UTF-16 and UTF-32 code units.

```cpp
auto a = u8'A';        // char8_t since C++20
auto b = u'ß';         // char16_t
auto c = U'🍌';        // char32_t
```

These types do not mean "one human-readable character." Unicode distinguishes code units, code points, grapheme clusters, and rendered glyphs. A visible character can require several code points, and UTF-16 may require a surrogate pair for one code point.

`char8_t` is especially important for overload resolution and API migration. Before C++20 a UTF-8 string literal had an array type based on `char`; in C++20 it is based on `char8_t`:

```cpp
u8"text" // const char8_t[N] in C++20
```

Code that previously passed `u8"..."` directly to a `const char*` API may therefore stop compiling after moving to C++20. That is an intentional type-system separation between ordinary narrow text and UTF-8 code units.

### `wchar_t` is a portability boundary, not a portable Unicode storage type

`wchar_t` is a distinct integer type, but its width and encoding model are implementation-defined. The most visible split is:

* Windows: typically 16-bit `wchar_t`, used with UTF-16-oriented APIs;
* many Unix-like systems: typically 32-bit `wchar_t`, often capable of holding a Unicode scalar value directly.

Consequently, serializing `wchar_t` values or exposing them in a binary protocol is almost always an ABI mistake. `wchar_t` is primarily a platform interface type.

### Only specific byte-like types have the arbitrary-object aliasing privilege

C++ gives `char`, `unsigned char`, and `std::byte` special permission to inspect an object's representation.

```cpp
std::uint32_t value = 0x12345678u;
auto p = reinterpret_cast<const unsigned char*>(&value);
```

Reading the object representation through `p` is allowed. The same rule does **not** generally apply to an arbitrary unrelated type, and notably the C++ wording does not grant the same special aliasing role to `signed char`.

This is one of the reasons low-level serializers, debuggers, hash implementations, and memory dumps use `unsigned char`, `char`, or `std::byte`.

`std::byte` is intentionally **not** an arithmetic integer type. It supports bitwise operations but rejects accidental arithmetic:

```cpp
std::byte b{0x0f};
b <<= 1;        // bit operation: fine
// ++b;         // error
// b + b;       // error
```

That makes `std::byte` a better semantic type for raw storage when the bytes are not meant to be numbers.

### Escape sequences have tokenization traps

The example deliberately shows a subtle lexical property: hexadecimal escapes consume hexadecimal digits for as long as they can.

```cpp
"\x41B"
```

This is not necessarily `"A" "B"`; the `B` is itself a hexadecimal digit and belongs to the escape. Octal escapes, by contrast, consume at most three octal digits.

A robust technique is to terminate a hex escape with string-literal concatenation:

```cpp
"\x41" "B"
```

Adjacent string literals are concatenated during translation.

### A particularly nasty library pitfall: `<cctype>` and negative `char`

The functions from `<cctype>` such as `std::isspace`, `std::toupper`, and `std::isdigit` accept an `int`, but their valid non-EOF inputs must be representable as `unsigned char`.

This is wrong on a platform where plain `char` is signed and `c` is negative:

```cpp
if (std::isspace(c)) { ... } // potentially undefined behavior
```

The safe idiom is:

```cpp
if (std::isspace(static_cast<unsigned char>(c))) { ... }
```

This bug survives code review because the promotion to `int` looks harmless. The problem is that the promoted value can be a negative integer that is neither `EOF` nor a valid unsigned-byte value.

**Engineering takeaway:** character types are not merely small integers. Decide whether a value represents text, a code unit, or raw storage, and choose the type accordingly.

---

## 2. Fundamental integer types, data models, ABI, and endianness

C++ intentionally specifies the fundamental integer types by **minimum range and relative ordering**, not by a universal bit width.

### Minimum widths are guaranteed; exact widths are not

The standard integer types are:

```cpp
signed char
short
int
long
long long
```

plus their unsigned counterparts. Their minimum widths are conventionally summarized as:

| Type          | Minimum width |
|---------------|--------------:|
| `signed char` |        8 bits |
| `short`       |       16 bits |
| `int`         |       16 bits |
| `long`        |       32 bits |
| `long long`   |       64 bits |

The size ordering is guaranteed:

```cpp
1 == sizeof(char)
sizeof(char) <= sizeof(short)
sizeof(short) <= sizeof(int)
sizeof(int) <= sizeof(long)
sizeof(long) <= sizeof(long long)
```

Equal sizes are permitted. Rank, width, and size are related concepts but are not interchangeable.

`long long` became a standard C++ type in C++11. It is at least 64 bits wide, but that is not a promise that every platform's preferred arithmetic width is 64 bits.

### Modern C++ guarantees two's-complement signed integers

Before C++20, the standard allowed signed magnitude, one's complement, and two's complement representations. C++20 standardized two's complement for the ordinary signed integer types.

That removes several historical representational oddities. For an N-bit signed type the familiar range is now:

```text
-2^(N-1) ... 2^(N-1)-1
```

But this does **not** make signed overflow wraparound arithmetic. Representation and arithmetic semantics are separate rules:

```cpp
int x = INT_MAX;
++x; // still undefined behavior
```

An optimizer may still assume that a well-defined execution never performs signed arithmetic overflow.

### Data models explain why `long` is a portability trap

The common data models are more useful than vague statements such as "`long` is 32 bits":

| Model | `int` | `long` | pointer | Typical environment          |
|-------|------:|-------:|--------:|------------------------------|
| ILP32 |    32 |     32 |      32 | 32-bit Unix/Windows          |
| LP64  |    32 |     64 |      64 | 64-bit Linux/macOS/most Unix |
| LLP64 |    32 |     32 |      64 | 64-bit Windows               |
| ILP64 |    64 |     64 |      64 | rare specialized systems     |

The practical consequence is that `long` is **64-bit on ordinary 64-bit Unix-like systems but remains 32-bit on Win64**.

That matters in:

* FFI boundaries;
* binary file layouts;
* `printf`/`scanf` format strings;
* RPC protocols;
* plugin ABIs;
* structures shared between processes;
* assumptions copied from Linux code into Windows code.

Changing a public function parameter from `int` to `long` can be an ABI change even if both happen to be 32 bits on the current build.

### `size_t` and pointer width are related, but not synonymous

`std::size_t` is an unsigned integer type capable of representing the size in bytes of any object. It is the type returned by `sizeof`.

```cpp
auto n = sizeof(object); // type: std::size_t
```

A pointer also has an implementation-defined representation. On mainstream flat-address-space ABIs, `sizeof(size_t)` and `sizeof(void*)` usually match, but C++ does not define `size_t` as "the pointer integer type." Capability machines, segmented systems, and other non-flat architectures are reasons not to encode that assumption into generic code.

For pointer differences, the standard library exposes the signed `std::ptrdiff_t`.

### Integer literals have types before they ever meet a variable

The type of integer literal depends on its **base, value, and suffix**.

For an unsuffixed decimal literal, the implementation tries, in order:

```text
int -> long -> long long
```

For an unsuffixed hexadecimal, octal, or binary literal, unsigned candidates are interleaved:

```text
int -> unsigned int -> long -> unsigned long -> long long -> unsigned long long
```

This means numerically equal literals can have different types:

```cpp
auto a = 2147483648;   // often long on LP64, long long on LLP64
auto b = 0x80000000;   // often unsigned int
```

That difference can propagate into overload resolution and mixed arithmetic.

C++23 also added the `z`/`Z` integer-literal suffix. An unsigned `uz` literal has type `std::size_t`; the signed form uses the signed counterpart of `size_t`.

```cpp
0uz   // std::size_t, C++23
```

For generic container/index code, that is often clearer than forcing a platform-dependent `unsigned long` or `unsigned long long` suffix.

### `-2147483648` is not a single negative literal token

The minus sign is an operator. The token is the positive literal `2147483648`, followed by unary `-`.

That matters at the edge of the signed range. The most portable spelling of a minimum value is not to hand-type the most-negative decimal literal; use the library constant:

```cpp
auto m = std::numeric_limits<int>::min();
```

This avoids relying on the positive token first fitting some wider candidate type.

### Endianness is a property of multibyte object representation

The example walks the bytes of an `unsigned long long` through an `unsigned char*`. That is a legitimate way to inspect the object representation.

On little-endian systems, the least-significant byte is stored at the lowest address. On big-endian systems, the most-significant byte is stored first.

Since C++20, `<bit>` exposes the implementation property explicitly:

```cpp
#include <bit>

if constexpr (std::endian::native == std::endian::little) {
    // ...
}
```

Since C++23, `std::byteswap` handles byte-order reversal for suitable integer types:

```cpp
auto network_order = std::byteswap(host_value);
```

These facilities are preferable to open-coded shifts when the intent is byte order rather than bit arithmetic.

### Do not use inactive union members as portable type punning

The example's union illustrates why little-endian machines can access the same address at different widths, but in standard C++ this construction is not a portable way to reinterpret an active `uint64_t` member as `uint32_t`, `uint16_t`, or `uint8_t` members.

```cpp
union U {
    std::uint8_t  u8;
    std::uint16_t u16;
    std::uint32_t u32;
    std::uint64_t u64;
};
```

Reading a non-active member is generally outside the portable C++ object model, even though compilers may support union punning as an extension.

Prefer one of these techniques depending on intent:

```cpp
// inspect bytes
std::array<std::byte, sizeof(value)> bytes;
std::memcpy(bytes.data(), &value, sizeof value);

// or, for same-size trivially copyable types in C++20
std::bit_cast<Destination>(source);
```

For extracting numeric subfields, explicit shifts and masks are often even clearer because they describe **numeric bit significance**, not machine byte order.

### Alignment is part of the type's ABI contract

`sizeof(T)` is only half the storage story. `alignof(T)` determines where objects of `T` may be placed.

```cpp
static_assert(alignof(std::uint64_t) >= 1);
```

Unaligned integer access is architecture-dependent: it can be natively supported, slower, split into multiple transactions, or trap. Packed structures therefore need special care. Merely writing:

```cpp
#pragma pack(push, 1)
```

can create members whose addresses are not suitably aligned for ordinary typed access on every target.

For wire formats, parsing from bytes with `memcpy`, `bit_cast`, shifts, or explicit load helpers is generally safer than mapping a packed network packet directly onto a C++ struct.

**Engineering takeaway:** fundamental integer names are ABI-level categories, not portable width declarations. Treat data model, endianness, alignment, and literal type as first-class design constraints at low-level boundaries.

---

## 3. Initialization syntax is part of integer safety

C++ has several initialization syntaxes that look interchangeable for simple integers but are not semantically identical in generic code.

```cpp
int a1{1};
int a2 = {1};
int a3 = 1;
int a4(1);
```

The differences become visible with narrowing, constructors, `std::initializer_list`, `auto`, and templates.

### Brace initialization rejects narrowing conversions

List initialization deliberately diagnoses many conversions that assignment-style initialization accepts:

```cpp
int a = 3.14;   // allowed: a becomes 3
int b{3.14};    // ill-formed: narrowing
```

For integer-to-integer conversion, list initialization has an important constant-expression exception. A constant value can be accepted if the target type can represent that specific value even when the source type's full range would not fit.

```cpp
unsigned char a{42};   // fine
// unsigned char b{300}; // error
```

This makes braces especially useful at boundaries where a value is being committed to a narrower representation.

### "Uniform initialization" is not actually uniform

Braces solve some problems and create another: constructors taking `std::initializer_list` receive special preference.

The example's `std::vector` distinction is the canonical case:

```cpp
std::vector<int> a{99}; // one element: 99
std::vector<int> b(99); // 99 zero-initialized elements
```

A more dangerous version is:

```cpp
std::vector<int> a{10, 20}; // elements 10 and 20
std::vector<int> b(10, 20); // ten elements, each 20
```

So braces are not a universal style replacement. They carry list-initialization semantics.

### `auto` plus braces changed historically

Modern C++ distinguishes direct-list initialization from copy-list initialization:

```cpp
auto a{99};    // int under the post-N3922 rules
auto b = {99}; // std::initializer_list<int>
```

Before the N3922 rule change, `auto a{99}` was deduced as `std::initializer_list<int>`. The corrected rule was incorporated into the C++17-era language and implemented by major compilers earlier as a defect-resolution behavior.

For experienced developers maintaining code across old toolchains, this is a reminder that some "language-version" differences arrive through defect reports and compiler backports rather than a clean `-std=` boundary.

Direct-list `auto` must have one element:

```cpp
auto x{1, 2}; // ill-formed
```

while copy-list deduction requires a common element type:

```cpp
auto x = {1, 2};     // initializer_list<int>
// auto y = {1, 2.0}; // cannot deduce one element type
```

### `auto` preserves less type information than many people assume

Top-level cv-qualification and references are usually dropped by ordinary `auto` deduction unless explicitly requested:

```cpp
const int n = 42;
auto a = n;        // int
auto& b = n;       // const int&
const auto& c = n; // const int&
```

This can matter for integer-like proxy types, atomics, bit-field interactions, and custom numeric classes.

`decltype(auto)` follows `decltype` rules instead and can preserve references:

```cpp
int x = 0;
int& ref = x;
auto a = ref;           // int
decltype(auto) b = ref; // int&
```

### `{}` value-initialization is a useful zeroing primitive

For scalar integer objects:

```cpp
int x{};
std::uint64_t y{};
```

produces zero. This is particularly useful for local variables and aggregates because it avoids the uninitialized-scalar default:

```cpp
int x; // indeterminate value if not otherwise initialized
```

Do not generalize this into "braces always mean zero," however. The result depends on the initialized type and its constructors.

### Narrowing checks do not replace runtime range checks

List initialization is primarily a compile-time mechanism. If the source is a runtime integer value, the conversion may still need an explicit check:

```cpp
std::uint32_t x = read_from_file();
// std::uint8_t y{x}; // narrowing: ill-formed regardless of runtime value
```

The compiler cannot accept this simply because the current execution might happen to produce 42. The correct pattern is to prove the range, then cast:

```cpp
if (x <= std::numeric_limits<std::uint8_t>::max()) {
    auto y = static_cast<std::uint8_t>(x);
}
```

C++20's `std::in_range` makes generic checked conversions easier and avoids signed/unsigned mistakes in the range test.

**Engineering takeaway:** initialization syntax is not cosmetic. Braces are a compile-time narrowing firewall, parentheses select different constructors, and `auto` has its own list-deduction rules.

---

## 4. Unsigned integers: modular arithmetic, not "non-negative integers"

Unsigned types are often described as integers that cannot be negative. That description is mathematically incomplete and encourages bad API design. A better model is:

> An N-bit unsigned integer implements arithmetic modulo `2^N`.

That is a different algebra from the ordinary integers.

### Wraparound is guaranteed

For an N-bit unsigned type:

```cpp
0u - 1u == UINT_MAX
UINT_MAX + 1u == 0u
```

This is defined behavior, not an overflow accident.

That property is useful for:

* bit masks;
* hash functions;
* checksums;
* sequence numbers with wraparound;
* cryptographic primitives when used carefully;
* hardware registers;
* modular arithmetic by design.

It is dangerous when the programmer only meant "this quantity should never be negative."

### Unsigned subtraction can turn a simple predicate into a huge value

The example contains the pattern:

```cpp
if (i - j >= 4) {
    ...
}
```

If both operands are unsigned and `i < j`, the subtraction wraps. For 32-bit `unsigned`, `1u - 2u` becomes `4294967295u`, so the condition is unexpectedly true.

The intended predicate is normally written without subtraction:

```cpp
if (i >= j && i - j >= 4) {
    ...
}
```

or, when algebra permits:

```cpp
if (i >= j + 4) {
    ...
}
```

The second form itself requires care if `j + 4` can wrap. Range reasoning cannot be avoided just by changing the syntax.

### Mixed signed/unsigned comparison invokes the usual arithmetic conversions

Consider:

```cpp
unsigned i = 1;
int j = -1;

bool b = j < i;
```

On an ordinary implementation where both have the same rank, `j` is converted to `unsigned int`. The value `-1` becomes `UINT_MAX`, so the comparison is false.

The surprising part is not "unsigned wins" as a universal rule. The actual usual arithmetic conversion rules consider:

1. integral promotions;
2. signedness;
3. conversion rank;
4. whether the signed type can represent all values of the unsigned type.

For example, if a signed type has higher rank **and** can represent every value of the unsigned operand's type, the unsigned value can be converted to the signed type instead.

C++20 provides comparison helpers that express mathematical intent without value-changing signedness conversions:

```cpp
#include <utility>

std::cmp_less(-1, 1u); // true
std::cmp_equal(-1, UINT_MAX); // false
```

These are valuable in generic code where operand signedness is not obvious at the call site.

### `size_t` makes signed/unsigned interaction unavoidable

Standard containers use unsigned `size_type` in their core interface:

```cpp
v.size(); // usually std::size_t
```

So even code that prefers signed arithmetic will meet unsigned values at library boundaries.

A common bad pattern is:

```cpp
for (int i = 0; i < v.size(); ++i) { ... }
```

It mixes signed and unsigned. Better choices include:

```cpp
for (std::size_t i = 0; i < v.size(); ++i) { ... }
```

or, if signed indexing is genuinely useful:

```cpp
for (auto i = 0; i < std::ssize(v); ++i) { ... } // C++20
```

or best of all when the index is not semantically needed:

```cpp
for (auto& element : v) { ... }
```

### Reverse loops are where unsigned bugs become folklore

This never terminates:

```cpp
for (std::size_t i = n - 1; i >= 0; --i) {
    ...
}
```

The condition `i >= 0` is always true for an unsigned type.

A compact unsigned reverse-iteration idiom is:

```cpp
for (std::size_t i = n; i-- > 0;) {
    // body sees n-1, n-2, ..., 0
}
```

The post-decrement is intentional: the comparison uses the old value, then `i` is decremented before the loop body executes.

For readability, ranges/reverse iterators are often preferable to clever index syntax.

### Shift width comes from the promoted left operand

This is the central point of the example's `show_shift()` function:

```cpp
long long x = 1075 << i;
```

The destination type does not control the shift. `1075` is an `int` literal, so the left operand is an `int` and the shift produces an `int`. Only afterward is the result converted to `long long`.

If 64-bit shifting is intended, type the operand accordingly:

```cpp
1075LL << i
1075ULL << i
```

This principle generalizes far beyond shifts:

> Assignment conversion happens after the right-hand expression has already been evaluated in the types chosen by the expression rules.

Writing a wider destination cannot rescue an operation that already overflowed, wrapped, or invoked undefined behavior in a narrower intermediate type.

### Shift rules are version-sensitive

For all standard versions, a negative shift count or a count greater than or equal to the width of the promoted left operand is undefined behavior.

```cpp
1u << 32 // UB if unsigned is 32 bits
```

Signed-shift semantics are historically subtler. Pre-C++20 left shift of a signed value was constrained by representability rules, and right shift of a negative value was implementation-defined. Since C++20, the shift specification is substantially more two's-complement-oriented; right shift of a negative signed value is arithmetic (rounding toward negative infinity), and left-shift result semantics are specified modulo the result width. Invalid shift counts remain undefined.

This is worth calling out because comments copied from older C++ references can be overly conservative or simply wrong under modern language modes.

### Integer promotion also affects bitwise NOT

A classic trap:

```cpp
std::uint8_t x = 0x0f;
auto y = ~x;
```

If `uint8_t` is an alias for `unsigned char`, `x` is first promoted to `int`; `~x` is therefore an `int`, not an 8-bit value. On a 32-bit `int`, `y` commonly becomes `0xfffffff0` rather than `0xf0`.

If eight-bit semantics are intended, make the truncation explicit:

```cpp
auto y = static_cast<std::uint8_t>(~x);
```

### Use unsigned for modular semantics, not merely domain validation

An unsigned type does not enforce "must be non-negative" at an API boundary:

```cpp
void set_count(unsigned n);
set_count(-1); // well-formed conversion; likely a huge value
```

If negative input is a contract violation, validation or a stronger domain type is needed. Merely changing `int` to `unsigned` can make mistakes harder to detect.

**Engineering takeaway:** use unsigned when modulo-`2^N` behavior, bit-level interpretation, or an ABI requires it. Do not assume unsigned automatically makes a quantity safer just because its conceptual domain is non-negative.

---

## 5. Integral promotions: the invisible rewrite before arithmetic

Integral promotions are among the most important "invisible" rules in C++. They happen before many operators, and because they preserve the numeric value they are ranked better than ordinary conversions during overload resolution.

### Promotion is not synonymous with widening

A promotion is a specific language category. For ordinary narrow integer types, the destination is selected by rule, not simply by "next larger type."

For `char`, `signed char`, `unsigned char`, `short`, and `unsigned short`, the basic rule is:

* promote to `int` if `int` can represent every value of the source type;
* otherwise promote to `unsigned int`.

On mainstream systems this means even `unsigned char` and usually `unsigned short` promote to **signed `int`**:

```cpp
unsigned char uc = 255;
auto x = +uc;
static_assert(std::is_same_v<decltype(x), int>); // typical 32-bit-int ABI
```

This corrects a common misconception that "unsigned small types promote to unsigned int." They often do not.

The distinction becomes observable in expressions:

```cpp
unsigned char a = 200;
unsigned char b = 200;
auto c = a + b; // usually int 400
```

No 8-bit overflow occurs during the addition.

### Promotion preserves value; ordinary conversion need not

That preservation guarantee is why overload resolution prefers a promotion:

```cpp
void f(int);
void f(short);

char c = 0;
f(c); // f(int): promotion beats char -> short conversion
```

This can matter when seemingly harmless overloads are added to an established API. The "closest width" overload is not necessarily the best conversion sequence.

### Unary operators reveal promotions cleanly

Unary `+` is sometimes useful when teaching or debugging because it forces integral promotion without otherwise changing the value:

```cpp
unsigned char x = 200;
auto promoted = +x;
```

Likewise `~x`, unary `-x`, and many binary arithmetic/bitwise operators operate after promotion.

For low-level code, a `static_assert` on the expression type can be more informative than staring at declarations:

```cpp
static_assert(std::is_same_v<decltype(+x), int>);
```

### `char16_t`, `char32_t`, and `wchar_t` have special promotion rules

The Unicode character types do not simply follow the ordinary `char`/`short` rule. They are promoted to the first type in an ordered list that can represent their full range:

```text
int
unsigned int
long
unsigned long
long long
unsigned long long
```

On a typical system, `char32_t` can represent values up to `0xffffffff`, so it commonly promotes to `unsigned int`, not `int`.

Therefore, code such as:

```cpp
char32_t c = U'🍌';
int x = c;
```

should not be mentally modeled as "`char32_t` promotes to `int`." The expression may promote to `unsigned int`, after which the initialization of `x` performs a separate integral conversion.

### Enums are another promotion boundary

Unscoped enumerations participate in integral promotion. For an unscoped enum with a fixed underlying type, conversion to that underlying type is privileged in the promotion rules.

```cpp
enum Mode : unsigned short { A, B };
```

Scoped enumerations (`enum class`) do not implicitly convert to integers:

```cpp
enum class Mode : unsigned short { A, B };
// int x = Mode::A; // error
```

C++23's `std::to_underlying` is the explicit, intention-revealing bridge:

```cpp
auto raw = std::to_underlying(Mode::A);
```

### Bit-fields have their own promotion corner cases

Bit-fields are one of the places where "declared type" and "expression type" diverge most sharply.

```cpp
struct Flags {
    unsigned ready : 1;
};
```

The lvalue-to-rvalue conversion and integral promotion rules can promote a bit-field to `int` when its range fits. This affects overloads and arithmetic even though the bit-field was declared with an unsigned type.

Code that uses bit-fields as hardware-register or protocol abstractions should therefore avoid assuming that arithmetic preserves the apparent field type.

### The usual arithmetic conversions happen after promotions

For a binary integer operator such as `+`, `<`, `&`, or `%`, a useful mental pipeline is:

```text
lvalue-to-rvalue
    -> integral promotions
    -> find common arithmetic type
    -> convert operands to that type
    -> perform operation
```

This explains many "impossible" bugs. The code you wrote might show `short` and `unsigned short`, but the CPU-level arithmetic selected by the language may be `int`, `unsigned int`, `long`, or another common type.

**Engineering takeaway:** when an integer expression surprises you, inspect the promoted operand types before inspecting the destination variable. The destination is usually too late to explain what happened.

---

## 6. Numeric conversions: where values actually change

Promotions are value-preserving. Numeric conversions are where truncation, modulo reduction, precision loss, and undefined behavior enter the picture.

For modern C++, it is useful to separate five categories:

1. integer to integer;
2. signedness changes;
3. floating point to integer;
4. integer to floating point;
5. conversions to `bool` or enum types.

### Integer-to-integer conversion is defined modulo the destination width in C++20+

For an unsigned destination, the rule has long been modulo `2^N`:

```cpp
std::uint32_t x = 65535;
std::uint8_t y = static_cast<std::uint8_t>(x); // 255, if uint8_t exists
```

For a signed destination, historical C++ used implementation-defined behavior when the source value was not representable. **Since C++20**, the result is the unique destination value congruent to the source modulo `2^N`, where `N` is the bit width used by the destination type for this rule.

That means comments claiming that an out-of-range `uint32_t -> int32_t` conversion is necessarily implementation-defined are correct for older language rules but outdated for C++20 and later.

Do not confuse this with signed arithmetic overflow:

```cpp
std::int32_t x = INT32_MAX;
++x; // UB: arithmetic overflow
```

Conversion and arithmetic have different rules.

### Narrowing can be well-defined and still be a bug

This conversion is perfectly defined:

```cpp
std::uint32_t x = 0x12345678u;
auto y = static_cast<std::uint8_t>(x); // low 8 bits: 0x78
```

The fact that behavior is defined says nothing about whether it was intended.

A good code-review question is therefore not merely "is this UB?" but:

> Is this conversion value-preserving for all inputs allowed by the surrounding contract?

If not, the cast is a semantic boundary and deserves a visible range check, assertion, saturating policy, or explicit truncation comment.

### C++20 gives generic code `std::in_range`

Manual checks are easy to get wrong when source and destination have different signedness:

```cpp
if (x >= 0 && x <= std::numeric_limits<unsigned>::max()) { ... }
```

If `x` is itself unsigned, the first test is tautological. If a limit expression is converted to the wrong common type, the upper test can also become wrong.

C++20 provides:

```cpp
if (std::in_range<std::uint8_t>(x)) {
    auto y = static_cast<std::uint8_t>(x);
}
```

The integer comparison helpers `std::cmp_less`, `std::cmp_greater`, and friends solve the related comparison problem without value-changing signed/unsigned conversion.

### `uint8_t` has an API/IO identity crisis

On implementations that provide `std::uint8_t`, it is typically a typedef of `unsigned char`, because C++ does not have a separate built-in "exactly 8-bit arithmetic type."

That means overload resolution treats it as a character type:

```cpp
std::uint8_t x = 65;
std::cout << x; // may print 'A', not 65
```

For numeric output, promote or cast:

```cpp
std::cout << +x;
std::cout << static_cast<unsigned>(x);
```

This is not an iostream bug. A typedef does not create a new type.

The same issue can affect formatting APIs, serialization overloads, template specialization, and functions overloaded on `unsigned char`.

### Floating-to-integer conversion is much more dangerous than unsigned narrowing

Floating-to-integer conversion truncates toward zero:

```cpp
static_cast<int>( 1.9) ==  1
static_cast<int>(-1.9) == -1
```

But if the truncated value cannot be represented by the destination integer type, behavior is undefined. Modulo arithmetic does **not** rescue an unsigned destination.

NaN is not a valid in-range integer value either.

So this is dangerous:

```cpp
int x = static_cast<int>(external_double);
```

unless the range and finiteness are already established.

A robust boundary usually checks:

* `std::isfinite(x)`;
* lower bound;
* upper bound;
* the desired rounding policy (`trunc`, `floor`, `ceil`, nearest, etc.).

### Integer-to-floating conversion can lose integer identity

IEEE-754 binary64 (`double` on mainstream systems) has 53 bits of significand precision. Therefore, every integer up to `2^53` is exactly representable, but not every integer above it is.

```cpp
std::uint64_t a = (1ULL << 53);
std::uint64_t b = a + 1;

double da = static_cast<double>(a);
double db = static_cast<double>(b);

// da and db may be equal
```

This matters in systems that use `double` as a generic number carrier -- for example JSON stacks, scripting bridges, metrics systems, and JavaScript-facing APIs. A 64-bit database identifier can silently stop being injective when represented as binary64.

The maximum exactly representable *consecutive* integer is a precision property, not a range property. `double` can represent vastly larger magnitudes, but with gaps between representable integers.

### Conversion to `bool` collapses almost the entire numeric domain

For arithmetic values:

```cpp
0      -> false
nonzero -> true
```

This includes negative values and floating NaN: NaN is not equal to zero, so conversion to `bool` produces `true`.

That makes code such as this semantically very different from integer conversion:

```cpp
bool b = 0.5; // true
int  i = 0.5; // 0
```

### Converting arbitrary integers to enums requires a semantic validity check

A scoped enum with a fixed underlying type has a well-defined storage domain that is often much larger than its listed enumerators:

```cpp
enum class Color : std::uint8_t {
    Red = 1,
    Green = 2,
    Blue = 3
};

Color c = static_cast<Color>(250);
```

Whether a particular integer-to-enum conversion is well-defined depends on the enum's representable range and the language rules, but even a representable result may be **semantically invalid** because no enumerator has that value.

Therefore, deserialization needs two checks:

1. can the raw value be represented in the underlying type / enum domain?
2. is it one of the values accepted by the protocol or application?

Casts solve neither policy question.

### Do not "fix" average overflow with a formula that changes rounding accidentally

A classic overflow-safe average trick is:

```cpp
a / 2 + b / 2
```

but for integers it can lose the remainder twice:

```cpp
1 / 2 + 1 / 2 == 0
(1 + 1) / 2     == 1
```

For unsigned integers, a better algebraic identity is often:

```cpp
a + (b - a) / 2
```

provided ordering/preconditions are appropriate. Since C++20, `std::midpoint` is usually the correct standard facility for midpoint calculation because it is designed to avoid overflow.

This is a useful lesson from the conversion example: avoiding overflow is not enough; the replacement formula must preserve the required rounding semantics.

### Checked conversion should be a reusable policy

Large codebases benefit from naming the boundary instead of scattering casts:

```cpp
template <std::integral To, std::integral From>
To checked_cast(From value)
{
    if (!std::in_range<To>(value))
        throw std::range_error("integer conversion out of range");
    return static_cast<To>(value);
}
```

Real systems may prefer `std::expected`, assertions, error codes, saturating conversion, or domain-specific validation. The important property is that **range loss is a policy decision rather than an incidental side effect of assignment**.

**Engineering takeaway:** explicit casts make conversions visible; they do not make them safe. Range preservation and semantic validity still need to be proved.

---

## 7. `<cstdint>`: exact width, the least width, fast width, and pointer-sized integers

`<cstdint>` is often taught as "the header that gives us `int32_t`." Its actual design is more nuanced: it provides several different families because "exactly N bits," "at least N bits," "smallest such type," and "fastest such type" are different requirements.

### `intN_t` and `uintN_t` are optional exact-width types

If `std::int32_t` exists, it is exactly 32 bits wide with no padding bits. Likewise for `std::uint32_t`.

Crucially, an implementation that has no exact 32-bit integer type does **not** substitute a 36-bit or 64-bit type and call it `int32_t`. The typedef is simply not provided.

That corrects a common misunderstanding.

Exact-width types are excellent for:

* binary protocol fields;
* file formats;
* cryptographic algorithms;
* bit-exact hardware interfaces;
* hashes and checksums;
* serialization schemas whose widths are externally specified.

They are not automatically the best choice for every loop counter or local arithmetic temporary.

### Exact-width types also imply strong representation properties

The exact-width typedefs require no padding bits. Their existence therefore tells you more than just `sizeof(T) * CHAR_BIT`.

For example, if `std::uint32_t` exists, it has exactly 32 value bits. If an implementation cannot provide those semantics, the name is absent.

This matters on exotic word-addressed or non-octet-byte systems where a familiar `sizeof(std::uint32_t) == 4` model may not make sense.

### `int_leastN_t` answers a different question

`std::int_least32_t` means:

> the smallest available signed integer type with at least 32 bits of width.

It is the portability choice when a minimum range matters but an exact object representation does not.

If an exact `int32_t` exists, `int_least32_t` normally names that type. If the smallest available type is wider, `int_least32_t` may be wider.

This family is especially relevant to genuinely unusual targets rather than mainstream desktop/server code, but it precisely expresses the language requirement.

### `int_fastN_t` is an implementation choice, not a microbenchmark promise

`std::int_fast32_t` means an implementation-selected integer type of at least 32 bits intended to be fast.

It may be 64 bits. On some 64-bit ABIs, `int_fast16_t` and `int_fast32_t` have historically been aliases of a 64-bit `long` because register arithmetic favored native word width.

This creates a non-obvious tradeoff:

* arithmetic on a wider type may be convenient or fast in registers;
* arrays of that type use more memory;
* larger arrays increase cache footprint and memory bandwidth;
* ABI choices may preserve an old definition even after CPU performance characteristics evolve.

So "fast" does not mean "provably fastest for your workload in 2026." It means the implementation selected a type for that role.

For dense arrays, data-oriented code often prefers exact or least-width storage and promotes to a convenient arithmetic type when loading values.

### `uint8_t` may be a character type because there is no distinct built-in byte-sized integer

If an implementation's 8-bit exact unsigned type is `unsigned char`, then:

```cpp
using std::uint8_t = unsigned char; // conceptually
```

A typedef preserves type identity. This is why `uint8_t` can choose character overloads and why `std::cout << uint8_t{65}` may print `A`.

If you need a strong semantic "8-bit number" type, a typedef cannot provide it; use a wrapper type or another stronger abstraction.

### `intptr_t` and `uintptr_t` are for pointer round-trips, not pointer arithmetic

When provided, `std::intptr_t` and `std::uintptr_t` are integer types capable of representing converted `void*` values for the intended round-trip semantics of the implementation.

They are useful in low-level interfaces, diagnostics, tagging schemes on supported platforms, and APIs that explicitly traffic in addresses as integers.

They are not substitutes for pointers:

```cpp
std::uintptr_t address;
```

has no provenance, lifetime, bounds, or pointee type in the C++ type system. On capability architectures, these distinctions become especially important.

For ordinary pointer arithmetic, keep the value as a pointer or use `std::ptrdiff_t` for differences.

### `intmax_t` is about the implementation's integer type universe, not necessarily performance

`std::intmax_t` and `std::uintmax_t` are maximum-width standard-library integer types intended to represent very wide integer values available through the standard integer model.

They are useful for generic formatting/parsing bridges and macros, but they are often a poor choice for hot data structures: "widest" generally means more storage and potentially different calling conventions.

### Literal macros solve a pre-type problem

Macros such as `INT64_C` and `UINT64_C` exist because the correct literal suffix for an exact-width typedef can vary by data model.

For example, a 64-bit integer might be `long` on LP64 and `long long` on LLP64. The macro selects an appropriate constant type without forcing the source to guess the ABI suffix.

```cpp
auto x = UINT64_C(0xdeadbeef);
```

In modern generic C++ these macros are less visible than in C interoperability code, but the underlying issue remains real: **literal type is chosen before assignment**.

### C++26 adds width macros

C++26 adds macros such as `INT32_WIDTH`, `INT_FAST32_WIDTH`, `INT_LEAST32_WIDTH`, `INTMAX_WIDTH`, and their unsigned counterparts where the corresponding types exist.

These report the bit width directly and are preferable to reverse-engineering it from `sizeof(T) * CHAR_BIT` when padding/value-bit distinctions matter.

For code targeting older standards, `std::numeric_limits<T>::digits` remains important:

```cpp
std::numeric_limits<std::uint32_t>::digits // 32
std::numeric_limits<std::int32_t>::digits  // 31 sign-excluding value bits
```

### Pick integer types by contract

A useful decision table is:

| Requirement                                        | Typical choice                              |
|----------------------------------------------------|---------------------------------------------|
| Natural arithmetic/local counter                   | `int`, sometimes `long long`                |
| Container size/index API                           | container `size_type` / `std::size_t`       |
| Signed index/difference                            | `std::ptrdiff_t`, `std::ssize` result       |
| Exactly N-bit wire/storage field                   | `std::uintN_t` / `std::intN_t` if available |
| At least N bits, compact                           | `std::uint_leastN_t`                        |
| At least N bits, implementation-selected fast type | `std::uint_fastN_t`                         |
| Integer representation of pointer when supported   | `std::uintptr_t` / `std::intptr_t`          |
| Raw storage bytes                                  | `std::byte` / `unsigned char`               |

The goal is not to replace every `int` with `int32_t`. The goal is to make the **contract** explicit where width or representation is part of correctness.

**Engineering takeaway:** `<cstdint>` offers families because there is no single notion of "the right fixed integer." Exact width, minimum width, speed, storage density, pointer representation, and ABI stability are different design axes.

---

## 8. Random integers: the range mapping is part of the algorithm

Random integer generation is a useful final example because it combines many of the chapter's themes:

* integer range and overflow;
* modulo arithmetic;
* exact result domains;
* reproducibility across implementations;
* implicit assumptions about the bit width;
* performance tradeoffs that are not obvious from syntax.

### `rand()` is a legacy compatibility API, not a modern C++ RNG abstraction

`std::rand()` exposes global hidden state and returns an integer in `[0, RAND_MAX]`. The standard gives only weak quality guarantees.

The common idiom:

```cpp
lo + std::rand() % (hi - lo + 1)
```

has several independent problems:

1. the range expression can overflow;
2. `%` can introduce modulo bias;
3. `RAND_MAX` may expose relatively few random bits;
4. generator state is global;
5. the generator quality is implementation-dependent;
6. it is unsuitable for cryptographic security.

### Modulo bias is an integer-divisibility problem

Suppose the generator has `M` equally likely output values and we want `N` result buckets. If `M` is not divisible by `N`, then `r % N` cannot distribute all buckets equally.

For example, if a generator produced ten equally likely values `0..9` and we mapped with `% 4`:

```text
0 -> 0
1 -> 1
2 -> 2
3 -> 3
4 -> 0
5 -> 1
6 -> 2
7 -> 3
8 -> 0
9 -> 1
```

Buckets 0 and 1 occur three times; 2 and 3 occur twice.

For ordinary application code the bias may be small, but in Monte Carlo work, randomized algorithms, shuffling, load balancing, or security-sensitive contexts it can be unacceptable.

### Rejection sampling removes modulo bias

The example implements the right basic idea: discard generator outputs from the incomplete tail so that the accepted output count is an exact multiple of the target range.

A subtle implementation detail is that the source domain has `RAND_MAX + 1` values, not `RAND_MAX` values. Expressions involving that quantity must avoid overflow when `RAND_MAX` is itself the maximum value of the chosen unsigned type.

This is a good example of why seemingly "statistical" code still needs precise integer reasoning.

### `<random>` separates engines from distributions

C++11's random library deliberately splits two responsibilities:

```text
engine       -> pseudo-random bits / integer sequence
distribution -> map those bits to a statistical distribution
```

For example:

```cpp
std::mt19937 engine(seed);
std::uniform_int_distribution<int> dist(0, 9);

auto x = dist(engine);
```

`std::uniform_int_distribution` produces values on the inclusive interval `[a, b]` and performs appropriate range mapping rather than asking the user to write `%` arithmetic.

This separation is one of the strongest design ideas in `<random>`: choose the engine for state size, speed, period, reproducibility, and statistical properties; choose the distribution for the desired output model.

### `mt19937` is deterministic and standardized, but not cryptographic

`std::mt19937` is the 32-bit Mersenne Twister engine with a very large period. Given the same engine state, its generated engine sequence is specified.

But it is **not** suitable for cryptographic secrets. Its state is large but recoverable from sufficient output, and it was designed for simulation-quality pseudorandomness, not adversarial unpredictability.

Use platform cryptographic APIs or a vetted cryptographic library for keys, tokens, password-reset secrets, nonces with security requirements, and similar tasks.

### Reproducible engine output is not the same as reproducible distribution output

This is an advanced but important portability trap.

A standard engine such as `std::mt19937` has specified behavior. But the standard does not require every implementation of `std::uniform_int_distribution` to use the same mapping algorithm internally.

Therefore, this can be reproducible on one standard library and produce a different integer sequence on another:

```cpp
std::mt19937 eng(12345);
std::uniform_int_distribution<int> dist(1, 6);
```

If a test, simulation snapshot, replay file, or distributed protocol requires **bit-for-bit cross-library reproducibility**, pinning only the engine and seed may be insufficient. You need to specify the mapping algorithm too -- or store/replay the generated data.

### Seeding and state size are different concepts

A single 32-bit seed can initialize `mt19937`, but the engine's internal state is much larger. A single seed therefore reaches only a tiny subset of possible internal states.

`std::seed_seq` exists to diffuse a sequence of seed words into an engine state:

```cpp
std::seed_seq seq{a, b, c, d};
std::mt19937 eng(seq);
```

Using several values from `std::random_device` can provide more seed entropy than a single call, but `seed_seq` cannot create entropy that was not present in its inputs.

For deterministic tests, the correct seed is usually a literal constant recorded in source or test data. Reproducibility is a feature, not a weakness, in testing.

### `std::random_device` is not guaranteed to be a hardware entropy source

`std::random_device` is intended to provide non-deterministic random numbers when the implementation has such a facility, but the standard permits a deterministic pseudo-random fallback when it does not.

It is normally used to seed a PRNG rather than as the high-throughput generator itself.

This means code should not infer "cryptographically secure" merely from the class name `random_device`.

### Time-based seeding is often worse than it looks

Legacy code often uses:

```cpp
std::srand(std::time(nullptr));
```

Processes starting in the same second can receive the same seed. Higher-resolution clocks reduce obvious collisions but do not turn timestamps into entropy.

Time-based seeds are predictable, which is unacceptable for security and sometimes undesirable even for simulations launched in parallel.

Use a fixed seed when reproducibility is wanted and an appropriate entropy source when unpredictability is wanted. Do not blur those two goals.

### Per-thread engines avoid shared mutable RNG state

A generator object has mutable state. Sharing one engine across threads without synchronization is a data race; sharing with a lock can become a scalability bottleneck.

A common design is one engine per worker/thread, seeded in a controlled way:

```cpp
thread_local std::mt19937 engine = make_engine_for_this_thread();
```

The hard part is then seed management: naively giving every thread adjacent seeds may or may not meet the statistical independence requirements of the chosen engine and application.

For serious simulation work, stream splitting/jump-ahead facilities or generators designed for parallel streams may be preferable to blindly cloning Mersenne Twister instances.

### Benchmarking RNGs requires separating generation from distribution

The example compares `rand()`, corrected `rand()` range mapping, and `mt19937 + uniform_int_distribution`. Such a benchmark is educational, but the result is not a universal ranking.

The measured cost combines:

* engine state update;
* distribution/range reduction;
* rejection probability;
* inlining;
* standard-library implementation;
* compiler optimization;
* memory/state footprint;
* contention if global/shared state is involved.

If RNG performance matters, benchmark the exact workload and distinguish:

```text
engine only
engine + bounded mapping
distribution object construction
hot repeated distribution calls
multi-threaded contention/state locality
```

A generator with excellent scalar throughput may also have a large state that hurts cache behavior when thousands of engines are stored.

### Be explicit about interval semantics

`std::uniform_int_distribution<int>(lo, hi)` is **inclusive at both ends**. Many APIs in modern C++ use half-open ranges `[first, last)`, so this difference is easy to misremember.

A half-open random index for a container of size `n` is normally:

```cpp
std::uniform_int_distribution<std::size_t> dist(0, n - 1);
```

which of course requires `n != 0`.

For generic code, the empty case and the possible width of `n - 1` should be handled before constructing the distribution.

**Engineering takeaway:** random integer generation is not "take random bits and apply `%`." Generator state, range mapping, seeding, reproducibility, statistical quality, concurrency, and security are separate concerns.

---

## Rules worth keeping in working memory

The eight examples point to a small set of rules that explain a surprisingly large fraction of integer bugs in production C++.

### 1. Expression type matters more than the destination type

```cpp
long long x = 1 << 40;
```

The left shift is selected and evaluated before conversion to `long long`. A wide destination does not widen earlier intermediate arithmetic.

Type the operands, not merely the result.

### 2. Promotions happen before usual arithmetic conversions

Small integer types frequently disappear from the expression before the operator executes.

```cpp
std::uint8_t a = 200, b = 100;
auto x = a + b; // usually int
```

### 3. Signed overflow and signed conversion are different rules

```cpp
int x = INT_MAX;
++x; // undefined arithmetic overflow
```

is not governed by the same rule as converting an out-of-range unsigned integer to a signed integer. Since C++20, the latter uses modulo-congruence semantics.

### 4. Unsigned means modulo arithmetic

Use it deliberately. It is not a runtime validator for "must be >= 0."

### 5. Exact-width typedefs are conditional

`std::uint32_t` means exactly 32 bits if it exists. It is not "the next type at least 32 bits." That job belongs to the `least` and `fast` families.

### 6. A typedef does not make a new type

If `uint8_t` aliases `unsigned char`, overload resolution and I/O see `unsigned char`.

### 7. Binary representation is not a portable object schema

Do not serialize native structs by dumping their bytes unless the ABI, padding, endianness, alignment, type widths, and versioning are all explicitly part of the format.

### 8. "Defined behavior" is weaker than "correct behavior"

Unsigned wraparound and narrowing integer conversion can be perfectly defined and completely wrong for the application.

### 9. Prefer library facilities that encode the difficult rule

Modern C++ contains useful integer-safety tools:

```cpp
std::numeric_limits<T>
std::cmp_less / std::cmp_equal       // C++20
std::in_range<T>                     // C++20
std::midpoint                        // C++20
std::endian                          // C++20
std::bit_cast                        // C++20
std::ssize                           // C++20
std::to_underlying                   // C++23
std::byteswap                        // C++23
```

These facilities do not eliminate the need to understand the rules, but they reduce the amount of handwritten code that must reimplement them correctly.

### 10. Follow the review checklist for integer code

When reviewing integer-heavy C++, ask the following questions:

1. What are the **actual expression types after promotion**?
2. Can any intermediate operation overflow before assignment to a wider destination?
3. Are signed and unsigned operands mixed?
4. Is unsigned wraparound intended, or merely possible?
5. Is a narrowing conversion guaranteed to preserve all valid inputs?
6. Does the code rely on a particular data model such as LP64 or LLP64?
7. Does it assume `CHAR_BIT == 8`?
8. Is native endianness leaking into an external format?
9. Is code inspecting bytes through a permitted aliasing type?
10. Is union type punning being mistaken for portable C++?
11. Does `uint8_t` accidentally select a character overload?
12. Is an integer literal itself already unsigned or wider than expected?
13. Is a shift performed in the intended width, with a valid shift count?
14. Is floating-to-integer conversion range-checked before the cast?
15. Is integer-to-floating conversion allowed to lose identity above the exact-precision limit?
16. Is an enum value only representable, or also semantically valid?
17. Is random range mapping unbiased and overflow-safe?
18. Does deterministic RNG behavior need to survive a change of standard-library implementation?
19. Is the chosen `<cstdint>` family expressing exact width, the least width, or arithmetic preference correctly?
20. Would `std::in_range`, `std::cmp_*`, `std::midpoint`, `std::endian`, or `std::byteswap` express the rule more directly?

If these questions have explicit answers, the code is usually operating at the right level of rigor for systems, finance, embedded, networking, serialization, and other domains where integer mistakes become expensive.

---

## Diagnostics, useful compiler settings and extensions

Many dangerous integer conversions are legal C++, so a clean default warning set is not enough. On GCC/Clang, useful warning groups often include:

```text
-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow
```

Depending on the codebase, `-Wconversion` and `-Wsign-conversion` can be noisy, especially around legacy APIs. That is not a reason to ignore them; it is a reason to introduce them deliberately, possibly per target or after cleaning the highest-value code paths.

Useful sanitizers include:

```text
-fsanitize=undefined
```

for many classes of integer UB, including signed overflow and invalid shifts. Clang also provides integer-focused sanitizer options beyond the strict ISO-UB set, which can be useful for detecting suspicious unsigned wraparound or implicit truncation during testing.

For intentional wraparound, avoid globally disabling diagnostics if a narrower suppression or an explicit unsigned operation can document the intent.

Static analysis tools can also catch:

* narrowing assignments;
* suspicious signed/unsigned comparisons;
* shifts by invalid widths;
* lossy enum conversion;
* `char` passed directly to `<cctype>` functions;
* serialization assumptions tied to native layout.

The strongest approach is layered: types and APIs that express intent, compiler warnings, sanitizers, static analysis, and targeted tests at numeric boundaries.

---

## Standards timeline

### C++98/03: core integer model and promotions

The language already specified fundamental integer behavior that still matters:

* the fundamental integer type set and relative rank (`char`, `short`, `int`, `long`, ...);
* integral promotions and the usual arithmetic conversions;
* `sizeof(char) == 1` and the role of `CHAR_BIT` (byte width vs. octet);
* the emphasis on minimum ranges and relative ordering rather than universal bit widths.

### C++11: exact-width typedefs and Unicode code units

C++11 introduced key facilities used by modern integer code:

* `long long` and the `<cstdint>` typedefs (exact/least/fast widths);
* `char16_t` and `char32_t` for UTF-16/UTF-32 code units;
* `<random>` and other numeric utilities that surface integer pitfalls;
* list initialization (which affects narrowing and deduction in generic code).

### C++14: literal conveniences

C++14 added developer ergonomics for literals:

* binary integer literals (0b...);
* digit separators (`'`) for more readable large literals.

### C++17: explicit byte type and deduction fixes

C++17 clarified byte and deduction semantics:

* `std::byte` as a non-arithmetic byte-storage type;
* corrected `auto` direct-list/deduction rules (post-N3922 behavior implemented by compilers).

### C++20: representation guarantees and conversion helpers

C++20 brought several semantic and library changes important to integer code:

* two's-complement signed representation for ordinary signed integers;
* revised integer-conversion behavior (defined modulo semantics for out-of-range conversions);
* `char8_t` for UTF-8 code units;
* `std::bit_cast`, `std::endian`, `std::in_range`, `std::midpoint`, `std::ssize` for safe bit/size/endian and range utilities.

### C++23: ergonomics and enum/helpers

C++23 added useful helpers:

* `z`/`Z` integer-literal suffixes producing `std::size_t`/signed-size equivalents;
* `std::to_underlying` and `std::byteswap` for enums and byte-order operations.

### C++26: width macros

C++26 is expected to standardize named width macros in `<cstdint>` (e.g. `INT32_WIDTH`) and corresponding least/fast/pointer-width macros.

When maintaining code that supports multiple language modes, comment the **version dependency**, not merely the observed behavior of the current compiler.

---

## Further reading

Useful standard-library and language-reference entry points:

* C++ fundamental types: <https://en.cppreference.com/w/cpp/language/types>
* Implicit conversions and promotions: <https://en.cppreference.com/w/cpp/language/implicit_conversion>
* Arithmetic and shift operators: <https://en.cppreference.com/w/cpp/language/operator_arithmetic>
* Integer literals: <https://en.cppreference.com/w/cpp/language/integer_literal>
* Fixed-width integer types: <https://en.cppreference.com/w/cpp/types/integer>
* Safe integer comparisons: <https://en.cppreference.com/w/cpp/utility/intcmp>
* `std::in_range`: <https://en.cppreference.com/w/cpp/utility/in_range>
* Random number generation: <https://en.cppreference.com/w/cpp/numeric/random>
* `std::random_device`: <https://en.cppreference.com/w/cpp/numeric/random/random_device>


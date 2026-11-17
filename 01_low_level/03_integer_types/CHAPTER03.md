# Chapter 03. Integer Types: Representation, Conversion, and Arithmetic Contracts

An integer is often treated as the simplest kind of value a program can manipulate. It can be counted, compared, shifted, and stored without the apparent complexity of floating-point arithmetic or user-defined objects. Yet some of the most elusive defects in systems software arise precisely because integer expressions look simpler than their rules.

A character read from a socket may become a negative number. Two unsigned lengths may produce an enormous positive distance when one is subtracted from the other. A result assigned to a 64-bit variable may already have overflowed in a 32-bit intermediate. The same literal may have different types on Windows and Linux, while two programs using the same Mersenne Twister seed may disagree about the sequence of *bounded* random values they generate.

These behaviors follow a consistent set of language rules. The difficulty is that several contracts meet in one expression: type identity, width, rank, signedness, representation, conversion, ABI, and application meaning. We will develop those contracts from character types through fundamental integer arithmetic, initialization, fixed-width types, and random-number generation. The companion examples are \`character_types.cpp\`, \`integer_types.cpp\`, \`universal_init.cpp\`, \`unsigned_types.cpp\`, \`promotions.cpp\`, \`conversions.cpp\`, \`fixed_size.cpp\`, and \`random_int.cpp\`.

## 1. Characters, bytes, and integer values

C++ character types occupy an unusual position. They can represent text code units, behave as small integers in expressions, and—some of them—provide a permitted view of an object's representation. These roles are related but should not be mistaken for one another.

The types \`char\`, \`signed char\`, and \`unsigned char\` are three distinct types. Plain \`char\` has the same size, alignment, and representation as one of the signed or unsigned character types, but it is not an alias for either:

\`\`\`cpp
static_assert(!std::is_same_v<char, signed char>);
static_assert(!std::is_same_v<char, unsigned char>);

void accept(char);
void accept(signed char);
void accept(unsigned char);

accept('A'); // accept(char)
\`\`\`

Whether plain \`char\` has signed or unsigned values is an implementation decision. GCC and Clang provide options such as \`-fsigned-char\` and \`-funsigned-char\` to change the choice, so arithmetic involving ordinary text bytes may behave differently without source changes.

For example, on a machine with eight-bit signed plain \`char\`:

\`\`\`cpp
char c = '\xff';
auto unchanged = c; // char
auto promoted = +c; // normally int; value often -1
\`\`\`

On a machine where plain \`char\` is unsigned, the promoted value is commonly 255. The details of the character literal and its encoding also depend on the implementation, so binary-data code should not use this as a portable method of manufacturing an octet. Use \`unsigned char\` or \`std::byte\` when the intended meaning is raw storage.

### 1.1 A byte is an addressable unit, not necessarily an octet

The definition \`sizeof(char) == 1\` is unconditional. The claim that one byte contains eight bits is not. The macro \`CHAR_BIT\`, declared in \`<climits>\`, reports the number of bits in a C++ byte; the language requires at least eight.

\`\`\`cpp
static_assert(sizeof(char) == 1);
static_assert(CHAR_BIT >= 8);
\`\`\`

Machines with nine-, twelve-, sixteen-, and other-width addressable units are unusual today but illustrate why a language-level byte must not be equated automatically with an eight-bit octet. An array of four \`std::byte\` objects occupies four C++ bytes, but has 32 bits only when \`CHAR_BIT == 8\`.

A network protocol or file format typically defines its fields in octets. Making the requirement explicit is better than silently assuming every target satisfies it:

\`\`\`cpp
static_assert(CHAR_BIT == 8,
              "This implementation requires eight-bit bytes");
\`\`\`

Character arithmetic adds another subtlety. Operators normally apply integral promotions first:

\`\`\`cpp
unsigned char a = 200;
unsigned char b = 100;
auto sum = a + b; // commonly int, value 300
\`\`\`

The addition is not performed in an eight-bit \`unsigned char\` on the usual 32-bit-\`int\` platform. The result type of an expression is not necessarily the declared type of its operands.

### 1.2 Unicode code units are not characters on the screen

C++11 introduced \`char16_t\` and \`char32_t\`; C++20 introduced distinct \`char8_t\`. They represent code units for UTF-16, UTF-32, and UTF-8-oriented interfaces, respectively:

\`\`\`cpp
auto a = u8'A'; // char8_t since C++20
auto b = u'ß';  // char16_t
auto c = U'🍌'; // char32_t
\`\`\`

Neither a code unit nor even a Unicode code point necessarily corresponds to one user-perceived character. UTF-16 can require surrogate pairs; a visible grapheme may consist of several code points; rendering can combine or transform glyphs. Text-processing APIs should specify which of these units they count.

The type change for UTF-8 literals in C++20 has practical migration consequences:

\`\`\`cpp
u8"text" // const char8_t[N] array in C++20
\`\`\`

Before C++20, the elements of a UTF-8 string literal had type \`char\`. Source code that once passed such a literal to a \`const char*\` API may stop compiling after a language-mode upgrade. The new distinction is intentional: UTF-8 code units are no longer silently treated as ordinary narrow text.

The older \`wchar_t\` is a separate fundamental integer type with implementation-defined width and encoding conventions. Windows commonly uses 16-bit \`wchar_t\` with UTF-16 APIs, while many Unix-like systems use 32-bit \`wchar_t\`. It is useful as a platform interface type, but a poor field type for portable serialized data.

### 1.3 The object-representation exception

The language permits an object's representation to be inspected through \`char\`, \`unsigned char\`, and \`std::byte\` access paths:

\`\`\`cpp
std::uint32_t value = 0x12345678u;
auto* raw =
    reinterpret_cast<const unsigned char*>(&value);
\`\`\`

This access is a special exception to the usual type-aliasing restrictions. It does not authorize interpreting the object arbitrarily as an unrelated type, and the C++ rule does not give \`signed char\` the same general aliasing privilege. Nor does inspecting the bytes guarantee their order is stable across machines.

\`std::byte\`, introduced in C++17, makes the representation role especially clear. It is a scoped enumeration, not an arithmetic integer:

\`\`\`cpp
std::byte b{0x0f};
b <<= 1;    // bit manipulation
// ++b;     // ill-formed
// b + b;   // ill-formed
\`\`\`

Use \`std::byte\` when the objects are bytes rather than quantities. Choose \`unsigned char\` when numeric byte operations or interoperability make that more appropriate.

### 1.4 Two deceptively small lexical and library traps

A hexadecimal escape consumes subsequent hexadecimal digits until the sequence ends. Thus \`"\\x41B"\` is not the same as the letter A followed by B; the \`B\` is itself a hex digit. Octal escapes consume at most three octal digits. Adjacent-literal concatenation provides an explicit boundary:

\`\`\`cpp
auto text = "\x41" "B"; // two characters: 'A' and 'B'
\`\`\`

The functions in \`<cctype>\` present a more dangerous problem. They accept an \`int\`, but a call such as \`std::isspace(c)\` is valid only if its argument is \`EOF\` or represents an \`unsigned char\` value. Passing a negative signed \`char\` that is not \`EOF\` invokes undefined behavior:

\`\`\`cpp
char c = /* input byte */;
bool space = std::isspace(
    static_cast<unsigned char>(c)) != 0;
\`\`\`

An implicit promotion from \`char\` to \`int\` does not fix the domain violation. This distinction between *type compatibility* and *valid values* will recur throughout the chapter.

## 2. Fundamental integer types and the ABI

C++ specifies minimum widths and a size ordering for its standard integer types, not one universal storage layout. The signed fundamental sequence includes \`signed char\`, \`short\`, \`int\`, \`long\`, and \`long long\` (standardized in C++11), alongside the corresponding unsigned types.

The familiar minimum widths are eight bits for \`signed char\`, sixteen for \`short\` and \`int\`, thirty-two for \`long\`, and sixty-four for \`long long\`. Their storage sizes satisfy

\`\`\`cpp
sizeof(char) <= sizeof(short);
sizeof(short) <= sizeof(int);
sizeof(int) <= sizeof(long);
sizeof(long) <= sizeof(long long);
\`\`\`

The sizes may be equal. Integer conversion *rank*, bit width, object size, and representable range must nevertheless be treated as distinct properties.

### 2.1 Two's complement is now a language guarantee

Before C++20, the signed representation rules admitted sign-magnitude and ones'-complement alongside two's complement. C++20 standardized the two's-complement representation of ordinary signed integers. An N-bit signed type has the familiar range from \(-2^{N-1}\) through \(2^{N-1}-1\).

This representation rule does not authorize arithmetic overflow:

\`\`\`cpp
int maximum = std::numeric_limits<int>::max();
// ++maximum; // undefined behavior
\`\`\`

The compiler is still entitled to assume that an otherwise well-defined program does not overflow a signed integer expression. Representation and arithmetic semantics are different contracts.

### 2.2 Data models are observable at interfaces

The customary ABI data-model names summarize widely encountered choices:

| Model | int | long | Pointer | Common environment |
|---|---:|---:|---:|---|
| ILP32 | 32 | 32 | 32 | 32-bit systems |
| LP64 | 32 | 64 | 64 | 64-bit Linux, macOS, Unix |
| LLP64 | 32 | 32 | 64 | 64-bit Windows |
| ILP64 | 64 | 64 | 64 | Specialized systems |

These are common models, not the only models permitted by the standard. In particular, \`long\` is normally 64 bits under LP64 but 32 under Windows LLP64. A public interface involving \`long\` can therefore change representation or calling convention across targets.

This matters for C and foreign-function interfaces, shared memory, plugin APIs, binary files, RPC, and the format strings supplied to \`printf\` and \`scanf\`. The width of a parameter is an ABI matter even when a source declaration is accepted on both platforms.

\`std::size_t\` is the unsigned result type of \`sizeof\` and represents object sizes; \`std::ptrdiff_t\` is the signed type used for pointer differences. Neither is a synonym for “integer with the same width as a pointer.” Flat-address-space ABIs often make \`sizeof(std::size_t)\` and \`sizeof(void*)\` equal, but capability pointers and segmented addressing make it important not to rely on that coincidence in portable code.

### 2.3 Literals acquire types before assignment

A literal is typed before the initializer or assignment containing it is considered. For an unsuffixed decimal integer literal, the candidate sequence is \`int\`, \`long\`, then \`long long\`. For an unsuffixed hexadecimal, octal, or binary literal, unsigned types also appear in the candidate sequence:

\`\`\`text
int, unsigned int, long, unsigned long,
long long, unsigned long long
\`\`\`

Consequently, on common 32-bit-\`int\` platforms:

\`\`\`cpp
auto decimal = 2147483648; // often long (LP64) or long long (LLP64)
auto hex     = 0x80000000; // often unsigned int
\`\`\`

The equal numerical values do not imply equal expression types. Overload selection and later arithmetic can diverge accordingly.

C++23 introduced \`z\`/\`Z\` size-related integer suffixes. In particular, \`0uz\` has type \`std::size_t\`; the signed \`z\` form uses its corresponding signed integer type. It can be preferable to guessing a platform-specific \`ul\` or \`ull\` suffix when a size-like constant is intended.

Even the spelling \`-2147483648\` has a trap: the minus is a unary operator applied to a positive literal. There is no separate negative integer literal token. To express an extreme negative value portably, use \`std::numeric_limits<int>::min()\` instead of assuming the positive token first fits in \`int\`.

### 2.4 Endianness and alignment concern representation, not mathematical value

A multibyte integer has a value independent of where its most and least significant bytes are stored. Little-endian machines place the least-significant byte at the lowest address; big-endian machines place the most significant there. An \`unsigned char\` or \`std::byte\` view can expose this order without changing the abstract integer.

C++20 provides \`std::endian\` in \`<bit>\`:

\`\`\`cpp
if constexpr (std::endian::native == std::endian::little) {
    // native representation is little-endian
}
\`\`\`

The library also admits targets whose endianness is neither uniformly little nor uniformly big. C++23 adds \`std::byteswap\` for reversing the bytes of suitable integral values; whether to swap depends on the desired source and destination order. It is not a universal “convert to network order” operation: a big-endian host may need no swap.

A tempting way to inspect overlapping integer widths is a union:

\`\`\`cpp
union U {
    std::uint8_t  u8;
    std::uint16_t u16;
    std::uint32_t u32;
    std::uint64_t u64;
};
\`\`\`

But reading an inactive member for type punning is not generally portable standard C++, even if a compiler supports it as an extension. For object representation, copy bytes with \`std::memcpy\`, use \`std::bit_cast\` between suitable same-size trivially copyable types in C++20, or read an approved byte view. When the goal is to extract numeric fields, shifts and masks often communicate intent better because they do not depend on machine byte order.

Alignment is another essential ABI property. \`sizeof(T)\` describes storage, while \`alignof(T)\` specifies an alignment requirement. An integer inside a packed structure may be unaligned for its natural type; directly treating network packet bytes as a packed C++ object may violate alignment, lifetime, aliasing, or representation assumptions. Some processors permit unaligned loads, perhaps at a cost, while others can trap.

For externally specified layouts, explicit byte parsing, carefully bounded copies, and endianness-aware load helpers are often the safer abstraction than a cast to a struct pointer.

## 3. Initialization is part of the type contract

C++ has several forms of initialization, and differences that appear cosmetic for an \`int\` become significant with narrowing, constructors, and deduction:

\`\`\`cpp
int a{1};
int b = {1};
int c = 1;
int d(1);
\`\`\`

List initialization diagnoses many conversions that other forms accept:

\`\`\`cpp
int truncated = 3.14; // valid, result 3
// int rejected{3.14}; // ill-formed narrowing
\`\`\`

For integral conversions, the rule has important constant-expression exceptions. An integer constant expression whose actual value fits the destination may be accepted even if the source *type* has a larger range:

\`\`\`cpp
unsigned char valid{42};
// unsigned char invalid{300}; // narrowing
\`\`\`

This makes braces an effective compile-time check where a literal or constant expression is committed to a smaller representation.

“Uniform initialization,” however, is not truly uniform. Constructors taking \`std::initializer_list\` receive special preference in list initialization:

\`\`\`cpp
std::vector<int> a{99};     // one element containing 99
std::vector<int> b(99);     // 99 value-initialized elements
std::vector<int> c{10, 20}; // two elements
std::vector<int> d(10, 20); // ten elements, each 20
\`\`\`

The syntax is part of the API selection mechanism, not simply a matter of code style.

### 3.1 The changing meaning of \`auto\` with braces

The rules for deducing \`auto\` from a braced initializer have a history. Under the post-N3922 rule:

\`\`\`cpp
auto a{99};    // int
auto b = {99}; // std::initializer_list<int>
// auto c{1, 2}; // ill-formed
\`\`\`

Direct-list \`auto\` deduction requires a single element; copy-list initialization deduces an initializer-list element type, which must be consistent:

\`\`\`cpp
auto values = {1, 2};
// auto mixed = {1, 2.0}; // no common deduced element type
\`\`\`

Older compiler modes may have treated \`auto a{99}\` as an initializer list before the defect correction became widely implemented. This is an example of a language defect report changing practical behavior outside a clean version boundary.

Ordinary \`auto\` deduction also removes top-level cv-qualification and references when copying by value:

\`\`\`cpp
const int source = 42;
auto a = source;        // int
auto& b = source;       // const int&
const auto& c = source; // const int&
\`\`\`

\`decltype(auto)\` instead follows the \`decltype\` rules and can preserve references:

\`\`\`cpp
int x = 0;
int& alias = x;
auto copy = alias;           // int
decltype(auto) ref = alias;  // int&
\`\`\`

This can be important in generic code involving bit-field expressions, proxy objects, atomics, and user-defined numeric types; do not assume deduction always produces the declared type of the original object.

Empty braces provide a useful zero-initialization idiom for scalars:

\`\`\`cpp
int count{};
std::uint64_t mask{};
\`\`\`

An uninitialized automatic scalar declared as \`int count;\` would not have the same guarantee. But braces do not universally mean zero for every class type; constructors and other initialization rules remain relevant.

Finally, compile-time narrowing protection is not a runtime range check. If an input arrives as a \`std::uint32_t\`, the compiler cannot assume its current value will fit \`std::uint8_t\`. Test the range and only then convert:

\`\`\`cpp
if (std::in_range<std::uint8_t>(input)) { // C++20
    auto byte = static_cast<std::uint8_t>(input);
}
\`\`\`

Braces expose some narrowing problems early; they do not relieve an interface from validating runtime data.

## 4. Unsigned arithmetic is modular arithmetic

It is common to describe an unsigned type as a type for numbers that cannot be negative. The more useful model is mathematical: an unsigned N-bit integer computes modulo \(2^N\).

\`\`\`cpp
static_assert(0u - 1u == UINT_MAX);
static_assert(UINT_MAX + 1u == 0u);
\`\`\`

This is defined behavior, valuable for masks, hashes, checksums, sequence counters, some cryptographic primitives, hardware registers, and arithmetic intentionally conducted in a finite ring. It is not automatically desirable for sizes, distances, or domain quantities.

Suppose two unsigned indices are compared by subtraction:

\`\`\`cpp
if (i - j >= 4) {
    // ...
}
\`\`\`

When \`i < j\`, subtraction wraps; with 32-bit \`unsigned\`, \`1u - 2u\` gives \`4294967295u\`. The predicate now succeeds for the wrong reason. Write the intended ordering explicitly:

\`\`\`cpp
if (i >= j && i - j >= 4) {
    // ...
}
\`\`\`

The alternative \`i >= j + 4\` needs its own proof that \`j + 4\` cannot wrap. Rearranging an expression is not the same as proving its range.

### 4.1 Signedness interacts with conversion rank

Mixed comparisons use the usual arithmetic conversions. Consider:

\`\`\`cpp
unsigned i = 1;
int j = -1;
bool smaller = j < i; // commonly false
\`\`\`

On common ABIs, \`j\` converts to \`unsigned int\`; its value becomes \`UINT_MAX\`, so it is not less than 1. The rule is not simply “unsigned always wins.” The conversion sequence considers promotions, rank, signedness, and whether a higher-rank signed type can represent every value of the unsigned operand type.

C++20 comparison utilities express mathematical intent without that surprise:

\`\`\`cpp
std::cmp_less(-1, 1u);          // true
std::cmp_equal(-1, UINT_MAX);   // false
\`\`\`

Container sizes make this issue hard to avoid, since \`size()\` normally returns an unsigned \`size_type\`. A signed \`int\` loop variable compared against \`v.size()\` mixes categories; appropriate alternatives include a \`std::size_t\` index, a signed index bounded by \`std::ssize(v)\` (C++20), or a range-for loop when no index is needed.

\`\`\`cpp
for (std::size_t i = 0; i < v.size(); ++i) { /* ... */ }
for (auto i = 0; i < std::ssize(v); ++i) { /* ... */ }
for (auto& element : v) { /* ... */ }
\`\`\`

Unsigned reverse loops are another classic trap:

\`\`\`cpp
// Incorrect: condition remains true for every unsigned i
// for (std::size_t i = n - 1; i >= 0; --i) {}

for (std::size_t i = n; i-- > 0;) {
    // n-1, n-2, ..., 0
}
\`\`\`

The post-decrement comparison uses the old index value, so the body sees each valid index in reverse order. For readers unfamiliar with the idiom, reverse iterators or ranges may communicate intent more clearly.

### 4.2 Shift expressions reveal the real operand type

Assignment never retroactively widens an intermediate expression:

\`\`\`cpp
long long result = 1075 << i;
\`\`\`

The literal \`1075\` is normally an \`int\`. The shift is therefore computed in \`int\`; only afterward is the result converted to \`long long\`. If 64-bit arithmetic is intended, type the *left operand* accordingly:

\`\`\`cpp
long long result = 1075LL << i;
unsigned long long bits = 1075ULL << i;
\`\`\`

A negative shift count, or one at least as large as the width of the promoted left operand, is undefined behavior. In particular \`1u << 32\` is invalid when \`unsigned int\` is 32 bits.

The signed shift rules changed historically. Before C++20, left shift of signed values was restricted by representability rules, and right shift of negative signed values was implementation-defined. Since C++20, left-shift results follow the specified modulo-\(2^N\) bit-width semantics, while right shift of a negative signed integer rounds division by the power of two toward negative infinity. Invalid shift counts remain undefined. This is a good reason to annotate language-version requirements when teaching bit-level operations. <https://en.cppreference.com/w/cpp/language/operator_arithmetic>

Integral promotions affect bitwise NOT as well:

\`\`\`cpp
std::uint8_t value = 0x0f;
auto inverted = ~value; // commonly int, not uint8_t
auto low_bits = static_cast<std::uint8_t>(~value);
\`\`\`

On the usual platform the promoted result has many high bits set; the explicit final conversion expresses that only the low eight bits are intended.

Finally, unsigned types do not enforce a nonnegative *input* contract:

\`\`\`cpp
void set_count(unsigned count);
set_count(-1); // well-formed conversion, likely enormous count
\`\`\`

Select unsigned types for their intended algebra or ABI contract, not simply because negative values would make no sense to the application.

## 5. Integral promotions: arithmetic before arithmetic

Integral promotions are implicit value-preserving conversions applied to small integral operands before many operators. “Promotion” is a precise language category, not simply another word for widening.

For \`char\`, \`signed char\`, \`unsigned char\`, \`short\`, and \`unsigned short\`, the ordinary rule promotes to \`int\` if it can represent the entire source range; otherwise the promotion is to \`unsigned int\`. Thus on a familiar 32-bit-\`int\` ABI:

\`\`\`cpp
unsigned char x = 200;
auto promoted = +x; // int, 200
static_assert(std::is_same_v<decltype(+x), int>);
\`\`\`

Even two unsigned eight-bit operands can add as signed \`int\`:

\`\`\`cpp
unsigned char a = 200, b = 200;
auto sum = a + b; // usually int, 400
\`\`\`

No eight-bit wrap occurs in this addition. The promotion preserves the value; a later ordinary conversion may not.

Promotion has a better overload-resolution rank than an ordinary conversion:

\`\`\`cpp
void f(int);
void f(short);

char c = 0;
f(c); // f(int), not the 'closer-looking' short overload
\`\`\`

Unary plus offers a simple way to observe promotions, but unary minus, bitwise NOT, and many binary operators also apply them. A \`static_assert\` on \`decltype(expression)\` can be more informative than inspecting variable declarations.

### 5.1 Character, enumeration, and bit-field exceptions

\`char16_t\`, \`char32_t\`, and \`wchar_t\` have special promotion rules. They promote to the first suitable type from the ordered sequence \`int\`, \`unsigned int\`, \`long\`, \`unsigned long\`, \`long long\`, and \`unsigned long long\` (with additional extended-integer considerations as applicable). A \`char32_t\` commonly promotes to \`unsigned int\` rather than \`int\`.

\`\`\`cpp
char32_t c = U'🍌';
int i = c; // may involve promotion followed by a separate conversion
\`\`\`

Do not infer the promotion destination from the fact that these types are “characters.”

Unscoped enums can participate in integral promotions. Fixed-underlying-type unscoped enums have special rules favoring that underlying type, while scoped enums deliberately suppress ordinary implicit integer conversion:

\`\`\`cpp
enum OldMode : unsigned short { Off, On };
enum class Mode : unsigned short { Off, On };

// int n = Mode::On; // ill-formed
auto raw = std::to_underlying(Mode::On); // C++23
\`\`\`

Bit-fields have their own promotion corner cases. An \`unsigned\` bit-field of width one, for instance, has values 0 and 1, so ordinary lvalue-to-rvalue and promotion semantics may produce \`int\` in expressions:

\`\`\`cpp
struct Flags { unsigned ready : 1; };
\`\`\`

For hardware or packed-protocol abstractions this is a strong warning against assuming that the declaration's unsigned spelling survives each operation unchanged.

For a binary operator such as \`+\`, \`<\`, \`&\`, or \`%\`, the reliable conceptual sequence is: obtain operand values, apply applicable integral promotions, determine the common type through the usual arithmetic conversions, convert operands to that type, and perform the operation. The destination variable only sees the *result* of that sequence.

## 6. Conversions: the place where values change

Unlike promotions, ordinary integer conversions can change a value. The main families to distinguish are integer-to-integer conversion, signedness change, floating-to-integer conversion, integer-to-floating conversion, and conversions involving boolean or enumeration types.

For an unsigned destination, integer conversion is modulo \(2^N\):

\`\`\`cpp
std::uint32_t input = 65535;
auto low = static_cast<std::uint8_t>(input); // 255
\`\`\`

Before C++20, converting an out-of-range value to a signed integer type involved implementation-defined behavior. C++20 specifies the congruent result modulo \(2^N\) in the destination width. Neither rule makes signed *arithmetic overflow* valid; incrementing \`INT32_MAX\` in a signed 32-bit type remains undefined behavior.

A defined conversion can nevertheless be a logical bug:

\`\`\`cpp
std::uint32_t value = 0x12345678u;
auto byte = static_cast<std::uint8_t>(value); // 0x78
\`\`\`

The cast announces that a conversion is intended, but says nothing about whether discarding high bits is acceptable. At an API boundary, ask whether the conversion preserves every value permitted by the domain. If not, validate, reject, saturate, or document deliberate truncation.

C++20's \`std::in_range\` is useful for generic integer checking, as are the \`std::cmp_*\` comparison helpers. Handwritten comparisons such as \`x >= 0 && x <= max\` are easy to get wrong when \`x\` and \`max\` have different signedness.

### 6.1 An eight-bit number that behaves as a character

\`std::uint8_t\`, when provided, is often an alias for \`unsigned char\`. A typedef does not create a distinct type, so iostreams may select the character-output overload:

\`\`\`cpp
std::uint8_t byte = 65;
std::cout << byte;                    // often 'A'
std::cout << +byte;                   // 65 on typical ABIs
std::cout << static_cast<unsigned>(byte); // 65
\`\`\`

The same alias identity can affect overload resolution, formatting libraries, serializers, and template specializations. If an API requires a strongly distinct “eight-bit number” concept, an alias cannot supply it; use a wrapper type.

### 6.2 Crossing the floating-point boundary

Converting a floating-point value to an integer truncates toward zero:

\`\`\`cpp
static_cast<int>( 1.9); //  1
static_cast<int>(-1.9); // -1
\`\`\`

If the truncated result cannot be represented by the destination integer type, the behavior is undefined. This includes invalid cases involving NaN; converting to an unsigned type does not provide modulo wrapping for an out-of-range floating value.

A robust conversion from an external floating value establishes finiteness with \`std::isfinite\`, checks the intended range, and chooses a rounding policy—truncation, floor, ceiling, or nearest. The range tests must themselves avoid representing an integer bound inaccurately in the floating format.

The reverse conversion can lose precision without exceeding the floating type's range. Ordinary IEEE-754 binary64 has a 53-bit significand:

\`\`\`cpp
std::uint64_t a = 1ULL << 53;
std::uint64_t b = a + 1;
double da = static_cast<double>(a);
double db = static_cast<double>(b);
// da and db can be equal
\`\`\`

Every integer through \(2^{53}\) can be represented exactly, but consecutive integer values above that boundary need not be distinguishable. This matters in JSON and JavaScript bridges, metrics, databases, and serialized identifiers. Floating-point range and exact integer precision are different properties.

Conversion to \`bool\` collapses almost the entire arithmetic domain: zero becomes false, and all nonzero arithmetic values—including negative values and floating NaN—become true.

\`\`\`cpp
bool flag = 0.5; // true
int number = 0.5; // 0
\`\`\`

An enumeration cast poses a different question. A fixed-underlying-type enum may represent values not listed among its enumerators:

\`\`\`cpp
enum class Color : std::uint8_t { Red = 1, Green = 2, Blue = 3 };
Color c = static_cast<Color>(250);
\`\`\`

A representable value is not necessarily a *valid application state*. A deserializer must check both the underlying integer range and the set of values allowed by the protocol. Cast syntax is not a substitute for semantic validation.

### 6.3 Overflow-safe formulas still need correct rounding

Consider a common attempt to compute an average without overflowing \`a + b\`:

\`\`\`cpp
a / 2 + b / 2
\`\`\`

Integer division truncates, so with \`a == b == 1\` this yields zero instead of one. For appropriately ordered unsigned operands, \`a + (b - a) / 2\` can avoid intermediate overflow, but its own ordering precondition matters. Since C++20, \`std::midpoint\` provides a standard midpoint operation designed to avoid overflow and with specified rounding behavior.

The general engineering principle is that a replacement formula must preserve the required *result semantics*, not merely avoid undefined behavior.

Checked conversion can be centralized:

\`\`\`cpp
template<std::integral To, std::integral From>
To checked_cast(From value) {
    if (!std::in_range<To>(value))
        throw std::range_error("integer conversion out of range");
    return static_cast<To>(value);
}
\`\`\`

A codebase may instead require \`std::expected\`, an error code, an assertion, or saturating conversion. The important improvement is that value loss becomes a named, reviewable policy rather than an incidental consequence of assignment.

## 7. Selecting integer types by their guarantees

The header \`<cstdint>\` offers several families because “exact width,” “minimum width,” “compact storage,” and “fast arithmetic” are not the same requirement.

An exact-width name such as \`std::uint32_t\` is supplied only if the implementation has a suitable type with exactly that width and no padding bits. The implementation cannot substitute a 36-bit integer while calling it \`uint32_t\`. These types are natural when bits are part of the external contract: wire fields, file formats, cryptographic algorithms, hardware interfaces, and stable serialization schemas.

The \`std::int_least32_t\` family names the smallest available signed type with at least the specified width. This is appropriate when the *range* is required but exact physical representation is not.

The \`std::int_fast32_t\` family names an implementation-selected type with at least the requested width, intended to support fast arithmetic. It can be wider—sometimes much wider—than the minimum. A 64-bit register-friendly choice may increase array size, cache pressure, and memory bandwidth. “Fast” is a library implementation policy, not a universal microbenchmark guarantee.

A dense array of records often benefits from compact exact- or least-width fields while arithmetic uses larger promoted temporaries. Data-oriented design must evaluate storage and compute costs separately.

### 7.1 Pointer-sized and maximum-width integer types

When available, \`std::intptr_t\` and \`std::uintptr_t\` are capable of representing converted object pointers for the implementation's supported round-trip purposes. They can be useful in diagnostics, address-oriented APIs, and carefully documented tagging schemes.

They are not pointers. An integer holding address bits does not carry a pointee type, lifetime guarantee, bounds, or the complete semantics of pointer arithmetic. Capability architectures make these omissions especially visible. Keep values as pointers when the program means pointers; use \`std::ptrdiff_t\` for valid differences.

\`std::intmax_t\` and \`std::uintmax_t\` serve the widest standard integer-formatting and conversion role. Their purpose is compatibility and range, not optimal layout or performance in hot data structures.

### 7.2 Literal and width macros

Literal macros such as \`INT64_C\` and \`UINT64_C\` solve the problem that the correct integer-literal type/suffix can differ across ABIs:

\`\`\`cpp
auto value = UINT64_C(0xdeadbeef);
\`\`\`

The macros are traditionally tied to corresponding least-width integer types, so they should be understood according to their defined type-selection contract rather than as an unconditional requirement that the resulting type equal \`std::uint64_t\`. They remain useful in C interoperability and portable constant declarations.

C++26 adds bit-width macros such as \`INT32_WIDTH\`, \`INT_FAST32_WIDTH\`, \`INT_LEAST32_WIDTH\`, and \`INTMAX_WIDTH\`, with corresponding unsigned macros where applicable. <https://en.cppreference.com/cpp/utility/types/integer> Earlier code can obtain a type's precision from \`std::numeric_limits<T>::digits\`: for a standard unsigned 32-bit type this is 32; for its signed counterpart it is 31, excluding the sign.

The choice of integer type should follow the intended contract. Ordinary local arithmetic often needs \`int\` or \`long long\`. Container sizes need the appropriate \`size_type\` or \`std::size_t\`; signed differences use \`std::ptrdiff_t\` or the result of \`std::ssize\`. Protocols may need exact-width types; portable minimum-range storage may use least-width types; implementation-oriented arithmetic may use fast-width types; raw object representation calls for \`std::byte\` or \`unsigned char\`.

Replacing every \`int\` with \`int32_t\` is no more principled than leaving every external binary field as \`long\`. The type should express the property that correctness depends on.

## 8. Random integers are also an integer-conversion problem

Random-number generation brings the chapter's concerns together: finite ranges, overflow, modulo arithmetic, algorithm choice, ABI independence, reproducibility, and concurrency.

The legacy \`std::rand()\` interface provides a value in the inclusive interval \`[0, RAND_MAX]\` using hidden global state and offers weak quality guarantees. Mapping it with

\`\`\`cpp
lo + std::rand() % (hi - lo + 1)
\`\`\`

can overflow when computing the width, introduce modulo bias, rely on few generator bits, create shared-state contention, and produce implementation-dependent statistical quality. It is not appropriate for cryptographic secrets.

### 8.1 Why the modulus is not usually uniform

Suppose a source has ten equally likely outcomes, 0 through 9, and the target has four buckets. Mapping with \`r % 4\` yields buckets 0 and 1 three times each, but buckets 2 and 3 only twice each. The problem is divisibility: an equal mapping requires the number of accepted source values to be a multiple of the destination range size.

Rejection sampling fixes this by discarding source values in the incomplete tail. A correct implementation must count the source domain accurately—\`rand()\` has \`RAND_MAX + 1\` possible outputs—and must calculate that count without overflow. Statistical correctness does not excuse careless integer arithmetic.

C++11's \`<random>\` separates the generator *engine* from a *distribution*:

\`\`\`cpp
std::mt19937 engine(12345);
std::uniform_int_distribution<int> die(1, 6);
int face = die(engine);
\`\`\`

An engine defines a deterministic sequence of pseudo-random values and has choices involving state size, period, throughput, and statistical properties. A distribution transforms values into the requested statistical model. \`std::uniform_int_distribution\` maps to the inclusive integer interval \`[a, b]\`, avoiding the need for handwritten modulo reduction.

That inclusive upper bound is easy to confuse with half-open iterator and container ranges. To draw a valid container index, use \`[0, n-1]\` only after establishing that \`n != 0\` and the distribution's result type can represent the range.

### 8.2 Reproducibility has more than one level

\`std::mt19937\` is a standardized deterministic Mersenne Twister engine. Given the same properly initialized state, the engine sequence is specified. The mapping algorithm inside \`std::uniform_int_distribution\`, however, is not fixed across all standard-library implementations. Two builds with identical engines and seeds can produce different sequences of *bounded* results.

A simulation protocol, deterministic replay system, or cross-platform regression test that needs bit-for-bit identical output must specify both the engine and the range-mapping algorithm—or record the generated values. Pinning the seed alone is insufficient.

A Mersenne Twister also is not a cryptographically secure generator. Its large state and long period do not prevent recovery of the state from sufficient output. Password-reset tokens, cryptographic keys, security nonces, and similar values require an appropriate cryptographic randomness source.

### 8.3 Entropy, seeds, and state

Seeding an \`mt19937\` with one 32-bit number initializes a much larger internal state from a small seed space. \`std::seed_seq\` can diffuse several seed words into the state:

\`\`\`cpp
std::seed_seq seq{a, b, c, d};
std::mt19937 engine(seq);
\`\`\`

But it cannot manufacture entropy not present in its inputs. \`std::random_device\` aims to provide nondeterministic values when the implementation has such a facility, yet the standard permits a deterministic implementation when not available. Its name alone does not establish suitability for a security-critical operation.

Time-based seeding such as \`std::srand(std::time(nullptr))\` can give identical seeds to processes launched in the same second, and higher-resolution clocks are still predictable. A fixed documented seed is preferable for deterministic testing; an appropriately sourced unpredictable seed is necessary when unpredictability is the requirement.

### 8.4 Concurrency and performance

An engine has mutable state. Concurrent unsynchronized access to a shared engine can be a data race; locking it may become a bottleneck. A common approach is to maintain one engine per thread or worker:

\`\`\`cpp
thread_local std::mt19937 engine = make_engine_for_this_thread();
\`\`\`

The challenge is seed and stream management. Adjacent seed integers do not automatically provide mathematically independent random streams. Serious simulations may need stream-splitting, jump-ahead facilities, or generators designed specifically for parallel execution.

A benchmark comparing \`rand()\`, corrected range mapping, and \`mt19937 + uniform_int_distribution\` measures more than one thing: engine update cost, distribution reduction, rejection probability, inlining, library implementation, state locality, and possibly synchronization. A wide generator state can be costly when thousands of engines compete for cache, even if the single-thread hot loop looks efficient.

Meaningful performance measurements separate the engine from the distribution, cold construction from repeated sampling, and single-thread throughput from contention. The fastest generator in one workload is not automatically the best statistical or engineering choice in another.

## 9. Diagnostics and engineering practice

A useful first step in any surprising integer expression is to write down the *expression types*, not only the declaration types. Ask which operands are promoted, which common type the operator chooses, whether the operation is defined for every allowed value, and whether an assignment performs an additional conversion afterward.

There are three distinct classes of risk. The language may leave behavior undefined, as with signed arithmetic overflow and invalid shifts. It may define an undesirable result, as with unsigned wraparound or truncation to \`uint8_t\`. Or the result may be mathematically valid but violate an application requirement, as when an enum contains an unrecognized protocol value or a floating point number loses integer identity. Code review should look for all three.

GCC and Clang builds commonly begin with \`-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion\`, while MSVC offers \`/W4 /permissive-\`. Warnings are especially valuable for signedness mismatches and lossy conversions, but their coverage varies. \`-pedantic-errors\` can strengthen checks against language extensions.

GCC/Clang UndefinedBehaviorSanitizer (\`-fsanitize=undefined\`) can instrument various arithmetic and shift failures; more targeted checks may be available depending on the toolchain. AddressSanitizer (\`-fsanitize=address\`, or MSVC \`/fsanitize=address\`) addresses invalid memory accesses and certain lifetime failures encountered in low-level parsing. clang-tidy, cppcheck, and MSVC \`/analyze\` can point out suspicious conversions and contracts without executing the relevant paths. None proves the absence of an error.

Compiler options also affect assumptions. GNU language modes such as \`-std=gnu++20\` can accept constructs forbidden by strict ISO modes. \`-fno-strict-aliasing\` changes some alias-based optimization assumptions but does not create a missing object or legalize every type pun. \`-fwrapv\` gives signed overflow wrapping semantics under the toolchain's documented contract, rather than changing ISO C++ itself. \`-ffast-math\` permits floating-point transformations inconsistent with strict numerical guarantees; it is particularly relevant where integers cross a floating representation.

Vendor facilities including Clang \`__builtin_assume\`, MSVC \`__assume\`, GCC/Clang \`__builtin_object_size\`, \`__attribute__\`, and \`__declspec\` may improve performance or diagnostics. C++23's \`[[assume]]\` supplies a standard form of an unchecked optimization assumption where supported. An assumption is not a validation check, and vendor-specific behavior should be isolated. The C++ Core Guidelines, Microsoft GSL, static analyzers, and Compiler Explorer are useful complements for reasoning about contracts and observing real code generation.

The strongest strategy is layered: choose a type whose range and representation match the requirement; validate boundary values; make lossy conversions explicit; test edge cases; instrument executed paths; and only then consider a narrowly specified implementation extension.

## 10. Evolution from C++98 through C++26

**C++98/03** already contained the core hierarchy of fundamental integer types, integral promotions, usual arithmetic conversions, implementation-selected data models, and the signed/unsigned traps that still matter today. The language did not promise modern exact-width integer aliases as standard C++ vocabulary.

**C++11** standardized \`long long\` and \`<cstdint>\` integer families in C++, along with scoped enumerations, list initialization, and the engine/distribution design of \`<random>\`. These facilities made range and representation contracts easier to state without changing the long-standing rules for every ordinary integer expression.

**C++14** introduced binary integer literals and digit separators, improving readability of masks and embedded values. It also sits in the historical period of the braced-\`auto\` deduction correction, whose practical rollout includes defect-report implementations.

**C++17** expanded compile-time and generic vocabulary without replacing integral promotions or usual arithmetic conversions. Facilities such as \`std::byte\`, \`std::is_same_v\`, and modern deduction mechanisms made low-level intent easier to express.

**C++20** mandated two's-complement signed representation and clarified key conversion and shift behavior. It introduced \`std::cmp_*\`, \`std::in_range\`, \`std::midpoint\`, \`std::ssize\`, \`std::endian\`, and other support for accurate integer reasoning. The separation between language-defined numeric results and application policy remained essential.

**C++23** introduced \`std::byteswap\`, \`std::to_underlying\`, and the size-related literal suffix \`z\`/\`uz\`, reducing the amount of ad hoc representation and enum-conversion code needed in common tasks.

**C++26** adds width-reporting macros for integer types, with support depending on the relevant names and headers; continued work on checked arithmetic and related numerical facilities requires explicit verification against library implementations. A proposal's appearance in a future standard is not a guarantee that a particular toolchain already provides it.

Features should always be documented with their required language or library version, rather than with a blanket assertion based on the compiler brand.

## 11. Moving from Java, Python, and C#

Integer intuition transfers imperfectly between languages. Java has fixed-width primitive integer types with defined wraparound behavior for ordinary integer overflow; Python's built-in integers provide arbitrary precision subject to resource limits; C# distinguishes checked and unchecked arithmetic contexts. C++ provides a mix of implementation-dependent fundamental widths, defined unsigned modulo arithmetic, and undefined signed arithmetic overflow. None of those models is adequately captured by the claim that “integers are integers everywhere.”

Likewise, exceptions or runtime checks familiar from managed environments cannot be assumed at a low-level C++ failure point. A C++ expression may be rejected during compilation, produce a defined but surprising converted result, exhibit unspecified or implementation-defined behavior, or invoke undefined behavior. The implementation will not automatically report every narrowing or invalid aliasing access at runtime.

The useful transition is to make requirements explicit rather than mechanically translate type names. Select \`std::size_t\` or a container's \`size_type\` where the library contract calls for it, but compare carefully against signed offsets. Select exact-width storage when a wire format requires it, and a suitable arithmetic type for computation. Validate integer conversions rather than relying on casts. Use owning containers and checked APIs when lifetime or bounds are part of the task.

The point is not that C++ offers fewer guarantees in every respect. It offers several different guarantees, and leaves other decisions under the control of the programmer. Correct low-level work consists of knowing which is which.

## 12. The working model

Every integer expression has at least three stories. The *language story* establishes the types, promotions, value conversions, and defined or undefined operations. The *implementation story* supplies sizes, ABI conventions, object representation, diagnostics, and performance. The *application story* determines the allowed range, wire format, rounding policy, statistical quality, and error behavior.

When these stories are confused, a small code change can alter a system in a large way. A new overload changes which promotion wins. A wider destination fails to prevent intermediate overflow. A move from LP64 to LLP64 changes the representation of \`long\`. A fixed random seed fails to guarantee identical bounded samples across different libraries.

The durable rules are therefore simple in wording but demanding in practice: distinguish rank from width and from signedness; trace promotions before performing arithmetic; treat mixed signedness as a conversion problem; check ranges at the boundary where information may be lost; use unsigned types for deliberate modular behavior; and select random generators and range mappings according to separate statistical and reproducibility requirements.

> A valid low-level operation needs a language-level contract, not merely a machine-level outcome that looks plausible.

Before accepting an integer-heavy interface, ask whether the intended range is encoded in its types, whether every conversion is value-preserving or explicitly policy-driven, whether edge cases can be tested without executing undefined behavior, and whether binary representation assumptions are documented. Width, alignment, endianness, and the ABI belong in that review whenever storage crosses an interface.

## Further reading

For precise normative conditions, consult the appropriate C++ standard revision and the references on [fundamental types](https://en.cppreference.com/w/cpp/language/types), [implicit conversions](https://en.cppreference.com/w/cpp/language/implicit_conversion), [arithmetic operators](https://en.cppreference.com/w/cpp/language/operator_arithmetic), [integer literals](https://en.cppreference.com/w/cpp/language/integer_literal), and [fixed-width integer types](https://en.cppreference.com/w/cpp/types/integer). The standard-library documentation for [integer comparisons](https://en.cppreference.com/w/cpp/utility/intcmp), [range checking](https://en.cppreference.com/w/cpp/utility/in_range), [random engines and distributions](https://en.cppreference.com/w/cpp/numeric/random), and [\`std::random_device\`](https://en.cppreference.com/w/cpp/numeric/random/random_device) provides the complementary library contracts.

# Chapter 08. Bitwise Operations: Integer Models, Masks, and Machine-Level Algorithms

Bitwise programming often begins with a compelling illusion: if the processor has a shift instruction and C++ has a shift operator, then the source expression must mean exactly what that instruction does. On one machine, under one compiler, it frequently does. But source code is not a CPU instruction listing. A C++ expression first passes through integer promotions, signedness rules, type widths, value-range constraints, and the abstract machine's rules for defined and undefined behavior. Only then does the optimizer select instructions.

That difference is responsible for many of the most persistent low-level bugs. A mask intended to set one high bit unexpectedly sets several. A rotation works at all tested shift counts except zero. A zero input to a compiler builtin invokes undefined behavior even though the hardware bit-scan instruction has a predictable result. An apparently branchless minimum is slower than an ordinary conditional expression. A network field is decoded correctly on a little-endian development machine and backward on the target.

The modern response is not to abandon bit-level algorithms. Bit operations are fundamental to memory allocators, compressed indexes, bitboards, hashing, cryptography, SIMD algorithms, data structures, device interfaces, and binary protocols. The response is to distinguish three questions: **what integer operation does C++ specify, what representation is being manipulated, and what application invariant does the resulting pattern express?** Modern C++ provides a much clearer vocabulary for the first two, but the third remains the programmer's responsibility.

The repository's seven companion programs examine representation, shifts, arithmetic techniques, bit scans, flags, bit containers, and data layout. They can be read alongside this chapter as executable demonstrations; the baseline remains C++20, with later facilities called out explicitly. The essential historical techniques remain valuable—not necessarily as code to copy, but as explanations of how hardware and representation shape algorithms.

## 1. Bits exist within an integer type

A bitwise expression is not an untyped operation on an infinitely long sequence of bits. Its operands have C++ types, and those types determine the number of representable value bits, the effect of integer promotions, and the meaning of a shift.

For any complete object type, `sizeof(T) * CHAR_BIT` gives the number of bits in its object representation. The constant `CHAR_BIT`, from `<climits>`, is at least eight, but the language does not require every byte to contain exactly eight bits. In contrast, `std::numeric_limits<T>::digits` gives the number of non-sign value bits for an integer type. These quantities are related but should not be used interchangeably for generic code.

~~~cpp
#include <climits>
#include <cstdint>
#include <limits>

using U = std::uint32_t;
static_assert(std::numeric_limits<U>::digits == 32);
static_assert(sizeof(U) * CHAR_BIT == 32);
~~~

The assertions describe an exact-width type when the implementation provides `std::uint32_t`; a generic `unsigned int` need not be 32 bits. Code using a fixed-width protocol must either require an appropriate exact-width type or explicitly accommodate a different representation.

Unsigned integer arithmetic provides the best-defined domain for reasoning about bit patterns. For an unsigned type with \(N\) value bits, results of ordinary unsigned arithmetic are reduced modulo \(2^N\). Thus an increment of the maximum unsigned value wraps to zero, and unsigned negation gives a modular additive inverse. Those are language guarantees, not fortunate hardware behavior.

~~~cpp
std::uint32_t x = 0;
auto minus_one = x - 1u; // UINT32_MAX, modulo 2^32
~~~

This is why masks, counters intended to wrap, bit permutations, and shifts normally begin with an explicitly unsigned type.

### 1.1 Signed integers changed meaning across language revisions

For C++98 through C++17, the representation of negative signed integers was not fixed to one scheme by the core language. Two's complement, ones' complement, and sign-magnitude representations were historically relevant alternatives, and many expressions had rules sensitive to the representation. Modern C++20 standardized two's-complement correspondence between signed and unsigned integer representations, substantially simplifying the description of stored bits.

It is nevertheless a mistake to conclude that *all signed bitwise operations are now equivalent to unsigned operations*. Signed addition overflow is still undefined behavior in standard C++20. Shifts have their own rules, and integer promotions still occur before an operator is applied.

The useful principle is not that signed types are forbidden in low-level code. It is that a negative mathematical integer and an \(N\)-bit pattern are different abstractions. When the algorithm is explicitly about a pattern, convert to a suitable unsigned type and keep the operations there.

~~~cpp
int signed_value = -1;
std::uint32_t bits = static_cast<std::uint32_t>(signed_value);
// modulo 2^32: bits == 0xffffffff on this fixed-width type
~~~

The conversion to unsigned is defined modulo the unsigned type's range. This is a portable conversion operation, unlike reading an unrelated integer type through a pointer to the signed object.

### 1.2 Integer promotions matter even when the source looks unsigned

The expression `~x` deserves particular attention. Before the unary complement is applied, an operand such as `std::uint8_t` is usually promoted to `int` if all its values fit there:

~~~cpp
std::uint8_t mask = 0x0fu;
auto inverted = ~mask;
// inverted is typically int, with all higher promoted bits set
~~~

That is not an eight-bit complement. If the intended result is an eight-bit pattern, convert or mask intentionally:

~~~cpp
std::uint8_t eight_bit_inverse =
    static_cast<std::uint8_t>(~mask);
~~~

Similar promotion effects arise with `unsigned char`, short integer types, small enums, and bit-fields. A loop that manipulates `uint8_t` values may actually execute most of its expressions in `int`.

The same warning applies to expressions like `1 << 31`. The literal `1` has type `int`; appending a suffix or explicitly choosing the left operand's type can be decisive. It is not enough that the destination on the left side of an assignment is unsigned: the expression is evaluated under the types of its operands before assignment conversion occurs.

## 2. Six operators, several distinct semantics

C++ defines bitwise AND (`&`), OR (`|`), exclusive OR (`^`), complement (`~`), left shift (`<<`), and right shift (`>>`). The binary bitwise logical operators apply the usual arithmetic conversions after integral promotions to obtain a common integral type. Complement performs integral promotion on its operand. Shifts promote each operand separately, and their result type is the promoted left operand.

These rules make seemingly symmetric expressions behave differently. A shift by a small signed count does not change the type of the shifted value, while an AND between a signed and an unsigned operand may convert the signed operand to unsigned before the bitwise operation.

### 2.1 Precedence is a correctness issue

Operator precedence follows the grammar, not what a human reader considers the natural mask test:

~~~cpp
// Parsed as a & (b == c):
a & b == c

// Explicitly tests masked bits:
(a & b) == c
~~~

Comparison operators bind more tightly than bitwise AND. A classic permission test intended to compare a masked value therefore silently changes meaning if parentheses are omitted. This is especially treacherous when all operands are integers or booleans, because both expressions can compile.

Use parentheses around masks and shifts when they participate in comparisons or compound logical expressions. The compiler may optimize either spelling equally well; the first obligation is to communicate and preserve the intended predicate.

### 2.2 Bitwise operators do not short-circuit

For ordinary integral expressions, `&` and `|` evaluate both operands, unlike `&&` and `||`. This distinction matters when the right operand may access a pointer, call a function, modify state, or perform an expensive computation:

~~~cpp
bool safe = ptr != nullptr && ptr->ready();

bool unsafe = ptr != nullptr & ptr->ready();
// Both sides of the second expression are evaluated.
~~~

In the second expression, the bitwise AND is applied after the relevant conversions; the right-hand access is attempted even if the pointer is null. Use logical operators for control-flow conditions unless evaluating both operands and combining their Boolean representations is genuinely intended.

Not every evaluation-order detail of all operands is otherwise specified, so bitwise expressions should not be used as a clever substitute for sequencing side effects.

## 3. Shifts: useful operations with strict boundaries

A left shift on an unsigned \(N\)-bit value by a valid count \(k\) corresponds to multiplication by \(2^k\) modulo \(2^N\). An unsigned right shift corresponds to division by \(2^k\), discarding lower bits. This is the foundation of bit-field extraction, masks, packed identifiers, and fixed-width arithmetic.

The shift count, however, is a precondition. For a built-in shift, a negative count or a count at least as large as the width of the promoted left operand produces undefined behavior:

~~~cpp
std::uint32_t x = 1u;

// x << 32; // undefined behavior: count equals width
// x << -1; // undefined behavior: negative count
~~~

The presence of a CPU instruction that masks the count to a smaller range does not change the C++ rule. On x86 a particular shift instruction may effectively reduce a count modulo an instruction-defined amount, but the compiler is free to use the language's undefined-behavior precondition when optimizing source code.

A robust helper validates counts before evaluating the operator:

~~~cpp
std::optional<std::uint32_t> shift_left(
    std::uint32_t value, int count)
{
    if (count < 0 || count >= 32)
        return std::nullopt;

    return value << count;
}
~~~

For a generic unsigned type, use `std::numeric_limits<U>::digits` rather than a handwritten width, and decide whether an invalid count should throw, return an error, clamp, or be excluded by the API's precondition.

### 3.1 The signed shift rules deserve version-specific precision

Older explanations often say that shifting a signed positive value into the sign bit is always undefined behavior. That slogan is inaccurate. Under the C++11–17 wording, a signed left shift of nonnegative `a` by valid count `b` was permitted when the mathematical product could be represented in the *corresponding unsigned type*, after which conversion back to signed produced the result. This is why the traditional `1 << 31` case on a 32-bit `int` cannot simply be labeled undefined without qualification. The rules have also changed across language versions and defect reports.

C++20 describes the signed left shift using the congruent modulo \(2^N\) result with the fixed two's-complement model. In contrast, ordinary *signed addition* overflow remains undefined. Conflating those operations gives both false alarms and real missed bugs.

Right shift of a negative signed integer was implementation-defined in older standard revisions. C++20 specifies arithmetic right shift for signed negative values, with the result corresponding to division by a power of two rounded toward negative infinity:

~~~cpp
int x = -8;
int y = x >> 1; // -4 in conforming C++20
~~~

This does not make every historical arithmetic-shift idiom cross-version portable. If the algorithm concerns a bit pattern or must compile under C++98–17 rules, work with unsigned types. If it deliberately relies on C++20 signed shift semantics, state that minimum language version.

### 3.2 Shift before narrowing, not after guessing

Consider construction of a bit mask:

~~~cpp
std::uint32_t mask = 1u << 31;
~~~

On an implementation where `unsigned int` is 32 bits this forms the highest bit. For generic exact-width code, the left operand should be chosen explicitly and the shift count checked against its promoted width:

~~~cpp
std::uint32_t mask = std::uint32_t{1} << 31;
~~~

This still depends on the exact-width type existing, and on that width being 32 bits as specified by `uint32_t`. The code is far clearer than relying on a cast applied only after the shift has already been evaluated.

A high-bit operation over `std::uint8_t` deserves even more caution because small unsigned types are promoted before shifting. Writing `uint8_t{1} << 7` generally evaluates in `int`, then converts back if assigned to an eight-bit value. The result for this specific case is benign, but reasoning as though every intermediate were eight bits is wrong.

## 4. Complement, two's complement, and useful identities

In an unsigned \(N\)-bit domain, complement flips each of the \(N\) bits. That gives the familiar modular identities:

~~~cpp
U negated = U{0} - x;
U via_complement = U(~x) + U{1};
// same value for an unsigned U, with appropriate promoted conversion
~~~

For exact-width types at least as wide as `unsigned int`, `~x + 1` naturally behaves in the unsigned domain. For smaller unsigned types, promotions can change intermediate widths, so a generic algorithm should enforce or restore the intended unsigned result type.

The identity

~~~cpp
x & -x
~~~

isolates the least significant set bit in a suitable unsigned modular domain. For example, `0b10110000` yields `0b00010000`. Another classic expression,

~~~cpp
x & (x - 1)
~~~

clears the least significant set bit. A power of two has exactly one set bit, so for unsigned `x`:

~~~cpp
bool power_of_two = x != 0 && (x & (x - 1)) == 0;
~~~

The `x != 0` test matters: zero satisfies `(x & (x - 1)) == 0` under unsigned wraparound but is not a power of two.

These identities form an important common vocabulary for bitmap allocators, radix trees, sparse set traversal, and compressed bit vectors. They are not tricks to use blindly with signed integers. A naïve `-x` may invoke signed overflow at the signed minimum, and promotion can alter the intended mask width.

## 5. Arithmetic without conventional arithmetic: what the old tricks teach

Classic references such as *Hacker's Delight* show how elementary integer operations can be assembled from shifts and bitwise logic. Some techniques were invaluable on older processors, in bit-serial hardware, or in code where a limited instruction set was available. They remain excellent exercises in understanding carries and modular arithmetic.

### 5.1 Addition with XOR and carries

XOR computes each sum bit without carry. AND locates positions where both operands contain a one; shifting those common ones left generates the carry into the next bit:

~~~cpp
std::uint32_t add_bits(std::uint32_t a, std::uint32_t b)
{
    while (b != 0) {
        const auto carry = a & b;
        a ^= b;
        b = carry << 1;
    }
    return a;
}
~~~

The computation converges because carries move toward more significant positions and, in a fixed unsigned width, eventually vanish. The result equals ordinary unsigned addition modulo \(2^{32}\).

That equality does not imply the loop is faster than `a + b`. A modern adder instruction already performs the carry propagation in dedicated hardware. The algorithm is valuable for instruction-constrained environments, arithmetic-circuit design, and teaching—not as a default micro-optimization.

Subtraction can likewise be represented using an additive inverse:

~~~cpp
std::uint32_t result = a + (~b + 1u);
~~~

The unsigned semantics make the modulo behavior clear. For arithmetic that must detect underflow or preserve a mathematical negative result, choose a different operation or a wider signed representation rather than relying on wraparound to mean something it does not.

### 5.2 Overflow tests are type- and operation-specific

For unsigned addition, the modular result is less than one operand exactly when wraparound occurs:

~~~cpp
std::uint32_t sum = a + b;
bool overflow = sum < a;
~~~

If `a` and `b` are both nonnegative unsigned values of the same width, either operand can be used in this test. This is not a general signed-overflow detector and should not be copied to signed types whose overflowing addition would already be undefined.

For unsigned subtraction, underflow is tested directly with `a < b` *before* subtraction. For multiplication, use division-based checks or compiler checked-arithmetic facilities, taking care with zero and type width. For allocation sizes or wire lengths, returning a checked result is often preferable to creating a wrapped value and hoping its consumer notices.

### 5.3 Branchless selection and the cost of cleverness

A Boolean condition can be turned into an all-zero or all-one unsigned mask:

~~~cpp
std::uint32_t mask = std::uint32_t{0} - std::uint32_t(a < b);
std::uint32_t selected =
    (a & mask) | (b & ~mask);
~~~

If `a < b`, the mask contains all ones and `selected` is `a`; otherwise it is `b`. This implements an unsigned minimum.

The expression is not inherently faster than `std::min(a,b)`. Modern compilers may lower a branchless conditional to a conditional move, predicated operation, or vector instruction. The expanded bitwise form can increase instruction count, inhibit optimization, or produce poor dependency chains. On a processor with accurate branch prediction, a conditional branch may be excellent. On a data-parallel path with unpredictable conditions, branch elimination may help—but that conclusion needs measurement.

The technique also needs correct unsigned typing: a signed negation or a mask of the wrong width can produce an invalid algorithm. The familiar `abs(INT_MIN)` problem is another illustration of why arithmetic identities cannot be moved casually into signed integer space: the positive counterpart of the most negative signed integer may not be representable.

## 6. Counting set bits

The *population count* or Hamming weight of an unsigned value is the number of one bits in its representation. For example, `0b101101` has four set bits. Applications include bitmap cardinality, chess bitboards, Bloom filters, compressed data structures, error-detection schemes, and set intersections.

Before C++20, programmers used several approaches. The Kernighan loop repeatedly clears the lowest set bit, performing one iteration per set bit:

~~~cpp
int count_ones(std::uint32_t x)
{
    int count = 0;
    while (x != 0) {
        x &= x - 1u;
        ++count;
    }
    return count;
}
~~~

For sparse patterns, the number of iterations is small; for dense ones, it can be large. SWAR (*SIMD Within A Register*) algorithms combine masks, shifts, and additions to count bits in parallel groups inside a machine word. Lookup tables trade memory traffic and cache footprint for fewer arithmetic operations. De Bruijn multiplication-based techniques are especially well known for bit-index lookup rather than for population count itself.

C++20 supplies the intended abstraction directly:

~~~cpp
#include <bit>
#include <cstdint>

static_assert(std::popcount(std::uint32_t{0b101101u}) == 4);
~~~

`std::popcount` accepts unsigned integer types and returns an `int`. The implementation can use native population-count instructions when available, or another correct lowering when not. This is normally preferable to manually selecting a historical algorithm based on assumptions about input bit density.

The utility counts *representation bits*, not a mathematical property of a negative signed number. If the source is signed, explicitly convert it to an unsigned type of the intended width before calling `popcount`. That conversion is defined modulo the unsigned range; the width still matters to how many high bits can be set.

## 7. Bit scans and the zero problem

Counting trailing zeros (`CTZ`) finds the position of the least significant one bit in a nonzero unsigned value. Counting leading zeros (`CLZ`) measures the number of zero bits before the most significant one. Both are fundamental in buddy allocators, priority selection, radix trees, binary codecs, compressed indexes, and bitmap traversal.

Historically, portable implementations used loops, lookup tables, or De Bruijn sequences. Compiler builtins and intrinsics often exposed direct machine instructions, but they introduced compatibility hazards. For instance, many GCC/Clang `__builtin_ctz` or `__builtin_clz` interfaces have undefined behavior for input zero, even if the selected hardware instruction gives a defined result on a particular processor.

C++20's functions specify the edge case:

~~~cpp
#include <bit>
#include <cstdint>

static_assert(std::countr_zero(std::uint32_t{0}) == 32);
static_assert(std::countl_zero(std::uint32_t{0}) == 32);
static_assert(std::countr_zero(std::uint32_t{0b1000}) == 3);
~~~

For zero, the count is the width of the unsigned type. This is a powerful guarantee, but a data structure still has to decide whether zero means “no available item,” “index 32,” an error, or some other sentinel. The library gives a bit count, not a domain-specific interpretation.

Related operations include `std::countl_one` and `std::countr_one`, which count runs of one bits from either end. These can be useful for scanning packed occupancy maps and prefix encodings.

### 7.1 `bit_width`, `bit_floor`, and `bit_ceil` are not scans with new names

`std::bit_width(x)` returns the number of bits needed to represent the unsigned value `x`, giving zero for zero. It is related to the location of the highest set bit but is not the count of leading zeros. `std::bit_floor(x)` returns the greatest power of two not exceeding `x`, or zero if `x` is zero. `std::bit_ceil(x)` returns the least power of two not less than `x`, with the special result 1 for zero.

~~~cpp
static_assert(std::bit_width(std::uint32_t{0}) == 0);
static_assert(std::bit_width(std::uint32_t{70}) == 7);
static_assert(std::bit_floor(std::uint32_t{70}) == 64);
static_assert(std::bit_ceil(std::uint32_t{70}) == 128);
static_assert(std::has_single_bit(std::uint32_t{64}));
~~~

The edge case for `bit_ceil` is not merely a performance detail: if the required power of two is not representable in the return type, its behavior is undefined. Rounding an untrusted allocation size to the next power of two must first verify that the input is within the safe domain. A standard library function can express the algorithm clearly while still having a significant precondition.

There is no standard `std::bit_round` in C++20's `<bit>`. The relevant standardized operations are `bit_floor`, `bit_ceil`, and `bit_width`. A rounding policy choosing the *nearest* power of two would require its own tie-breaking and overflow definition.

## 8. Rotation is not a shift with an inconvenient edge case

A rotate moves bits around a fixed-width word, reinserting bits shifted off one end at the other. It appears in cryptographic round functions, hash mixing, pseudorandom generators, and bit permutations.

The classic formula

~~~cpp
(x << n) | (x >> (width - n))
~~~

is dangerous as a general implementation. If `n == 0`, the second shift is by the full width. If `n == width`, the first shift is by the full width. Both violate the built-in shift-count precondition, even though the mathematical rotation by a multiple of the width should simply return `x`.

C++20 makes the intended operation explicit and defines count normalization:

~~~cpp
#include <bit>
#include <cstdint>

constexpr auto x = std::uint32_t{0x12345678u};

static_assert(std::rotl(x, 8) == 0x34567812u);
static_assert(std::rotr(x, 8) == 0x78123456u);
static_assert(std::rotl(x, 32) == x);
~~~

`std::rotl` and `std::rotr` accept unsigned integer types; their rotation counts may be normalized relative to the type width, including zero, full-width multiples, and negative counts. They communicate the operation directly to the compiler, which can select a rotate instruction if one is available.

Rotation is a permutation of a word's bits. It is not byte swapping, arithmetic multiplication, or a generic serializer. Those tasks have different contracts despite all being expressible with shifts and masks.

## 9. Masks, flags, and security predicates

The historical pattern

~~~cpp
#define READ  1
#define WRITE 2
~~~

offers short names but no type-level domain separation. A modern C++ API can describe independent permissions with a scoped enumeration and explicit unsigned representation:

~~~cpp
enum class Permission : std::uint32_t {
    none  = 0,
    read  = 1u << 0,
    write = 1u << 1,
    exec  = 1u << 2,
    all   = (1u << 0) | (1u << 1) | (1u << 2)
};
~~~

The type prevents accidental conversion to unrelated status enums or ordinary integers. Bitwise operators should be supplied deliberately for this *flag* domain, not indiscriminately for every enumeration:

~~~cpp
constexpr Permission operator|(Permission a,
                               Permission b) noexcept
{
    using U = std::underlying_type_t<Permission>;
    return static_cast<Permission>(
        static_cast<U>(a) | static_cast<U>(b));
}
~~~

A state enum such as `Traffic::red` or `Traffic::green` represents mutually exclusive alternatives; enabling OR between those states would obscure their semantics rather than enrich them.

### 9.1 The complement must respect the domain mask

A raw `~` turns on all bits absent from the operand, including reserved and unsupported bits. Because the enum has a fixed unsigned underlying type, the resulting pattern can be representable in the language while violating the application's flag schema.

Define the supported mask and constrain the result:

~~~cpp
constexpr Permission operator~(Permission p) noexcept
{
    using U = std::underlying_type_t<Permission>;
    constexpr U valid = static_cast<U>(Permission::all);
    return static_cast<Permission>(
        valid & ~static_cast<U>(p));
}
~~~

A design that must preserve unknown flags for forward compatibility should specify that policy separately rather than silently dropping them. A parser can require `(raw & ~valid_mask) == 0` when unknown bits are forbidden; an extensible protocol may instead retain them with separate validated semantics.

### 9.2 Any requested bit is not all requested bits

Given a bitmask of actual permissions and another mask of required permissions, two predicates are frequently confused:

~~~cpp
bool any(std::uint32_t actual, std::uint32_t requested)
{
    return (actual & requested) != 0;
}

bool all(std::uint32_t actual, std::uint32_t requested)
{
    return (actual & requested) == requested;
}
~~~

The first returns true if at least one requested bit is present. The second returns true only if every requested bit is present. Their difference can be a security boundary, not a harmless naming preference.

For an empty requested mask, `all(actual, 0)` is true, while `any(actual, 0)` is false. These results follow Boolean set algebra, but the API must decide whether an empty request means “nothing required,” invalid input, or a separate sentinel.

A correctly executed bitwise AND does not establish authorization correctness. The right predicate, supported mask, unknown-bit handling, and trust boundary all matter.

## 10. Bit containers and raw bytes have different purposes

C++98 supplied `std::bitset<N>`, a fixed-size logical collection of bits with indexing, testing, setting, resetting, flipping, counting, and bitwise operations. It is useful when the number of logical bits is a compile-time property, such as a set of feature flags or a small bitmap. Its representation is not required to expose one `bool` object per logical bit or a particular serialized byte layout.

`std::vector<bool>` has a special compact representation in many implementations. Its element access uses proxy reference types rather than ordinary `bool&` references. This is a real semantic trade-off: bit packing saves memory but may surprise generic code that assumes `*it` is an actual object reference. The specialization does not model a contiguous range of `bool` objects and should not be treated as equivalent to `std::vector<std::uint8_t>`.

Choosing between these containers depends on capacity, density, required element-address semantics, atomicity, and iteration patterns. A vector of bytes provides ordinary addressable elements at greater storage cost; a packed bitmap may save memory and improve cache residency but require proxy-aware algorithms and word-level synchronization.

C++17 introduced `std::byte` as a representation-oriented vocabulary type. It supports bitwise operations, but not ordinary arithmetic on a numerical value:

~~~cpp
#include <cstddef>

std::byte b{0x12};
auto lower = b & std::byte{0x0f};
unsigned n = std::to_integer<unsigned>(lower);
~~~

A `std::byte` expresses “raw storage or serialized byte” more clearly than `char` used as an accidental arithmetic type. However, a C++ byte is not universally an eight-bit octet; for a protocol defined in octets, check `CHAR_BIT == 8` or use an explicitly supported octet handling strategy.

Neither `bitset` nor `vector<bool>` supplies automatic per-bit atomic operations. For a concurrent bitmap, an array of suitably chosen atomic unsigned words is often the appropriate representation, with explicit mask operations and memory-order rules.

## 11. Value-level bits are not the bytes of a protocol

An integer has a numeric value with bits, and an object representation with bytes laid out in memory. The two are related, but they are not interchangeable descriptions.

Consider:

~~~cpp
constexpr std::uint32_t header = 0x12345678u;
constexpr auto first_octet = (header >> 24) & 0xffu;

static_assert(first_octet == 0x12u);
~~~

This extracts the high eight bits of the numerical value, independent of how the machine stores that integer's bytes. A direct `memcpy` of the integer into a packet instead copies its native object representation, whose byte order may be little-endian, big-endian, or subject to a more unusual arrangement.

C++20's `std::endian` exposes the native scalar byte-order category. C++23's `std::byteswap` reverses the bytes of suitable integral values; it helps implement byte-order conversion but does not choose the network format, field boundaries, sign representation policy, or padding rules.

~~~cpp
// C++23:
std::uint32_t network_word = std::byteswap(host_word);
// Correct use depends on the specified host/wire ordering.
~~~

Do not unconditionally byteswap a value when the host already uses the desired byte order; use a documented conversion function that checks the relevant layout assumptions.

### 11.1 Bit-fields do not define portable packet layouts

A declaration such as

~~~cpp
struct DeviceFlags {
    unsigned ready : 1;
    unsigned mode  : 3;
};
~~~

may look like a four-bit wire header. It is not. The allocation order of bit-fields within a storage unit, padding, alignment, and several layout details are implementation-defined. Bit-fields also have restrictions on address-taking and can undergo integer promotions when read.

For a portable format, decode bytes into a known unsigned integer width and use explicit shifts and masks. For a device register, follow the hardware vendor's register map and the platform's MMIO rules; ordinary C++ bit-fields may be appropriate only under a deliberately documented ABI/compiler contract.

`volatile` may be relevant for some device-memory accesses, but it is not an inter-thread synchronization primitive. Atomicity, ordering, and device bus behavior require separate rules.

### 11.2 A shift-based serializer can be clearer than a struct dump

When a protocol explicitly stores four octets in network order, extraction can make each field position visible:

~~~cpp
std::array<std::uint8_t, 4> encode_be(std::uint32_t x)
{
    return {
        static_cast<std::uint8_t>(x >> 24),
        static_cast<std::uint8_t>(x >> 16),
        static_cast<std::uint8_t>(x >> 8),
        static_cast<std::uint8_t>(x)
    };
}
~~~

The array is four octets if `std::uint8_t` exists as an eight-bit type, as assumed by the signature. This representation is independent of the host's native integer byte order. For decoding, shift unsigned widened values back into their specified positions and OR them together.

A format with sub-byte fields similarly needs explicit bit numbering: “bit 0” may refer to the least significant bit of a numeric field or to the first transmitted bit of a byte. Protocol documents need to distinguish those conventions. A C++ bit mask does not resolve them automatically.

## 12. Layout and bitwise algorithms

Efficient bitwise algorithms often owe as much to memory organization as to the instructions that manipulate individual bits.

A bitmap can compactly describe the occupancy of an allocator's free lists. A Bloom filter trades exact membership information for compact probabilistic tests. A chess bitboard represents a set of squares as machine-word bits. A compressed columnar index can intersect sets with word-level AND and count results with `popcount`. All exploit the fact that several logical Boolean decisions fit inside one integer object.

Radix sorting illustrates a different use: rather than compare complete keys, a pass partitions or distributes records according to selected key bits. A stable least-significant-digit (LSD) radix sort processes digit groups from low to high significance and preserves the relative order within each bucket. A most-significant-digit (MSD) approach partitions on high-order groups and proceeds recursively or iteratively within partitions. Both require deliberate treatment of signed keys, floating encodings, key ordering, and bucket-count overflow.

Radix sorting can be extremely fast when key widths are fixed and memory traffic is favorable, but it is not universally comparison-free in every implementation detail, nor automatically cache-friendly. Repeated passes can be bandwidth-bound, histograms and scatter phases can stress caches, and data movement can dominate the cost of bit extraction.

Bit packing likewise trades simple element addressing for higher density. A packed representation may reduce memory bandwidth and improve cache usage, while increasing instruction count for extraction and insertion. The correct trade-off depends on working-set size, access locality, vectorization, and the frequency of updates.

A single shift instruction is rarely the whole performance story. Data layout, aliasing, memory allocation, branch prediction, and NUMA placement can dominate the result.

## 13. Portability and the compiler's optimizer

Historical bit hacks often developed because certain operations were unavailable or expensive on older hardware. Counting bits with lookup tables, approximating arithmetic with masks, avoiding branches at all costs, or writing hand-rolled rotations could give substantial gains. The instruction set and optimizer have changed.

Modern x86 and ARM processors commonly offer population-count, bit-scan, rotate, conditional-select, and vector operations. Compilers recognize many conventional bit idioms and may lower `std::popcount`, `std::rotl`, and `std::has_single_bit` directly to efficient instructions. An obscure equivalent expression can instead prevent that recognition.

This is one reason standard vocabulary is not merely about readability. A standard function presents an operation with a precise edge-case contract, leaving the implementation free to choose the optimal target-specific lowering.

### 13.1 Undefined behavior is not a promise of the hardware result

Signed overflow, invalid shift counts, out-of-bounds memory accesses, and some invalid type punning permit optimizer assumptions that differ radically from the instruction-level intuition of a programmer reading assembly. An invalid left shift count may be optimized on the assumption that the count never occurs; testing what the hardware instruction does with 32 on one processor cannot prove a C++ program valid.

Where wrapping signed arithmetic is genuinely desired, vendor switches such as `-fwrapv` can impose specific implementation semantics, but they are compiler contracts rather than portable ISO C++ rules. The simpler approach for bit-pattern arithmetic is to use an unsigned type. `-fno-strict-aliasing` relaxes some alias-based optimization assumptions but does not legalize unrelated typed dereferences or missing object lifetimes.

Similarly, `-ffast-math` changes floating-point optimization rules and is not a general bitwise optimization flag; it should not be added to a build merely because the programmer wants faster integer code.

### 13.2 When “branchless” matters, benchmark an actual workload

A fair benchmark compares the complete operation under realistic data distributions and compiler settings. It should keep results observable, select the actual CPU target and instruction availability, and include memory effects if the operation is part of a larger algorithm.

For instance, a synthetic branchless minimum may appear superior under random inputs yet lose when a branch is almost always predicted. A SWAR popcount may excel on a target without hardware support but lose dramatically on a processor with a fast dedicated instruction. A packed bitmap may outperform a byte array for scans but underperform it for random individual updates.

Optimization decisions should be reported with both performance and correctness conditions. If a fast path requires a power-of-two capacity, nonzero input, aligned storage, or a limited flag domain, those requirements belong in the API rather than in a comment inside a benchmark.

## 14. A modern vocabulary in `<bit>`

C++20's `<bit>` standardized many formerly handwritten operations. The header includes bit counting, leading/trailing scans, rotations, power-of-two helpers, endianness inspection, and representation conversion through `std::bit_cast`.

These facilities do not all share the same operand rules. Most arithmetic bit helpers require an unsigned integer type; `std::bit_cast` works with equally sized trivially copyable types and transfers object representation rather than performing a numeric cast. `std::endian` describes byte layout rather than the result of arithmetic.

A practical mapping is:

| Intent | C++20 facility |
|---|---|
| Count one bits | `std::popcount` |
| Count leading/trailing zeros | `std::countl_zero`, `std::countr_zero` |
| Count leading/trailing ones | `std::countl_one`, `std::countr_one` |
| Rotate left/right | `std::rotl`, `std::rotr` |
| Test one set bit | `std::has_single_bit` |
| Determine required width | `std::bit_width` |
| Round down/up to power of two | `std::bit_floor`, `std::bit_ceil` |
| Inspect native endian category | `std::endian` |
| Transfer object representation | `std::bit_cast` |

C++23's `std::byteswap` adds a standard byte-reversal operation for integral values. None of these functions is intended to turn a logically invalid protocol mask into a valid one automatically. They correctly implement defined primitive operations; application invariants must be layered above them.

A feature-tested codebase can use `__cpp_lib_bitops`, `__cpp_lib_int_pow2`, and relevant library feature macros to select implementations when older standard libraries are still supported. The presence of the `<bit>` header alone does not necessarily imply availability of every C++20 and C++23 facility.

## 15. Data-parallel evolution through C++26

C++26 adds standard data-parallel facilities in `<simd>`, allowing code to express operations over several lanes and associated masks. These tools can replace some manual packing and vendor-specific vector-intrinsic code, especially when the algorithm naturally processes multiple independent elements.

The abstraction does not make every packed struct a SIMD vector or erase memory-model requirements. Aligned loads, vectorized stores, cross-lane operations, and masked accesses still need valid storage, alignment, and lifetime. An accelerator or CPU-specific intrinsic can have additional preconditions beyond the scalar C++ language.

When using SIMD for bitwise processing, distinguish word-level logical operations from lane-wise operations and from byte-addressed memory layouts. A vector of sixteen 8-bit code units, four 32-bit words, and one 128-bit integer may occupy the same number of bits but express different semantics and have different supported operations.

Concurrency introduces another independent dimension. Atomic word operations can support concurrent bitmaps:

~~~cpp
std::atomic<std::uint32_t> occupied{0};

void claim_bit(unsigned index)
{
    // Precondition: index < 32.
    occupied.fetch_or(std::uint32_t{1} << index,
                      std::memory_order_acq_rel);
}
~~~

The mask update is atomic, but the meaning of “claim” may require checking whether the bit was already set and coordinating associated resources. The chosen memory order must match how the bitmap publishes or protects other data. A single atomic fetch-or is not automatically a complete allocator protocol.

In C++26, scalar bit semantics remain what they were in the corresponding C++20/23 model. The new facilities expand the vocabulary for expressing parallel work; they do not relax shift-count, object-lifetime, or ownership constraints.

## 16. Diagnostics, standard modes, and testing

A warning-clean build is a useful baseline, but most bitwise bugs arise from combinations of types and inputs that a compiler cannot infer completely. GCC and Clang's `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion` reveal narrowing, signedness changes, and suspicious constructs. Additional shift and overflow warnings are available under compiler-specific names, such as `-Wshift-count-negative`, `-Wshift-count-overflow`, and related diagnostic groups. MSVC `/W4 /permissive-` and optional static analysis expose corresponding issues.

UndefinedBehaviorSanitizer (`-fsanitize=undefined`) can instrument many erroneous operations including invalid shift counts; Clang/GCC also expose selected shift-related sanitizer checks. It does not prove every bitwise algorithm correct. AddressSanitizer concerns storage accesses rather than the logic of a mask; it is relevant when packed buffers, bitmaps, and shift-generated indices are used to address memory. Static-analysis tools such as clang-tidy and cppcheck can detect suspicious conversions, overflow patterns, and unchecked boundaries, but application-level flag validity must still be tested.

Use strict ISO language modes such as `-std=c++20` where the book is teaching portable rules. GNU extension modes can accept behavior outside that baseline. Platform-specific builtins for population count, leading/trailing zeros, rotations, assumptions, and object-size inference should be isolated behind narrow adapters with documented zero-input and width preconditions.

The test matrix should include zero, one, the highest set bit, all ones, adjacent powers of two, the maximum input that can be rounded up safely, negative and full-width shift counts *as rejected inputs*, and flags containing reserved bits. A test should validate preconditions before executing an undefined shift or cast; it should not deliberately invoke UB and infer portability from the observed result.

For masks that control permissions, test every combination of granted and required flags, including the empty set and unknown bits. For serialization, compare exact wire octets on little-endian and big-endian expectations. For data-parallel and branchless optimizations, inspect optimized assembly with Compiler Explorer and measure representative workloads rather than merely counting source operators.

Vendor capabilities are useful when they serve a real goal. The C++ Core Guidelines, Microsoft GSL, compiler warnings, sanitizers, and static-analysis tools are complementary; the primary defense is a design that chooses the right unsigned representation and explicitly states its valid domain.

## 17. The standards timeline

**C++98/03.** Bitwise operators inherited the C-family integer promotions and shift-count rules. Unsigned modular arithmetic was defined; signed representation could differ across platforms. `std::bitset<N>` already provided a fixed-size logical bit collection. Many efficient operations depended on hand-written idioms or compiler extensions.

**C++11.** Scoped enums allowed strongly typed flag domains with explicit underlying types. Fixed-width integer aliases and related type machinery helped source code state its width assumptions. The core bitwise operators and their dependence on promotions remained recognizable.

**C++14.** Binary integer literals, such as `0b1010'0001`, and digit separators made masks easier to read and review. These were readability improvements rather than a change to the abstract arithmetic model.

**C++17.** `std::byte` introduced a distinct representation-oriented type with bitwise operators and no ordinary arithmetic. Bit containers and generic code continued to require careful handling of proxy references and type promotions.

**C++20.** Two's-complement integer representation and specified negative signed right-shift semantics strengthened the language model. The `<bit>` header brought `popcount`, scans, rotations, power-of-two functions, `bit_cast`, and `endian` into standard vocabulary. Many old implementation-specific tricks acquired readable, portable alternatives.

**C++23.** `std::byteswap` standardized integer byte reversal, simplifying explicit endian-conversion code. Source-level byte-order rules and scalar bit operations remained separate.

**C++26.** Standard data-parallel facilities in `<simd>` supplied lane and mask abstractions for vector-style computation. They complement scalar bit helpers and require real standard-library support. The shift preconditions, unsigned bit utilities, and application-specific flag contracts did not disappear.

The progression is best understood as a change in the vocabulary available to the programmer, not as a guarantee that every compiler or architecture implements every operation with the same instruction sequence.

## 18. Translating expectations from Java, Python, and C#

The syntax of bitwise operators looks familiar to developers arriving from Java, Python, or C#, but the width and overflow models can be different.

Java's `int` and `long` are fixed-width signed integer types with specified two's-complement arithmetic behavior, and the language provides both arithmetic right shift (`>>`) and logical right shift (`>>>`). Java masks shift counts rather than treating every out-of-range count like C++ undefined behavior. Copying Java shift code mechanically into C++ can therefore change semantics even when the operands have matching nominal widths.

Python integers have arbitrary precision rather than a fixed implicit machine width. A negative Python integer behaves conceptually as though it has infinitely many leading sign bits during bitwise operations. A Python mask such as `~x` therefore does not mean “invert exactly 32 bits” unless an explicit mask and width are applied. Negative shift counts raise exceptions. Converting a Python bit algorithm to C++ requires choosing a width, signedness, and overflow policy, not merely changing syntax.

C# has fixed-width integral types, checked and unchecked arithmetic contexts, and defined shift semantics for its built-in integer types. The checked/unchecked model is not equivalent to C++'s ordinary signed-overflow rules. C# also has bitwise operators and flag enums, but a port still needs to verify operand promotions, default signedness, shift-count handling, and representation boundaries.

The idiomatic C++ replacement is to choose an explicit unsigned width for representation-oriented work, validate or constrain shift counts, use `<bit>` for scans and rotations, and keep format decoding separate from memory layout. When a source language offers a runtime exception for an invalid operation, do not assume the analogous C++ expression throws rather than invoking undefined behavior.

## 19. Working rules

Bitwise programming is most reliable when three contracts remain distinct. The **language contract** determines operand promotions, arithmetic width, modulo behavior, and shift preconditions. The **representation contract** determines whether an integer corresponds to a machine word, packed storage, or external octets. The **domain contract** determines which bits signify actual flags, valid fields, permissions, or states.

Most failures come from substituting one for another. Hardware masking a shift count is mistaken for a source-language guarantee. A fixed enum width is mistaken for an authorization schema. `std::rotl` is mistaken for byte-order conversion. A branchless trick is mistaken for a performance win without a benchmark. A packed bit container is mistaken for a vector of independently addressable Boolean objects.

The practical review sequence is straightforward. First choose the integer type and make signedness explicit. Then check which promotions and conversions occur, and whether every shift count and rounded power-of-two result is in range. State the valid mask and treat unknown bits according to policy. Keep bit positions separate from serialized byte order and bit-field layout. Finally, profile the whole algorithm under realistic conditions before preserving a handwritten optimization that a standard utility already expresses.

> Low-level bitwise code is not made correct by resembling an instruction. It is made correct by preserving the language, representation, and application contracts at the same time.

## Further reading

The C++ standard's rules for [arithmetic, bitwise, and shift operators](https://en.cppreference.com/w/cpp/language/operator_arithmetic) define the core model. For the modern vocabulary consult the references for [`<bit>`](https://en.cppreference.com/w/cpp/header/bit), [bit manipulation functions](https://en.cppreference.com/w/cpp/numeric/bit), [`std::byteswap`](https://en.cppreference.com/w/cpp/numeric/byteswap), [`std::bitset`](https://en.cppreference.com/w/cpp/utility/bitset), [`std::byte`](https://en.cppreference.com/w/cpp/types/byte), and [C++26 data-parallel facilities](https://en.cppreference.com/w/cpp/numeric/simd).

Henry S. Warren Jr.'s *Hacker's Delight* offers a much deeper treatment of integer identities, bit scans, SWAR algorithms, and instruction-level techniques. [Compiler Explorer](https://godbolt.org/) is useful for seeing whether a target compiler lowers standard bit utilities to native instructions, and for comparing them with historic handwritten alternatives. The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) provide complementary guidance on type safety, interfaces, and implementation-specific low-level code.

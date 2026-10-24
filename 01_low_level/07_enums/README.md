# 07. Enumerations in Modern C++: From C++98 to C++26

An enumeration is a distinct C++ type with an integral underlying representation. That definition looks simple until one tries to answer which integer values an enum can hold, how it participates in arithmetic, why a seemingly innocuous cast can be undefined behavior, or whether a compiler can prove that a switch covers every meaningful state. The old C-style enumeration was built for named integer constants; modern scoped enumerations are intended to make semantic domains visible in the type system.

For experienced C++ developers, the difficult questions are not how to spell `enum class`, but where the language draws the boundaries between an enum's *enumerators*, its *value range*, its *underlying type*, and the *domain* accepted by an application. Those boundaries matter for protocols, bitmasks, overloads, FFI, ABI evolution, optimization, and exhaustive state machines.

This chapter develops the subject historically—from C++98 unscoped enums to C++26 compile-time reflection—while following the four source demonstrations in this directory:

1. [`01_enums_core/enums_core.cpp`](01_enums_core/enums_core.cpp) — unscoped enums, integer promotions, sizes, distinct types, and casts.
2. [`02_enums_scoped/enums_scoped.cpp`](02_enums_scoped/enums_scoped.cpp) — scoped enums, explicit representation, forward declarations, and conversions.
3. [`03_enums_operations/enums_operations.cpp`](03_enums_operations/enums_operations.cpp) — typed bit flags, overloaded operators, and masking.
4. [`04_modern_enums/modern_enums.cpp`](04_modern_enums/modern_enums.cpp) — state decoding, switches, and C++20 `using enum`.

The repository targets C++20. Code from C++23 and C++26 is either shown under explicit feature-test guards or labeled as a later-standard illustration; support for C++26 reflection is not presumed merely because a compiler accepts `-std=c++26`.

---

## 1. An enumeration is a distinct type, not an integer alias

At the machine level, an enum usually occupies storage in an integer representation. At the C++ type-system level, however, each enumeration declaration introduces a distinct type, even if two enumerations have identical enumerators and the same underlying representation. Confusing those facts leads to broken overloads and accidental cross-domain conversions.

### Two enums with the same values are still different types

Consider status and device-state values that happen to share their representation. There is no implicit assignment from one enum type to another, and this is intentional: the number alone does not define its meaning.

```cpp
#include <type_traits>

enum Status { ready = 0, failed = 1 };
enum DeviceState { offline = 0, online = 1 };

static_assert(!std::is_same_v<Status, DeviceState>);
// DeviceState d = ready; // ill-formed: distinct enum types
```

An unscoped enumerator such as `ready` participates in arithmetic through implicit integral conversion, but its enumeration type remains distinct. Two enums sharing an underlying type are not interchangeable API domains.

### Enumerators are compile-time constants, not stored fields

Each named enumerator is a constant of the enum type, usable where a constant expression is required. Its value may be implicit or explicitly assigned, and later unassigned enumerators increment the preceding value.

```cpp
enum Error { ok = 0, retry = 4, timeout, abandoned = timeout };
static_assert(timeout == 5);
static_assert(abandoned == timeout);

int retry_buffer[retry]{};  // constant bound
```

Nothing about this definition builds a runtime lookup table, reflection structure, or mapping from integers to names. Duplicate enumerator *values* are allowed; duplicate enumerator *names* in the same scope are not.

### Storage representation is not the complete semantic domain

When `sizeof(E)` is four, that alone does not mean every 32-bit pattern is a valid value of an enum without a fixed underlying type. The underlying integer *representation* and the enum's permitted *value range* are related but different concepts. Section 4 develops the distinction because it affects `static_cast`, sanitizer behavior, and wire-protocol parsing.

---

## 2. C++98 unscoped enums: convenient constants with implicit conversions

The C++98 form inherits much of C's original intent: a set of related named constants that can be passed to existing integer-oriented APIs. That convenience was essential for interoperability, but it also makes it easy to lose the intended domain.

### Enumerators enter the enclosing scope

An unscoped enum introduces its enumerator names into the surrounding scope. The enum type itself can be named and used in declarations, but the enumerators are not protected from collisions with unrelated constants.

```cpp
namespace graphics {
    enum Color { red, green, blue };
    Color current = red;
}

// enum Other { red, yellow }; // conflicts if placed in graphics
```

Namespaces and class scopes reduce collision risk, but do not replace the stronger encapsulation of scoped enumerators. Modern code can still use unscoped enums deliberately when legacy C interfaces or integral conversion behavior is part of the contract.

### Integral promotion can change both type and arithmetic behavior

An unscoped enum with a non-fixed underlying type promotes according to the integer promotion rules. It may become `int`, or another suitable integer type when its representable range does not fit. A fixed-underlying unscoped enum also permits conversion to its underlying type, with consequences for overload resolution.

```cpp
enum Position { left = 0, center = 1, right = 2 };
static_assert(std::is_same_v<decltype(left + 1), int>);

int adjusted = right + 10;
```

The result is an integer, not a newly constructed `Position`. This makes accidental arithmetic easy, and it is a reason to prefer scoped enums for states rather than numeric quantities.

### The usual arithmetic conversions are not enum-preserving

Once an unscoped enum enters a mixed arithmetic expression, the ordinary conversion machinery determines the result type. Signedness and integer rank can matter as much as the enum declaration itself. Treat warning-free arithmetic as insufficient evidence that an enum still denotes one of the application's defined states.

```cpp
enum Phase { initial = 1, final = 3 };
auto next = final + 1;  // integral result, not Phase
static_assert(std::is_integral_v<decltype(next)>);
```

If an algorithm genuinely needs modular arithmetic over a numeric domain, a plain integer or a deliberately designed wrapper may represent the requirement better than an enum.

### Unscoped enumerators can still be qualified in modern C++

C++11 permits the `E::member` spelling for unscoped enumerators. Qualification improves readability and resolves ambiguities without transforming the enum into a scoped enum.

```cpp
enum LegacyResult { success, failure };
auto result = LegacyResult::success; // C++11
```

This is particularly useful while modernizing legacy code incrementally, because the type and implicit conversions remain unchanged.

---

## 3. C++11 scoped enumerations establish a stronger API boundary

C++11 introduced `enum class` and `enum struct` as equivalent scoped-enumeration spellings. Their purpose is to prevent accidental name injection and implicit conversion to integer types. They do not, however, automatically validate external input.

### Scoped enumerators stay inside the enum scope

The distinction is immediately visible when two domains use the same common words.

```cpp
enum class Traffic { red, green };
enum class Alert { red, green };

Traffic light = Traffic::green;
// int x = light; // ill-formed
```

Scoped enums work well for state machines and typed options because an integer operation requires the programmer to explicitly cross the type boundary.

### The default underlying type of a scoped enum is `int`

The declaration `enum class Mode { idle, running };` has `int` as its underlying type. It is not an implementation-selected arbitrary integer type. The underlying type may instead be specified explicitly, subject to the rules for an integral underlying type.

```cpp
#include <cstdint>
#include <type_traits>

enum class Mode { idle, running };
enum class WireMode : std::uint8_t { idle, running };

static_assert(std::is_same_v<std::underlying_type_t<Mode>, int>);
static_assert(std::is_same_v<
    std::underlying_type_t<WireMode>, std::uint8_t>);
```

An explicit fixed type is valuable when another ABI or representation contract requires particular width and signedness. A fixed underlying type alone does not define a portable byte-order serialization format.

### Scoped types can have unnamed values

`enum class` prevents implicit integer conversions, not explicit casts or construction from certain integer values. The type still has a range tied to its fixed underlying representation, and the range can be wider than the named enumerators.

```cpp
enum class Mode : std::uint8_t { idle = 0, active = 1 };
Mode unknown = static_cast<Mode>(200); // defined, but unnamed
```

A domain parser should decide whether `200` is rejected, reserved for forward compatibility, or mapped to an explicit unknown state. Type safety protects the interface from accidental mixing; it does not provide schema validation.

---

## 4. The value range and the underlying integer type are different contracts

This is the most dangerous subtlety in classic enum code. A fixed-underlying enum and a non-fixed unscoped enum can respond differently to an integer cast—even when they appear to occupy identical storage on a particular platform.

### Non-fixed unscoped enum ranges are based on enumerator values

For an unscoped enum without a fixed underlying type, the language defines a representable range based on the smallest bit-field width sufficient for its enumerators, not necessarily the full machine range of its storage type. The range need not be the closed interval between the smallest and largest named values.

```cpp
enum Permission { read = 1, write = 2, execute = 4 };

Permission combination = static_cast<Permission>(3); // in range; unnamed
// auto invalid = static_cast<Permission>(8); // undefined behavior
```

A commonly observed implementation may retain the bits of `8` and appear to work, but the cast is not a portable operation. CWG 1766 clarified the out-of-range rule as undefined behavior; implementations and standards publications may describe its historical application differently.

### Fixed underlying types permit every representable underlying value

When the underlying type is fixed, the enum's value range is that of the underlying type. Conversion from an integer uses the corresponding underlying-type conversion first.

```cpp
enum class Permission : std::uint8_t {
    read = 1, write = 2, execute = 4
};

auto unknown = static_cast<Permission>(200); // defined unnamed value
```

This makes fixed-underlying enums suitable for preserving unknown protocol codes. It still does not make every such value *semantically valid* for an application.

### An unnamed value is not inherently an invalid C++ object

An enum object holding a representable value that lacks a name remains an enum object. That can be useful for bitmask combinations or extensible protocols. The correct classification has at least three layers: representable by the enum type; named by an enumerator; accepted by the application.

```cpp
enum class State : unsigned char { stopped = 0, started = 1 };
State s = static_cast<State>(2); // representable but unnamed

bool known(State x) {
    return x == State::stopped || x == State::started;
}
```

Conflating these layers is dangerous at trust boundaries. Validate integers *before* converting to a non-fixed enum if they may be outside its language-defined range.

### Never use an out-of-range example as a harmless demonstration

The `01_enums_core/enums_core.cpp` example demonstrates a safe in-range unnamed value: it casts `3` to a non-fixed enum whose enumerators are `1`, `2`, and `4`. The invalid out-of-range cast of `20` remains only as a warning in a comment.

The safe alternative is an in-range unnamed value on an enum whose range has room for one, or a fixed-underlying enum. A sanitizer need not diagnose every such invalid cast; correctness comes from the specified value-range contract.

---

## 5. Underlying types, layout, and ABI require explicit design

An enum is a proper object type with alignment and size, and its layout can affect binary compatibility. This is one of the few places where spelling the underlying type is not merely stylistic: it is part of an interface specification.

### An unscoped enum without a fixed underlying type leaves representation to the implementation

The implementation chooses a compatible underlying integer type that can represent the enumerators, subject to the language rules. Relying on an observed `sizeof` in one build is not sufficient to design a stable library ABI.

```cpp
enum LegacyCode { ok = 0, error = 70000 };
// sizeof(LegacyCode) is not a portable protocol width.
```

Compiler switches such as `-fshort-enums` can also affect layout decisions and break binary compatibility between compilation units. Such options belong in a documented whole-program ABI contract.

### Fixed underlying type implies matching size and representation

A fixed-underlying enum has the same size, value representation, and alignment as its specified underlying type under the standard's enum rules. It does not guarantee the host machine's byte order or the ABI calling convention of a surrounding library.

```cpp
enum class PacketKind : std::uint16_t { data = 1, ack = 2 };

static_assert(sizeof(PacketKind) == sizeof(std::uint16_t));
static_assert(alignof(PacketKind) == alignof(std::uint16_t));
```

The specific fixed-width integer typedef must exist for the target. `std::uint16_t` is not required on a machine without a suitable exact-width integer type.

### Treat public ABI, persistence format, and source API as three separate contracts

A public C++ function taking an enum has an ABI influenced by the target, toolchain, and compiler options. A stored protocol field has byte order and versioning requirements. A C++ source API has name lookup and conversion rules. Fixing an underlying type helps, but does not collapse these three independent questions into one guarantee.

A robust protocol reader extracts a specified-width integer from bytes, validates or preserves it according to the schema, and then constructs the enum. Raw `memcpy` of a struct containing enums is not a platform-independent wire format.

---

## 6. Forward declarations and opaque enum declarations are C++11 completeness tools

Before C++11, arbitrary enum types could not be forward-declared in the way class types can. C++11 introduced opaque enumeration declarations when the underlying type is fixed. Their unusual property is that the type is already complete after the opaque declaration, even though the enumerators are not yet defined.

### A scoped enum can be declared before listing enumerators

Because the default underlying type is `int`, this opaque declaration fixes enough representation information to form objects and apply `sizeof`.

```cpp
enum class ErrorCode;
static_assert(sizeof(ErrorCode) == sizeof(int));

enum class ErrorCode { success, failure };
```

This is not analogous to a forward-declared incomplete class. Code cannot refer to `ErrorCode::success` before the definition, but the representation and completeness rules allow layout-sensitive declarations.

### Unscoped opaque enums require an explicit underlying type

The C++11 declaration `enum Legacy : unsigned;` is permitted. Omitting its underlying type in an opaque unscoped declaration is not.

```cpp
enum Legacy : unsigned;
Legacy make_legacy(); // legal declaration

enum Legacy : unsigned { zero = 0, one = 1 };
```

Every subsequent definition must agree with the underlying type. This pattern can reduce header dependencies when enumerator names are needed in only a few implementation files.

---

## 7. Conversions and overload resolution expose the limits of enum type safety

The boundary between enums and integers is asymmetric. Unscoped enums convert implicitly to integers, scoped enums generally require explicit conversion, and overload resolution can prefer an exact enum match over an otherwise convenient integer overload.

### Exact enum matches outrank integral conversions

An overload set can contain both enum-aware and integer-aware functions. Passing an enum selects the exact match, but removing that overload can silently cause legacy unscoped code to call the integral overload.

```cpp
enum LegacyMode { on, off };

void configure(LegacyMode);
void configure(int);

void use() { configure(on); } // chooses configure(LegacyMode)
```

The risk is not that enums are aliases for integers; the risk is that implicit conversion makes an unintended fallback viable.

### Scoped enums do not participate in implicit arithmetic

Unlike `LegacyMode`, a scoped `enum class` does not implicitly select an `int` overload and does not accept `operator+` from the built-in arithmetic rules.

```cpp
enum class Mode { on, off };
// auto n = Mode::on + 1; // ill-formed
// configure(Mode::on);   // ill-formed without a suitable overload
```

A named explicit conversion is clearer than a C-style cast because it makes the crossing of abstraction boundaries visible in code review.

### C++17 direct-list initialization is intentionally narrow

C++17 permits direct-list initialization of a scoped enum, and of an unscoped enum with a fixed underlying type, from an integral value when the initialization rules permit it. This is not implicit conversion and is not a license for narrowing.

```cpp
enum class ByteKind : unsigned char {};
ByteKind one{1};         // C++17
// ByteKind too_large{300}; // narrowing: ill-formed
```

The rule supports small strong integer wrappers and selected low-level APIs. A named enumerator or parsing function is still preferable when an application requires a closed domain.

### C++23 `std::to_underlying` names the intended conversion

Before C++23, code typically used `static_cast<std::underlying_type_t<E>>(value)` or a short helper. The standard library now provides `std::to_underlying` in `<utility>`.

```cpp
#include <type_traits>
#include <utility>

enum class Code : unsigned { ok = 0, error = 17 };

constexpr auto raw(Code code) noexcept {
    return static_cast<std::underlying_type_t<Code>>(code); // C++14+
}
#if defined(__cpp_lib_to_underlying) && __cpp_lib_to_underlying >= 202102L
static_assert(std::to_underlying(Code::error) == 17u);
#endif
```

It extracts the representation value; it does not validate whether the enum is one of the named values.

---

## 8. Bitmask enums require a different domain model from state enums

A state enum usually denotes *one of several mutually exclusive states*. A bitmask enum usually denotes *any allowed combination* of independent flags. If both are modeled by `enum class`, their common syntax can conceal a major semantic difference.

### Choose non-overlapping bits and an unsigned underlying type

A permission flag set can use individual bits, usually with an explicitly unsigned fixed representation. A name such as `all` should mean exactly the OR of supported flags, not simply all bits of the storage type.

```cpp
enum class Permission : std::uint32_t {
    none = 0,
    read = 1u << 0,
    write = 1u << 1,
    execute = 1u << 2,
    all = (1u << 0) | (1u << 1) | (1u << 2)
};
```

An unsigned representation makes shifts and mask operations easier to reason about. Always ensure the shift count is less than the underlying width and that the left operand has a suitable unsigned type.

### Constrain bitwise operators to the intended enum

Do not add generic bitwise operators to every enum; doing so makes nonsensical operations such as `Traffic::red | Traffic::green` compile. A focused overload keeps permissions combinable without weakening unrelated state types.

```cpp
constexpr Permission operator|(Permission a, Permission b) noexcept {
    using U = std::underlying_type_t<Permission>;
    return static_cast<Permission>(
        static_cast<U>(a) | static_cast<U>(b));
}
```

A reusable flag-operator library can use an opt-in trait or concept to constrain overloads, but it must still define the legal mask and avoid unexpected argument-dependent lookup behavior.

### A raw bitwise complement usually sets unwanted bits

The complement of the numeric underlying value flips all bits, including bits with no named flag. This is not a type error because the fixed underlying enum can represent them, but it may violate the application-level mask domain.

```cpp
constexpr Permission without_read(Permission p) noexcept {
    using U = std::underlying_type_t<Permission>;
    constexpr U valid = static_cast<U>(Permission::all);
    return static_cast<Permission>(
        static_cast<U>(p) & (valid ^ static_cast<U>(Permission::read)));
}
```

The example removes a known bit without introducing unknown ones. When parsing flags from a protocol, validate `(raw & ~valid_mask) == 0` or define a deliberate rule for forward-compatible extensions.

### Testing for any flags is different from testing for all flags

A common helper returns true if *any* requested bit is present. Applications often also need a separate helper that tests whether *every* requested bit is present.

```cpp
constexpr bool contains_all(Permission actual, Permission required) {
    using U = std::underlying_type_t<Permission>;
    U a = static_cast<U>(actual), r = static_cast<U>(required);
    return (a & r) == r;
}
```

The zero-mask case follows naturally: every set contains all bits of `none`. Whether that is desirable must be settled by the API contract.

---

## 9. Switch exhaustiveness is a diagnostic policy, not a language guarantee

A `switch` on an enum is expressive precisely because the compiler can know the set of *named* enumerators. The compiler does not, however, guarantee that the switch handles every representable enum value. The best design separates missing named cases from intentionally unknown input.

### The absence of a default may reveal newly added enumerators

For internal closed state machines, a switch with explicit cases and an unreachable fallback after the switch can allow `-Wswitch` to warn when a new enumerator appears. A `default` branch can hide that diagnostic with some warning modes.

```cpp
enum class Mode { idle, running, stopped };

const char* name(Mode m) {
    switch (m) {
        case Mode::idle: return "idle";
        case Mode::running: return "running";
        case Mode::stopped: return "stopped";
    }
    return "<unknown>";
}
```

The final return is still essential because a fixed-underlying enum can hold an unnamed value. For genuinely unreachable internal states, fail explicitly instead of returning a plausible but incorrect state.

### A wire decoder should validate integers before dispatch

Do not make a `switch` over an unchecked cast the sole source of validation. Parse the incoming integer and map only accepted codes to enum values, so the distinction between known, reserved, and invalid inputs remains visible.

```cpp
#include <optional>

std::optional<Mode> decode(unsigned raw) {
    switch (raw) {
        case 0: return Mode::idle;
        case 1: return Mode::running;
        case 2: return Mode::stopped;
        default: return std::nullopt;
    }
}
```

Using `std::optional` is a C++17 option. C++98 code can express the same contract through a boolean return and output parameter. Explicit parsing is more stable than assuming every integer cast produces a meaningful enumerator.

### C++20 `using enum` is a local convenience with lookup consequences

C++20 `using enum` makes enumerators available in the current scope, including scoped-enum members. This can make a short switch or local dispatch table readable, but it can also recreate the name-collision risks avoided by scoped enums.

```cpp
void handle(Mode m) {
    using enum Mode;
    switch (m) {
        case idle: break;
        case running: break;
        case stopped: break;
    }
}
```

Prefer short local scopes. Avoid importing common enumerator names into public headers or broadly shared namespaces.

---

## 10. Template metaprogramming and generic enum constraints changed incrementally

An enum's type-level identity can be used to select overloads, control serialization, and write reusable formatting helpers. Each standard added vocabulary that reduces the need for handwritten detection code, but it remains easy to confuse a type trait with actual enumerator introspection.

### `std::underlying_type` exposes representation types, not enumerator lists

C++11 `std::underlying_type<E>` and C++14 `std::underlying_type_t<E>` are useful for converting enum values to the chosen underlying integer representation.

```cpp
template<class E>
constexpr auto representation(E e) noexcept
    -> std::underlying_type_t<E>
{
    return static_cast<std::underlying_type_t<E>>(e);
}
```

The trait does not reveal the number of enumerators, their identifiers, or gaps in their values. A general-purpose enumeration iteration library cannot be implemented from `underlying_type` alone.

### C++20 concepts can restrict templates to enumeration types

Generic formatting or serialization functions often need a deliberate constraint to avoid accepting ordinary integers by accident. `std::is_enum_v` has existed since C++17, and C++20 concepts make a constraint visible at an interface.

```cpp
template<class E>
concept Enumeration = std::is_enum_v<E>;

template<Enumeration E>
constexpr auto enum_bits(E e) {
    return static_cast<std::underlying_type_t<E>>(e);
}
```

The concept here is user-defined; C++20 does not provide a built-in `std::enum_type` concept. A constraint says what types are admitted, not whether an enum is intended as a flag set.

### C++23 `std::is_scoped_enum` separates the two language categories

C++23 adds `std::is_scoped_enum`, which allows generic code to distinguish classic unscoped enums from scoped enums without guessing from convertibility to `int`.

```cpp
#if defined(__cpp_lib_is_scoped_enum) && __cpp_lib_is_scoped_enum >= 202011L
static_assert(std::is_scoped_enum_v<Mode>);
static_assert(!std::is_scoped_enum_v<LegacyResult>);
#endif
```

This is useful for modernizing generic APIs and applying different policies to legacy integer-compatible enums. It still does not tell whether a particular scoped enum is a bitmask.

---

## 11. Enum design must distinguish validation, versioning, and serialization

Enums appear frequently in protocol fields, configuration files, persisted states, and plugin APIs. Their representation may be small, but their compatibility problems are not. This is an area where a few lines of explicit parsing often prevent expensive production failures.

### Persist stable numeric identifiers rather than declaration positions

Implicit numbering is convenient until someone inserts a new enumerator in the middle. Any protocol, file format, or external API should assign stable numeric IDs deliberately and document reserved ranges.

```cpp
enum class Message : std::uint16_t {
    hello = 1,
    payload = 10,
    goodbye = 100
};
```

Never treat enum ordinal position or contiguous naming as a portable serialization scheme. Aliases can even share the same number, defeating one-to-one inverse mappings.

### Decide whether unknown values are errors or forward-compatible extensions

A strict protocol might reject every unrecognized code. A rolling-upgrade system may need to preserve unknown fixed-underlying codes so newer messages can pass through older nodes without data loss.

These are different schemas. For strict domains, parse to a known enumerator or return an error. For extensible domains, use a representation that can retain unknown numeric values and distinguish them from known commands during dispatch.

### C interop and C++ enum class are different source-level interfaces

C headers usually use C-compatible enum declarations or integers. A C++ scoped enum is not a drop-in C type, even if its representation has the same width. Extern "C" adjusts linkage rules, not C++ type compatibility or calling convention details automatically.

For a stable C ABI, expose the exact documented scalar representation and convert at the language boundary. Avoid exporting an implementation-selected classic enum type as though its storage and ABI could never change.

### Concurrency safety does not follow from using enums

An enum can be a natural state stored in `std::atomic<State>` when the type meets the required conditions, but atomic reads and writes do not guarantee that transitions obey a protocol.

```cpp
#include <atomic>

enum class State : unsigned char { idle, active, stopped };
std::atomic<State> state{State::idle};
```

The atomic prevents data races on that object under valid use; it does not enforce legal transitions such as prohibiting `stopped -> active`. Use compare-exchange loops or a synchronized state machine when transition invariants matter.

---

## 12. C++26 static reflection changes enum introspection fundamentally

Before standard reflection, enumerator-name utilities typically relied on maintained tables, macros, generated code, or implementation tricks. These approaches can drift when enumerators are added, especially where enum values contain gaps or duplicates. C++26 reflection adds a standard compile-time mechanism for inspecting an enum's enumerators, but this requires actual compiler and library support.

### Reflection exposes enumerators rather than guessed integer ranges

C++26 reflection introduces `<meta>` and `std::meta::enumerators_of` to obtain reflections of the enumerators of an enum type in declaration order. This is categorically different from scanning all possible integral values or inferring names from vendor function-signature strings.

The following is **C++26 reflection syntax**, not C++20-compatible code:

```cpp
// C++26 only, when reflection and <meta> are supported:
#include <meta>
#include <vector>

enum class Fruit { apple = 1, pear = 8, plum = 8 };
consteval auto entries() {
    return std::meta::enumerators_of(^^Fruit);
}
```

The result describes *declarations*, which means aliases with equal numeric values may both appear. Applications must still choose whether a value-to-name conversion prefers the first declared alias, all aliases, or a canonical one.

### Name extraction and value extraction have separate semantics

A reflection of an enumerator can be used with the appropriate `std::meta` facilities to obtain identifier spelling and to generate a value-producing expression. This makes it possible to build compile-time tables without writing the same enumerator list twice.

Reflection is not a free runtime registry: it is a compile-time facility used by code-generation and constant-evaluation mechanisms. The generated tables and strings still consume program resources and may require design choices about linkage and binary size.

### Compiler and standard-library support must be feature-tested

C++26 reflection has a feature-test macro, and the library's `<meta>` interface must actually be available. The spelling and implemented subset of these facilities can evolve as toolchains adopt the standard.

Keep reflection examples in C++26-only blocks or separate demonstration sources. For the C++20 baseline of this repository, retain explicit tables or a documented external enum-reflection library, and avoid claiming that old C++ compilers can enumerate identifiers portably.

### Reflection does not replace domain validation

A reflection-generated list tells you which enumerators are declared; it does not automatically decide whether unnamed fixed-underlying values are supported extension codes. Nor does it prove that a bitmask combination is acceptable.

The same three-layer model—representable, named, accepted by the application—continues to govern enumeration correctness in C++26.

---

## 13. Cross-cutting traps worth recognizing in production code

The traps here have a common source: representing one concept as an integer does not mean every integer operation makes sense in that concept's domain.

### A cast can fail before the program reaches its validator

If an untrusted number is cast to a non-fixed unscoped enum outside its value range, the operation itself can have undefined behavior. Validating the resulting enum afterward is too late. Parse and validate the integer first, or choose a fixed-underlying representation explicitly when unnamed values must be preserved.

### A fixed underlying type does not guarantee named values

`enum class Mode : std::uint8_t` can hold unnamed values such as `200`. Robust switching, logging, and serialization must have a policy for these cases, even when every named enumerator is handled.

### A flags type is not necessarily a state type

It is sensible to combine independent flags with bitwise OR; it is usually nonsensical to OR two mutually exclusive states. Do not use one generic operator-enabling technique across all enumerations.

### A default switch case trades exhaustiveness diagnostics for fallback behavior

A `default` branch may conceal newly added enumerators from warning configurations, while omitting a fallback can make unnamed values flow past the switch. Use explicit named cases and a deliberate fallback policy chosen for the API.

### The underlying representation is not a schema

Stable protocols require explicit numeric identifiers, conversion rules, endianness, and versioning. A fixed-width enum is one useful component of that design, not the entire design.

---

## Rules worth keeping in working memory

The most durable model is to distinguish the enum *type*, its underlying *representation*, the language-defined *range*, its *named enumerators*, and the application's *accepted domain*. A strong API keeps those layers separate instead of assuming they are identical.

> An enum is a semantic type implemented using integers; the integers alone do not define its valid states.

### Core rules

1. **Prefer scoped enums for independent semantic domains.** Their lack of implicit integer conversion avoids accidental arithmetic and overload selection.
2. **Fix the underlying type when representation is a contract.** Also document serialization and ABI requirements separately.
3. **Validate before casting to a non-fixed enum.** Out-of-range conversion can be undefined behavior.
4. **Never equate an unnamed enum value with undefined behavior.** Fixed-underlying enums can legally represent unnamed values.
5. **Distinguish state enums from flag enums.** Only the latter should receive opt-in bitwise operators.
6. **Preserve exhaustiveness diagnostics without forgetting unknown inputs.** Cover named cases and define an explicit fallback.
7. **Treat reflection as introspection, not automatic business-rule validation.** C++26 enumerator lists still need domain semantics.

### Pitfalls at a glance

Each failure below is a confusion between two layers of the enum model.

| Pitfall | What happens | Do instead |
|---|---|---|
| `static_cast<PlainSmall>(20)` where non-fixed range excludes 20 | Undefined behavior | Validate source before cast; use a fixed type where appropriate |
| Assume `enum class E` has compiler-chosen underlying type | Contradicts C++11 default of `int` | Document default or explicitly select an integral underlying type |
| Assume every enum value has a declared name | Unknown or combined values remain possible | Provide validated parsing and fallback formatting |
| Use `~flag` without a mask | Produces bits outside supported flags | Mask to the declared flag universe |
| Treat a fixed-width enum as a wire layout | Byte order and compatibility remain unspecified | Serialize a specified-width integer explicitly |
| Use a default switch branch to silence warnings | Missing newly added enumerators can be overlooked | Enable enum warnings and apply a deliberate fallback strategy |

### Mental model summary

| Question | Non-fixed unscoped enum | Fixed unscoped enum | Scoped enum |
|---|---|---|---|
| Enumerators injected into enclosing scope | Yes | Yes | No |
| Implicit conversion to integers | Yes | Yes | No |
| Underlying type predetermined by declaration | No | Yes | Yes (`int` by default) |
| May legally hold unnamed values | Yes, within enum range | Yes, within underlying range | Yes, within underlying range |
| Appropriate for flag combinations | Sometimes, with care | Yes, with a flag contract | Yes, with explicit operators |
| Can be opaquely declared since C++11 | No (without fixing type) | Yes | Yes |

The key difference is not just whether conversion is implicit: the permitted value range can change the definedness of the conversion itself.

### Review checklist

When reviewing enum-heavy code, ask:

- [ ] Are all externally supplied numbers validated before any potentially out-of-range conversion?
- [ ] Is the underlying type fixed and documented wherever the ABI or protocol depends on it?
- [ ] Does every enum have an explicit policy for unknown or reserved values?
- [ ] Are bitwise operators restricted to flag types and masks restricted to supported bits?
- [ ] Can adding an enumerator trigger diagnostics at all relevant switches?
- [ ] Are enum identifiers, raw values, and protocol encodings intentionally stable?
- [ ] Are library-dependent C++23/26 features feature-tested rather than assumed?

---

## Diagnostics, useful compiler settings and extensions

Compiler diagnostics can detect incomplete switches and accidental enum/integer conversions, but they cannot infer an application's protocol policy. A warning-clean build does not prove that casting arbitrary wire values to an enum is safe or that every named value represents a supported runtime state.

### Warnings

| Compiler | Settings | Useful coverage |
|---|---|---|
| GCC | `-Wall -Wextra -Wpedantic -Wswitch -Wswitch-enum -Wconversion -Wenum-conversion` | Missing switch enumerators, suspicious conversions and extension use |
| Clang | `-Wall -Wextra -Wpedantic -Wswitch -Wswitch-enum -Wenum-conversion` | Incomplete switches and enum conversions, with toolchain-specific extra enum warnings |
| MSVC | `/W4 /permissive- /analyze` | Common enum switch and conversion diagnostics; consider C4061/C4062 configurations |

`-Wswitch-enum` can reveal omitted named cases even when a `default` branch exists, subject to compiler behavior. It cannot prove that every representable unnamed value is handled.

### Sanitizers and runtime checks

| Tool | Setting | What it contributes |
|---|---|---|
| Clang UBSan enum check | `-fsanitize=enum` | Detects some loads of enum representations outside a valid range; no business-domain validation |
| Clang/GCC UBSan | `-fsanitize=undefined` | Selected undefined behavior paths, with varying instrumentation and limitations |
| Static analysis | clang-tidy, cppcheck, MSVC `/analyze` | Suspicious casts, missing cases, and enum/integer API assumptions |
| Protocol tests | Exhaustively test allowed codes, reserved codes, and malformed inputs | Enforces the application-defined accepted domain |

A sanitizer may not catch the creation of an invalid enum value at the exact cast site. Do not use runtime instrumentation as an excuse to execute knowingly undefined conversions in example programs.

### Semantics-changing options

| Option | Effect | Recommendation |
|---|---|---|
| GCC/Clang `-fshort-enums` | Can change non-fixed enum representation and ABI | Avoid across independently built binary interfaces |
| `-std=gnu++20` versus `-std=c++20` | GNU extensions can change which declarations are accepted | Use ISO language modes for portable examples |
| Clang/GCC optimization choices | Can expose reliance on undefined out-of-range enum values | Correct invalid conversions, rather than relying on `-O0` observations |
| MSVC `/permissive-` | Tightens conformance checking | Use for portable language contracts |

These options affect source acceptance and ABI assumptions, not just speed. Review build flags whenever binaries exchange enum-containing values.

### Compiler extensions

| Extension | Toolchains | Purpose | Portable alternative |
|---|---|---|---|
| `__PRETTY_FUNCTION__`-based enum-name extraction | GCC/Clang and similar compiler-specific signatures | Attempts to discover names by instantiating templates | Explicit tables or C++26 reflection where supported |
| `__FUNCSIG__`-based extraction | MSVC | Similar compiler-specific name discovery | Explicit tables or C++26 reflection |
| Vendor flag-enum attributes | Selected compiler/toolchain environments | Diagnose intended bitmask use | Explicit bitwise operators and documented masks |
| `-fshort-enums` | GCC/Clang | Narrow implementation-selected representations | Explicit fixed underlying types at public interfaces |

A helper that depends on parsing compiler-provided function-signature strings is not standard enum reflection; isolate it, test exact supported compilers, and never treat it as a portable language guarantee.

### Libraries and tooling beyond the standard

| Library/tool | Purpose | When useful |
|---|---|---|
| magic_enum (third party) | Convenient enum names and lookup through compiler-dependent mechanisms | C++17/20 code when tradeoffs and supported enum ranges are understood |
| Better Enums / generated tables | Stable pre-reflection metadata | When the enumerator list must be reused for validation/formatting |
| clang-tidy | Static analysis | Review switch completeness and suspicious numeric conversions |
| Compiler Explorer | Compare compiler diagnostics and ABI effects | Investigate promotions and fixed-underlying representation |

Prefer explicit domain contracts, layer warnings and sanitizers on top, and keep non-standard introspection behind well-tested interfaces.

---

## Standards timeline

Enum evolution is an incremental move from integer-compatible named constants toward scoped domain types, explicit representation, and finally standardized compile-time introspection.

### C++98/03: Unscoped integer-compatible enumerations

Classic enums provide named integral constants with implicit integer conversions and implementation-selected underlying representations. Their values and promotions participate in integer expression rules; the language does not supply built-in enumeration names or iteration.

### C++11: Scoped enums and fixed underlying representations

`enum class`/`enum struct`, explicit underlying types on unscoped enums, opaque declarations, and `std::underlying_type` provide stronger type-safety and layout control. An opaque declaration with fixed underlying type yields a complete enum type even before the enumerator list appears.

### C++14: Alias convenience for underlying-type traits

`std::underlying_type_t` reduces template verbosity; no fundamental change to the enum value model occurs. Existing distinction between representation and semantic domain remains.

### C++17: Direct-list initialization from integers

Single-element direct-list initialization can initialize suitable enums from integers under constrained rules, without creating a general implicit integer conversion. `std::is_enum_v` is a convenient variable-template spelling for generic code.

### C++20: Scoped enumerator imports and generic constraints

`using enum` introduces enumerator names into a selected scope; concepts make it easier to constrain enum-generic APIs. The old implicit-conversion rules for unscoped enums and the separate notion of unnamed values still apply.

### C++23: Standard underlying-value extraction and scoped-enum traits

`std::to_underlying` in `<utility>` provides a named integer conversion, while `std::is_scoped_enum` identifies scoped enumeration types in generic code. Neither offers general enumerator-list introspection.

### C++26: Static reflection and enumerator discovery

C++26 static reflection introduces `<meta>` and `std::meta::enumerators_of`, permitting compile-time access to declared enumerators and supporting generation of mapping utilities. This is a genuine advance beyond hand-maintained tables, although real availability requires compiler and standard-library support and reflection does not validate business-domain values.

When discussing support, name the precise standard and the feature-test conditions used by the example instead of assuming every compiler labeled C++26 has the full library.

---

## Migration note for Java / Python / C# developers

Languages with managed enum-like classes often provide runtime access to named members and convenience APIs for strings, membership, or reflection. C++ enums historically supply an efficient typed integer domain, not an automatically maintained runtime registry. Modern scoped enums improve type safety, but do not by themselves enforce a closed set of named values.

### Where intuition transfers and where it breaks

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| Enumeration model | Class-based enum constants | `Enum` / `IntEnum` objects | Value-type `enum` with fixed underlying integer | Distinct enum type with scoped or unscoped enumerators |
| Unnamed integer value | Not generally a Java enum instance | `Enum` construction typically rejects unknown values | Explicit casts can produce unnamed values | Fixed-underlying enum can represent unnamed values |
| Listing members | `values()`/reflection | Iteration and `__members__` | `Enum.GetValues` | No portable built-in listing before C++26 reflection |
| Integer conversion | Explicit numeric fields by design | `IntEnum` integrates with int | Explicit enum/numeric conversion | Implicit for unscoped; explicit for scoped |
| Invalid range | Managed validation and exceptions | Exceptions for unrecognized `Enum` values | Underlying integral value can be cast | Out-of-range conversion to non-fixed enums may have UB |

### Common wrong assumptions

An enum class in Java behaves like a closed collection of singleton objects; a C++ `enum class` does not guarantee that every representable value has a named enumerator. A Python `Enum` constructor often raises for an unknown integer; the corresponding C++ cast may produce an unnamed value or even undefined behavior for a non-fixed enum. C# programmers will recognize unnamed enum values but must still account for C++'s distinct range rules and ABI.

### Idiomatic C++ replacement

| Managed-language habit | Idiomatic C++ |
|---|---|
| Query names/values automatically | Maintain a constexpr table, use a tested library, or C++26 reflection |
| Rely on constructor rejection of unknown codes | Parse integers explicitly and return `std::optional<E>` or an error result |
| Combine flags through general integer operators | Define constrained typed flag operations and a valid mask |
| Persist ordinal positions | Use explicit stable underlying values and a defined byte serialization |
| Depend on exhaustive enum dispatch | Use warning-backed switches with a deliberate unknown-value fallback |

The important adaptation is to encode the intended *domain* in APIs rather than expecting the runtime to infer it from enumerator spelling.

---

## Further reading

These references are useful for checking standard wording on ranges, conversions, enum declarations, and reflection; the worked examples above show how the pieces interact.

### Standard and language reference

* [Enumeration declarations and conversions](https://en.cppreference.com/w/cpp/language/enum)
* [Explicit `static_cast` to an enum](https://en.cppreference.com/w/cpp/language/static_cast)
* [`std::underlying_type`](https://en.cppreference.com/w/cpp/types/underlying_type)
* [`std::to_underlying` (C++23)](https://en.cppreference.com/w/cpp/utility/to_underlying)
* [`std::is_scoped_enum` (C++23)](https://en.cppreference.com/w/cpp/types/is_scoped_enum)
* [`std::meta::enumerators_of` (C++26)](https://en.cppreference.com/w/cpp/meta/enumerators_of)

### Proposals and guidelines

* [P2996 — Reflection for C++26](https://wg21.link/P2996)
* [C++ Core Guidelines, Enum: Enumerations](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-enum)
* [WG21 CWG 1766: Out-of-range enum conversions](https://cplusplus.github.io/CWG/issues/1766.html)

### Articles and tools

* [Compiler Explorer](https://godbolt.org/) — compare warnings, representations, and optimization assumptions
* [magic_enum](https://github.com/Neargye/magic_enum) — example of third-party pre-reflection enum inspection with implementation constraints

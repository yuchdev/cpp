# Chapter 07. Enumerations: Types, Values, and the Boundaries of Meaning

A program may represent a device state as the number 2, a network command as the number 2, and a file permission mask as the number 2. The bits may be identical, but the three values do not mean the same thing. This is the central reason enumerations exist. An enumeration gives a numerical representation a distinct C++ type and a set of named constants, allowing code to express the *domain* in which the number is meaningful.

The complication is that the domain is not necessarily the list of named constants. An enumeration can have values with no enumerator name. Its representable range can differ from the range of its underlying storage type. For classic unscoped enums, arithmetic may silently discard the enum type altogether; for scoped enums, explicit conversion is required, but an explicit conversion still does not validate the value against an application schema. These distinctions become critical in protocol decoding, bitmask design, cross-platform ABIs, exhaustive state machines, and generic libraries.

This chapter follows four companion examples: [core enumerations](01_enums_core/enums_core.cpp), [scoped enumerations](02_enums_scoped/enums_scoped.cpp), [enum operations and bitmasks](03_enums_operations/enums_operations.cpp), and [modern enum patterns](04_modern_enums/modern_enums.cpp). The repository targets C++20, so C++23 and C++26 facilities are identified separately.

## 1. A distinct type with an integral representation

Consider two seemingly similar definitions:

~~~cpp
enum Status { ready = 0, failed = 1 };
enum DeviceState { offline = 0, online = 1 };

static_assert(!std::is_same_v<Status, DeviceState>);
~~~

Both enumerations might use identical integer storage, but they are distinct types. This is not a cosmetic difference. A function expecting a device state should not automatically accept an error status simply because both happen to use 0 and 1.

The type relationship is asymmetric: an unscoped enum can often convert *outward* to an integer, while an arbitrary integer cannot implicitly convert *into* that enum. Thus:

~~~cpp
int code = ready;        // permitted unscoped conversion
// DeviceState d = ready; // no implicit enum-to-enum conversion
~~~

Each named enumerator is a constant expression of the enumeration's type. Its value can be explicitly specified or derived from the preceding enumerator:

~~~cpp
enum Error {
    ok = 0,
    retry = 4,
    timeout,                 // 5
    abandoned = timeout      // another name for 5
};

static_assert(timeout == 5);
static_assert(abandoned == timeout);
int retry_buffer[retry]{};
~~~

An enumeration definition does not create runtime fields or a map from integers to names. The enumerators are compile-time constants, and their numeric values need not be unique. In the example, both `timeout` and `abandoned` denote 5. If a serializer later converts a value of 5 into a name, it must choose which name to report. The language cannot infer that application policy.

At this point it is useful to separate three concepts that will recur throughout the chapter.

The **underlying type** determines a compatible integral representation and matters for storage and ABI. The **enumeration value range** determines which values an enumeration object can represent under the language rules. The **application domain** determines which of those values the program accepts as meaningful states, flags, or protocol codes. The set of **named enumerators** is yet another set: it may cover the application domain, include aliases, leave gaps, or deliberately omit future extension values.

A correct design should never assume that these sets are necessarily identical.

## 2. The C++98 unscoped enumeration and its costs

The original C++ enumeration was designed as a convenient way to give names to related integer constants while remaining friendly to integer-oriented code. Its enumerators enter the enclosing scope:

~~~cpp
namespace graphics {
    enum Color { red, green, blue };
    Color background = red;
}
~~~

This is short and familiar, but a second unscoped enum in the same namespace cannot declare another `red` without a name collision. The old design also encourages accidental mixing with arithmetic. Enumerators of unscoped enums participate in integral promotions:

~~~cpp
enum Position { left = 0, center = 1, right = 2 };

auto shifted = right + 10;
static_assert(std::is_same_v<decltype(shifted), int>);
~~~

The resulting value is an `int`, not a `Position`. No automatic “next valid position” operation has been performed; the usual arithmetic conversions have simply taken over.

The exact promotion rules depend on the enum and its permitted range. A non-fixed unscoped enum can promote to `int` or to another suitable integer type according to those rules, and a fixed-underlying unscoped enum has its own underlying-type conversion implications. Do not assume every unscoped enum expression always evaluates as `int`, particularly with wide unsigned enumerator values or overload sets.

~~~cpp
enum Phase { initial = 1, final = 3 };
auto n = final + 1; // integer expression, not another Phase
~~~

This also affects overload resolution:

~~~cpp
enum LegacyMode { on, off };

void configure(LegacyMode);
void configure(int);

configure(on); // exact enum overload
~~~

The enum overload wins by an exact match. But if it disappears during a refactoring, the integer overload remains viable through implicit conversion; legacy source can silently change which function is called. That is an API hazard distinct from explicit enum-to-integer conversion.

C++11 allowed an unscoped enumerator to be named with its enum qualifier:

~~~cpp
enum LegacyResult { success, failure };
auto result = LegacyResult::success;
~~~

Qualification improves readability without changing the fundamental unscoped type or its integral conversions. It can be useful during incremental modernization, but is not equivalent to introducing a scoped enum.

The old form is still useful for compatibility with historical interfaces, constants that deliberately participate in integer expressions, and some C-facing code. The important question is whether implicit integer behavior is intended. If it is not, a scoped enumeration expresses the constraint more reliably.

## 3. C++11 scoped enums establish a deliberate conversion boundary

C++11 introduced `enum class` and the equivalent spelling `enum struct`. Their enumerators remain in the enum's scope and do not implicitly convert to integers:

~~~cpp
enum class Traffic { red, green };
enum class Alert { red, green };

Traffic signal = Traffic::green;
// int n = signal;            // ill-formed
// signal = Alert::green;     // ill-formed
// auto next = signal + 1;    // ill-formed
~~~

This makes scoped enums well suited to mutually exclusive states, commands, results, and typed configuration parameters. Name collisions are avoided without scattering common names such as `on`, `off`, or `error` into an enclosing namespace.

An ordinary scoped enum has `int` as its default underlying type. Unlike an unscoped enum without a fixed underlying type, the representation is fixed at the declaration:

~~~cpp
enum class Mode { idle, running };
static_assert(std::is_same_v<
    std::underlying_type_t<Mode>, int>);
~~~

A different underlying integer type can be specified explicitly:

~~~cpp
enum class WireMode : std::uint8_t {
    idle = 0,
    running = 1
};
~~~

This is useful at binary and ABI boundaries, where the representation's width and signedness are intentional rather than incidental. The exact-width integer typedef must, of course, be provided by the target implementation.

Yet scoped enums should not be mistaken for closed algebraic data types that can hold only named states:

~~~cpp
enum class Mode : std::uint8_t {
    idle = 0,
    active = 1
};

Mode other = static_cast<Mode>(200); // defined, unnamed value
~~~

The explicit conversion is legal because 200 is representable in the fixed underlying type. The code has not made `200` a named state, and it has not established that an application should accept it.

This is one of the most important differences between *type safety* and *input validation*: `enum class` blocks accidental implicit conversions, but explicit casts and externally obtained representations can still produce semantically unexpected values.

## 4. The dangerous difference between a value range and a storage width

Suppose a classic unscoped enumeration has no fixed underlying type:

~~~cpp
enum Permission {
    read = 1,
    write = 2,
    execute = 4
};
~~~

One might assume that if `sizeof(Permission)` happens to be four, every 32-bit integer could safely be cast to `Permission`. That reasoning is incorrect.

For an unscoped enum without a fixed underlying type, the permitted value range is determined through the enum-range rules based on the enumerator values, not simply by declaring every bit pattern of the implementation-selected underlying storage type to be valid. For the example, the enumerators require enough bits to represent 4, yielding a range that can include unnamed values such as 3 and 7:

~~~cpp
Permission combined = static_cast<Permission>(3); // valid unnamed value
// Permission invalid = static_cast<Permission>(8); // undefined behavior
~~~

The second conversion lies outside the enumeration's value range. Modern specifications, including the resolution of CWG 1766, make an out-of-range conversion to such an enum undefined behavior. The fact that a particular CPU can store 8 in the same number of bytes is irrelevant. If a cast can already invoke undefined behavior, validating the enum *after the cast* is too late.

Now compare a fixed-underlying enumeration:

~~~cpp
enum class PermissionBits : std::uint8_t {
    read = 1,
    write = 2,
    execute = 4
};

auto raw = static_cast<PermissionBits>(200);
~~~

Here the enum's permitted values correspond to its fixed underlying type's range. The conversion to a fixed-underlying enumeration proceeds through that underlying type, so the value 200 is well-defined as an enum value even though it is unnamed. An integer value outside the underlying type's range follows the underlying integer conversion rules before becoming the enum value.

The crucial categories are therefore:

1. **Representable by the language-defined enum type.** Does construction or conversion produce a permitted value?
2. **Named by an enumerator.** Is there a declared name for that value?
3. **Accepted by the application.** Does the schema, state machine, or flag policy allow it?

These are not equivalent. Unnamed values are essential for many bitmask combinations and forward-compatible protocols, but may be unacceptable in a closed state machine.

### 4.1 Do not assume consecutive enumerators

Implicitly assigned values are consecutive only within the stretches where the definition uses default numbering. Explicit values may create gaps, aliases, and large jumps:

~~~cpp
enum class Opcode : unsigned {
    hello = 1,
    payload = 10,
    goodbye = 100
};
~~~

Neither the declaration order nor the number of enumerators determines an exhaustive numeric interval of valid application commands. A loop from `hello` to `goodbye` is not an enum iterator. Even C++26 reflection, as we shall see, operates on declarations rather than inventing a domain policy for gaps.

## 5. Underlying types and binary compatibility

An enumeration is a real object type with size, alignment, and representation. For a classic unscoped enum without an explicit underlying type, the implementation chooses a suitable integral underlying type under the language's constraints:

~~~cpp
enum LegacyCode {
    ok = 0,
    error = 70'000
};
~~~

One build's observed `sizeof(LegacyCode)` is not a portable wire width. Compiler settings, including GCC/Clang-style `-fshort-enums` options where supported, can influence enum layout and make separately compiled libraries binary-incompatible.

A fixed underlying type provides a stronger representation guarantee:

~~~cpp
enum class PacketKind : std::uint16_t {
    data = 1,
    ack = 2
};

static_assert(sizeof(PacketKind) == sizeof(std::uint16_t));
static_assert(alignof(PacketKind) == alignof(std::uint16_t));
~~~

For fixed-underlying enums, the relevant size, alignment, and value representation follow that underlying type. It remains necessary to consider the platform's calling convention, struct padding, byte order, and packing policies.

It is helpful to keep three contracts separate. The **source interface** decides name lookup, implicit conversion, overload resolution, and generic type distinctions. The **ABI** decides how a parameter or member is passed and laid out by a particular toolchain. The **persistent or wire representation** decides exact field width, byte order, reserved values, and version negotiation. Writing `enum class : std::uint16_t` addresses part of the first two concerns; it does not single-handedly define the third.

A portable reader for a network field should extract the specified-width integer using the format's byte order, validate the accepted codes, and then convert to the enum. Copying a native struct containing an enum into a packet does not create a general cross-platform binary protocol.

## 6. Opaque declarations and compilation boundaries

C++11 introduced an unusual but valuable facility: an enumeration can be declared without yet defining its enumerators if its underlying type is fixed. The enumeration is already a complete type after that opaque declaration, even though the enumerator names are not available:

~~~cpp
enum class ErrorCode;
static_assert(sizeof(ErrorCode) == sizeof(int));

enum class ErrorCode {
    success,
    failure
};
~~~

The default `int` underlying type of a scoped enum supplies enough information for the first declaration to determine the representation. This differs from a forward-declared class, which remains incomplete until its definition.

An unscoped opaque enum must explicitly specify the underlying type:

~~~cpp
enum Legacy : unsigned;
Legacy make_legacy();

enum Legacy : unsigned {
    zero = 0,
    one = 1
};
~~~

The later definition must agree about the underlying type. This feature can reduce compilation dependencies when a header only needs the type in signatures or data layouts and the actual named values belong in another header or implementation unit.

The distinction is subtle but useful in large systems: one can depend on an enum's representation without depending on the whole set of enumerator declarations. Conversely, code that names `ErrorCode::success` still requires the definition where that enumerator is known.

## 7. Explicit conversions and their limits

For an unscoped enum, implicit conversion to an integer is part of the old model. For a scoped enum, the boundary normally requires `static_cast`:

~~~cpp
enum class Code : unsigned {
    ok = 0,
    error = 17
};

auto n = static_cast<unsigned>(Code::error);
~~~

Generic code should not guess the underlying type from the compiler's observed size:

~~~cpp
template<class E>
constexpr std::underlying_type_t<E> representation(E value) noexcept
{
    return static_cast<std::underlying_type_t<E>>(value);
}
~~~

The `std::underlying_type` trait arrived in C++11, with `std::underlying_type_t` in C++14. It reports the underlying representation type; it does not produce enumerator names or validate the value.

C++23 provides a standard spelling for the same intent:

~~~cpp
#include <utility>

auto code = std::to_underlying(Code::error); // 17u
~~~

Use the feature-test macro `__cpp_lib_to_underlying` where the repository still builds with C++20. The utility extracts the underlying value but has no schema-awareness.

### 7.1 Direct-list initialization is deliberately constrained

C++17 permits certain integral direct-list initializations of a scoped enum or a fixed-underlying unscoped enum, provided the list-initialization rules do not forbid the conversion:

~~~cpp
enum class ByteKind : unsigned char {};

ByteKind one{1};           // C++17
// ByteKind invalid{300};  // narrowing
~~~

This is useful for low-level typed integer wrappers where an enum is intentionally used to prevent implicit mixing with other scalar types. But it does not turn arbitrary enum declarations into validated state types. A named enumerator or explicit parser remains preferable when only a closed set of values is accepted.

### 7.2 Overload design should make the crossing obvious

For classic enums, an integer overload can unintentionally act as a fallback; scoped enums avoid that implicit conversion. However, a function that accepts `unsigned` may still tempt callers to write a cast at every call site, pushing domain checks out of the public API. A better approach is often to expose both a typed operation and an explicitly named decoder:

~~~cpp
void dispatch(Code code);
std::optional<Code> decode_code(unsigned input);
~~~

Callers dealing with trusted internal states use the typed function. Callers handling external integers must cross a validation boundary. This is clearer than accepting raw integers everywhere and treating a cast as proof of correctness.

## 8. An enum of flags is not an enum of states

A state enumeration commonly represents one of several mutually exclusive alternatives. A bitmask enumeration instead represents a combination of independent Boolean features. Both can be declared with `enum class`, but their valid operations are different.

For a permission mask, choose nonoverlapping bits in an unsigned representation:

~~~cpp
enum class Permission : std::uint32_t {
    none    = 0,
    read    = 1u << 0,
    write   = 1u << 1,
    execute = 1u << 2,
    all     = (1u << 0) | (1u << 1) | (1u << 2)
};
~~~

The named value `all` means “every flag supported by this domain,” not every bit of a 32-bit integer. Reserving high bits for future features is part of the format and versioning contract.

Because scoped enums have no implicit bitwise arithmetic, operations must be defined deliberately:

~~~cpp
constexpr Permission operator|(Permission a, Permission b) noexcept
{
    using U = std::underlying_type_t<Permission>;
    return static_cast<Permission>(
        static_cast<U>(a) | static_cast<U>(b));
}
~~~

This overload is appropriate for permissions but would be nonsense for `Traffic::red | Traffic::green`. Avoid defining universal `operator|` and `operator&` for every enum simply because generic templates can do so. A reusable flag library should opt types in through an explicit trait or concept and specify which values and masks are allowed.

### 8.1 Complement, exclusion, and unknown bits

A naïve unary complement flips every bit of the underlying integer:

~~~cpp
// ~read would set all bits except the read bit
~~~

For a fixed-underlying flag type that might be language-valid yet application-invalid: it introduces unnamed bits that the schema never assigned. Mask the complement against the supported domain:

~~~cpp
constexpr Permission without_read(Permission p) noexcept
{
    using U = std::underlying_type_t<Permission>;
    constexpr U valid = static_cast<U>(Permission::all);
    constexpr U read_bit = static_cast<U>(Permission::read);

    return static_cast<Permission>(
        static_cast<U>(p) & (valid ^ read_bit));
}
~~~

The operation deliberately removes `read` while dropping unsupported bits. If the application instead needs to preserve future flags, this is a different policy; blindly clearing all unknown bits can be as harmful as accepting them.

The phrase “contains flags” also conceals two distinct predicates:

~~~cpp
constexpr bool contains_all(Permission actual,
                            Permission required) noexcept
{
    using U = std::underlying_type_t<Permission>;
    const U a = static_cast<U>(actual);
    const U r = static_cast<U>(required);
    return (a & r) == r;
}

constexpr bool contains_any(Permission actual,
                            Permission requested) noexcept
{
    using U = std::underlying_type_t<Permission>;
    return (static_cast<U>(actual) &
            static_cast<U>(requested)) != 0;
}
~~~

The first succeeds when every requested bit is present. The second succeeds when at least one is present. With a zero required mask, `contains_all` is true for every value, while `contains_any` is false. That algebraic result is sensible, but an API should document it if the caller may expect “none” to be handled specially.

Shift expressions must obey the integer rules developed earlier in the book. The left operand should be an appropriate unsigned integer type and the shift count must be within its width. A fixed underlying type also does not exempt flags from validation: receiving `0x80000000` over the wire does not make that flag supported merely because it fits in `std::uint32_t`.

## 9. State machines, switches, and exhaustive handling

A `switch` is attractive for enum-based state machines because every named alternative can appear as a distinct case. But a switch over an enum does not automatically validate that the value is one of those named alternatives.

~~~cpp
enum class Mode {
    idle,
    running,
    stopped
};

const char* name(Mode mode)
{
    switch (mode) {
        case Mode::idle:    return "idle";
        case Mode::running: return "running";
        case Mode::stopped: return "stopped";
    }
    return "<unknown>";
}
~~~

The fallback matters because the fixed-underlying scoped enum may contain unnamed values. For a closed internal state machine, the fallback could instead report a violated invariant or terminate. For a protocol-facing parser, it might return an error or preserve the raw value. Silently choosing one of the known states is generally unsafe.

### 9.1 The `default` trade-off

Compiler warnings such as `-Wswitch` can detect that a newly added named enumerator is missing from a switch. A `default` case often suppresses that specific diagnostic, although compiler modes such as `-Wswitch-enum` may still report omissions. Therefore a switch with every named case and a fallback *after* the switch can be useful for internal code: it gets exhaustive-case diagnostics and still defines behavior for unnamed values.

This is a diagnostic strategy, not a guarantee supplied by the language. C++ does not forbid a switch from receiving an unnamed fixed-underlying value. Nor does an exhaustive list of named cases mean the application has completely handled a future version of an external protocol.

### 9.2 Validate before dispatch

For external input, a decoder should accept a specified integer representation and translate only recognized commands:

~~~cpp
#include <optional>

std::optional<Mode> decode(unsigned raw)
{
    switch (raw) {
        case 0: return Mode::idle;
        case 1: return Mode::running;
        case 2: return Mode::stopped;
        default: return std::nullopt;
    }
}
~~~

This makes trust boundaries explicit and avoids out-of-range casts to classic non-fixed enums. For extensible protocols, a decoder might instead return a variant of a known enum and an unknown numeric code, preserving forward-compatible information without accidentally dispatching it as a known command.

C++20's `using enum` can shorten local code:

~~~cpp
void handle(Mode m)
{
    using enum Mode;

    switch (m) {
        case idle:    break;
        case running: break;
        case stopped: break;
    }
}
~~~

The convenience has a cost: enumerator names become visible in that scope, reintroducing some collision opportunities. Keep imports local, particularly when the enumerators are commonplace words such as `error` or `open`. Broad use in public headers can make lookup fragile.

## 10. Generic enumeration programming

A strongly typed API can benefit from generic serialization, logging, and formatting helpers. But the language's type traits historically exposed an enum's **representation**, not the list of its declared identifiers.

The basic test is `std::is_enum_v<T>` (variable-template spelling available since C++17). C++20 makes such constraints readable with a user-defined concept:

~~~cpp
template<class T>
concept Enumeration = std::is_enum_v<T>;

template<Enumeration E>
constexpr auto raw_value(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
~~~

There is no built-in standard `std::enum_type` concept in C++20. The user-defined concept merely asserts that `T` is an enumeration; it says nothing about whether it is a state, permission mask, protocol opcode, or bit field.

C++23's `std::is_scoped_enum` allows code to distinguish scoped from unscoped enums without guessing based on convertibility to `int`:

~~~cpp
#if defined(__cpp_lib_is_scoped_enum) && \
    __cpp_lib_is_scoped_enum >= 202011L
static_assert(std::is_scoped_enum_v<Mode>);
static_assert(!std::is_scoped_enum_v<LegacyResult>);
#endif
~~~

The distinction is useful when modernizing integer-compatible APIs or applying different parsing rules to old and new enum categories. Yet a scoped enum is not necessarily a flags type, and a classic enum is not necessarily a numerical quantity. Policies should be attached to the actual domain, not inferred from syntax alone.

Before language reflection, generic enumerator-name libraries required maintained tables, macros, code generation, compiler-specific string extraction, or value-range scanning. Each approach has blind spots. Gaps and aliases defeat “all values from zero to maximum” assumptions; vendor function-signature parsing is not standard C++; tables can drift when a declaration changes.

A production design can select an explicit mapping table and test it against the intended source of truth. Such duplication can be acceptable when stability and cross-language interoperability matter more than eliminating maintenance, particularly if schema generation keeps the files synchronized.

## 11. Serialization, schema evolution, and ABI boundaries

An enum commonly appears in precisely the places where an incorrect assumption is expensive: persisted state, interprocess messages, configuration files, plugin interfaces, shared-memory layouts, and network packets.

If a serialized value must remain stable, assign explicit IDs:

~~~cpp
enum class Message : std::uint16_t {
    hello   = 1,
    payload = 10,
    goodbye = 100
};
~~~

Inserting a new declaration does not now silently renumber the existing codes. Reserve numeric ranges where forward compatibility demands it, document deprecated identifiers, and specify whether several names can share one code.

A persisted enum field should not be described merely as “whatever `sizeof(E)` bytes the C++ compiler uses.” The protocol needs exact integer width, byte order, treatment of unknown codes, and version policy. Parsing a little-endian 16-bit network field and passing it through `std::to_underlying` are opposite operations: the former interprets external bytes under an explicit format; the latter extracts the already stored enum's underlying numerical value.

### 11.1 Strict and extensible domains need different decoders

A closed finite-state machine may reject every unknown code. A rolling-upgrade network system may need to retain an unrecognized code to forward it transparently. A configuration parser may offer a warning and a documented fallback. Each policy is valid in a different domain.

A fixed-underlying enum is convenient for preserving unnamed values, but using that capability implicitly can be misleading. Consider a typed result that distinguishes recognized commands from unknown raw values:

~~~cpp
struct UnknownMessage {
    std::uint16_t code;
};

using ParsedMessage = std::variant<Message, UnknownMessage>;
~~~

The parser can preserve future extensions while ensuring that business logic expecting `Message` handles only recognized named values. This is often more robust than casting every incoming number to an enum and hoping each subsequent switch contains a defensive `default`.

### 11.2 C and C++ interfaces

C++ scoped enums are not direct C source-language types. Even when a fixed-underlying enum and an integer have matching representation, a C ABI should normally expose explicitly specified integer parameters or C-compatible declarations, then convert into a scoped C++ enum at the boundary.

`extern "C"` changes language linkage; it does not magically permit a C compiler to parse `enum class` or guarantee that a C++ type's calling convention is equivalent to an arbitrary C declaration. Cross-platform libraries should record ABI expectations, compiler options, and the serialization schema separately.

### 11.3 Enumerations and concurrency

An enum can be stored in `std::atomic<State>` when the atomic template requirements are satisfied:

~~~cpp
enum class State : unsigned char {
    idle,
    active,
    stopped
};

std::atomic<State> state{State::idle};
~~~

Atomic operations on the enum avoid data races on that atomic object. They do not encode legal state transitions. A thread can still store `active` after another thread has stored `stopped` unless the protocol forbids it through synchronization and validated compare-exchange logic. A typed state does not by itself make a state machine correct.

For lock-free software, this distinction is especially important. The domain of possible enum values is finite, but the protocol may require monotonic transitions, generation counters, publication ordering, or lifetime guarantees for objects associated with each state.

## 12. C++26 reflection: enumerators become inspectable declarations

The history of enum introspection changes substantially with C++26 static reflection. Previously, standard C++ could ask for an enum's underlying type but not ask the compiler to enumerate all declarations or retrieve their source-level identifiers. Standard reflection adds a compile-time way to describe program entities, including enumerators.

C++26's `<meta>` vocabulary includes `std::meta::enumerators_of`, which can obtain a sequence of enumerator reflections in declaration order. For illustration, the following is **C++26-only syntax**, not valid in the repository's C++20 baseline:

~~~cpp
// C++26 reflection, when supported:
#include <meta>

enum class Fruit {
    apple = 1,
    pear = 8,
    plum = 8
};

consteval auto entries()
{
    return std::meta::enumerators_of(^^Fruit);
}
~~~

The important change is conceptual. Reflection discovers *declarations*, not just a guessed integer range. Both `pear` and `plum` may be represented as distinct enumerator declarations even though they have the same numeric value. That allows a generator to produce name tables and value tables without manually duplicating the source list.

It also introduces new design questions. If a value has two names, which is the canonical formatter output? Should unknown values be displayed numerically, preserved, or rejected? How much compile-time metadata should become runtime strings or lookup structures? A reflection-generated mapping is still an implementation of a domain policy, not a replacement for one.

Name extraction and value extraction are distinct reflection operations, involving the appropriate `std::meta` identifier and value/splice facilities. A compiler accepting the broad language flag `-std=c++26` does not guarantee that the implemented compiler and standard library ship every reflection facility. Feature tests and isolated C++26 demonstrations are necessary. For a C++20-based book project, explicit tables or a carefully selected external library remain the appropriate executable examples.

Reflection does not change the fact that fixed-underlying enums can hold unnamed values. It can describe the enumerator declarations, but cannot decide whether the application should accept a future raw code or an unknown flag combination.

## 13. Diagnostics and compiler-specific behavior

Because enums look like integers at the machine level, a dangerous conversion can compile cleanly and may even behave as intended in a small test. Diagnostic tools should be chosen to catch specific boundary failures.

GCC and Clang warning families such as `-Wall -Wextra -Wpedantic`, `-Wswitch`, `-Wswitch-enum`, `-Wenum-conversion`, and `-Wenum-compare` (where available) identify different risks: missing named cases, suspicious arithmetic, and mixing enum types. Some warnings are enabled implicitly by broader warning groups; exact behavior is compiler-version dependent. MSVC `/W4 /permissive-` and `/analyze` provide corresponding language and static-analysis feedback.

The difference between `-Wswitch` and `-Wswitch-enum` can be architecturally useful. One encourages explicit enumeration of cases that a `default` might conceal; the other can warn about missing enumerators even with a default, depending on implementation. A compiler warning is a maintenance guardrail, not a runtime validator of every fixed-underlying enum value.

UndefinedBehaviorSanitizer's enum checks, exposed through options such as `-fsanitize=enum` where supported, may detect executed invalid loads or conversions involving enum values outside the language-permitted range. Coverage is incomplete, and a fixed-underlying enum containing an unnamed but representable value is not necessarily undefined behavior. Static analyzers such as clang-tidy, cppcheck, and MSVC analysis can help find questionable casts and incomplete switch handling, but still cannot infer the application's protocol schema.

Compiler options that select shorter enum representations, particularly `-fshort-enums`, can change layouts and ABIs. GNU dialect modes may accept extensions or nonportable constructs that strict ISO modes reject; prefer `-std=c++20` plus suitable pedantic diagnostics for the repository baseline. Build flags should be considered part of the documented binary interface wherever enum layout crosses compilation units.

Vendor reflection or enum-name tricks built on function signature strings, such as `__PRETTY_FUNCTION__` or `__FUNCSIG__`, are implementation-specific. They can be practical inside a version-pinned codebase, but must not be presented as standard enumeration reflection. Libraries such as magic_enum can offer convenience under stated compiler support and value-range restrictions; generated schema tables or manual maps may be more appropriate for public protocols.

Compiler Explorer is valuable for comparing storage and overload results across compilers. For source-level design, the C++ Core Guidelines help frame the choice between a scoped state, an unsigned flag mask, and a representation-oriented integer field. Test suites should explicitly cover named values, unnamed in-range values, non-fixed range boundaries, unknown wire codes, aliases, and switching behavior after adding a new enumerator.

The general testing principle is to avoid executing undefined casts merely to see what happens. Probe the valid range, document the invalid case, and let compiler diagnostics and sanitizers supplement—not replace—the language model.

## 14. How the enumeration model evolved

**C++98/03.** Classic unscoped enumerations provided distinct types with named integer constants and implicit promotions to integer types. Underlying representations were implementation-selected for enums without fixed underlying types. This form suited C-era interfaces but readily lost domain information in arithmetic.

**C++11.** Scoped enums (`enum class`/`enum struct`) introduced strong scope and explicit integer-conversion boundaries. Fixed underlying types became directly expressible, opaque enum declarations became useful completeness tools, and `std::underlying_type` provided generic representation discovery. Scoped enums also established `int` as their default underlying type.

**C++14.** `std::underlying_type_t` simplified the common trait spelling. The foundational conversion and value-range rules remained unchanged.

**C++17.** Restricted direct-list initialization of suitable fixed-underlying enums from integral values provided a practical way to construct strong integer-like wrappers without implicit integer conversions. The narrowing restrictions still apply.

**C++20.** `using enum` made local import of enumerator names possible. Concepts improved expression of generic enum constraints; the language did not thereby acquire a standard API for obtaining enumerator names.

**C++23.** `std::to_underlying` supplied a named standard conversion for enum representation values, and `std::is_scoped_enum` distinguished scoped from unscoped enums in generic code.

**C++26.** Static reflection introduced facilities for compile-time enumerator discovery through `<meta>`, including `std::meta::enumerators_of`. Real compiler and library support must be feature-tested; the underlying enum value and application validation rules did not disappear.

The evolution has made it increasingly easy to preserve semantic types, examine representation deliberately, and introspect declarations. It has not turned an enum into a dynamically checked variant or made arbitrary external integers trustworthy.

## 15. From managed-language enum expectations to C++

Java enum constants are instances of a specialized enum class, with an object-oriented identity and language-supported operations such as `values()` and `ordinal()`. Python's `enum.Enum` provides runtime objects, mapping behavior, and metaclass-backed introspection; `IntEnum` deliberately opts into integer compatibility. C# enums are value types with integral underlying representations and can hold underlying values without named members.

C++ is closest to the value-oriented representation model, but its details differ significantly. A scoped `enum class` is not a Java-style class of singleton objects. Its declaration does not create a runtime reflection table, and a fixed-underlying enum can contain unnamed values. A classic unscoped enum can convert to integers and participate in arithmetic, while a scoped enum normally cannot.

Developers migrating from Python or Java should be especially cautious about expecting a checked cast that rejects an unknown value. `static_cast<Enum>(n)` does not generally throw. For a classic non-fixed enum, an out-of-range cast can even invoke undefined behavior. Input validation is an explicit operation, not an automatic side effect of constructing an enum-typed variable.

The best translation is to separate named domain values from external representation. Use a scoped enum for internal states, a defined unsigned representation for flags, explicit integer fields at serialization and C ABI boundaries, and parsers that decide whether unknown codes are rejected or preserved. Reflection and helper libraries can reduce repeated tables, but do not replace a schema.

## 16. The working model

A useful enumeration model has four layers: an underlying integer representation, a language-defined set of representable enum values, the subset assigned enumerator names, and the values accepted by the application. Most enum bugs are caused by treating two of these layers as identical.

The errors take several forms. An unnamed but representable fixed-underlying value is mistaken for undefined behavior; an out-of-range cast to a non-fixed enum is mistaken for harmless truncation. A state enum is given unrestricted bitwise operators. A fixed underlying type is treated as a complete network format. A switch with every named case is assumed to reject unknown values, or a `default` hides a newly introduced state. A shared atomic enum is mistaken for a complete concurrent state-machine protocol.

The remedy is not to stop using enums. It is to choose the right kind of enum for the domain, preserve conversions as explicit boundaries, and make the relationship between names and numeric values deliberate.

> **An enumeration names a domain; its underlying integer stores a value. The storage alone does not define which values the domain accepts.**

When reviewing an enum-oriented API, ask whether it is a state, a flag set, or an extensible code. Ask whether all valid values are named, whether unknown raw values must survive, whether the representation crosses an ABI or persistence boundary, and whether integer casts can invoke undefined behavior before any validator runs. These questions matter more than whether the declaration uses `enum` or `enum class` for stylistic reasons.

## Further reading

The language and library references for [enumerations](https://en.cppreference.com/w/cpp/language/enum), [implicit conversions](https://en.cppreference.com/w/cpp/language/implicit_conversion), [`std::underlying_type`](https://en.cppreference.com/w/cpp/types/underlying_type), [`std::to_underlying`](https://en.cppreference.com/w/cpp/utility/to_underlying), and [`std::is_scoped_enum`](https://en.cppreference.com/w/cpp/types/is_scoped_enum) document the precise distinctions discussed here.

The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) provide related guidance on scoped enums, type safety, and interfaces. [WG21 P2996](https://wg21.link/P2996) is the central technical reference for modern static reflection, including reflection of program entities and enumeration declarations. [Compiler Explorer](https://godbolt.org/) is useful for comparing enum layout, conversion code, and switch diagnostics across targets.

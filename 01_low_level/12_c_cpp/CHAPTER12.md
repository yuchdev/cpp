# Chapter 12. C and C++: Shared Heritage, Different Languages, and the ABI Between Them

There is a reassuring story frequently told about C and C++: C++ grew out of C, therefore a C program is nearly a C++ program, and connecting the two should amount to wrapping declarations in `extern "C"`. The story contains enough truth to be seductive and enough omissions to cause expensive failures.

C and C++ share much of their syntax, a great deal of their vocabulary, and decades of platform conventions. They are both used to implement operating systems, embedded firmware, databases, media pipelines, numerical libraries, and the lowest layers of language runtimes. A C++ application can often call a C library with little apparent ceremony. Some of the most successful cross-language interfaces in software are C-style APIs.

But the languages are not successive editions of one standard. They have different type systems, different name lookup, different initialization rules, different facilities for resource management, and sometimes different meanings for identical tokens. Even their shared binary calling convention is an agreement made by a toolchain and target ABI, not a guarantee that every valid C and C++ type can be passed interchangeably.

This distinction becomes important precisely where advanced developers work: high-performance systems, JNI and Python extensions, device drivers, native plugins, foreign-function interfaces, and long-lived binary libraries. A mistake at the boundary may pass ordinary compilation, survive unit tests, and emerge only when the library is upgraded or loaded on another architecture.

The central premise of this chapter is that **interoperability is a designed interface between two languages, not a side effect of similar syntax**. We will examine the languages separately, identify the subset of contracts that can be shared, and then construct a stable bridge through which ownership, lifetime, errors, and callback behavior can be expressed.

The six companion programs show the subject in action: [name mangling and overloads](01_name_mangling_and_overload/name_mangling_and_overload.cpp), [C linkage and callbacks](02_extern_c_callbacks/extern_c_callbacks.cpp), [static initialization and startup](03_static_init_and_main/static_init_and_main.cpp), [headers and fundamental types](04_headers_and_types/headers_and_types.cpp), [C features not in ISO C++](05_c_things_not_in_cpp/c_things_not_in_cpp.c), and [C++ features absent from C](06_cpp_things_not_in_c/cpp_things_not_in_c.cpp). The [CMake targets](CMakeLists.txt) reveal which files are compiled as C and which as C++. The repository's baseline is C++20; where C23, C++23, or C++26 facilities matter, they are identified explicitly.

## 1. The family resemblance stops at the language boundary

The historical relationship is real. Early C++ was built as an extension of C and inherited declarations, pointers, arrays, arithmetic, operators, struct syntax, and the ability to call conventional C libraries. Much of C's programming vocabulary remains immediately familiar to a C++ developer.

Yet **C++ is not a strict superset of modern C**. C evolved after C++ became a separate standardized language. C99 added features such as variable-length arrays, compound literals, `restrict`, and designated initialization forms that did not enter ISO C++ with identical semantics. C11 introduced a standardized atomic system with spelling and type rules different from C++11's atomics. C23 and C++23 each modernized their language without merging the standards.

Even code that parses in both languages may mean something different.

~~~c
/* C */
void* allocate(size_t n);
int* p = allocate(sizeof(int));
~~~

The implicit conversion from `void*` to `int*` is permitted in C, but not in standard C++. A C++ compiler requires an explicit conversion, and modern C++ generally offers a better ownership abstraction than raw allocation for constructed objects.

Another difference is fundamental to naming. In C, a structure tag and an ordinary identifier inhabit distinct name spaces. In C++, class names participate in ordinary type name lookup in a different way:

~~~c
/* C */
struct Device { int id; };
struct Device value;
~~~

In C++, `Device value;` is normally valid after the corresponding class definition. These differences are not merely stylistic: they affect shared headers, code generation, and the way opaque handles can be declared for both languages.

The first practical rule is therefore to compile each source file according to the language it actually uses. A `.c` file should not be silently reinterpreted as C++ just because the final application is written in C++. A mixed project can compile C translation units with a selected C dialect and C++ translation units with a selected C++ dialect, then link compatible object files.

## 2. Source compatibility and ABI compatibility are distinct

Three different questions are often compressed into one word, “compatible.”

**Source compatibility** asks whether text is accepted and has the intended meaning when compiled by the selected language. **Type/interface compatibility** asks whether callers and callees agree on types, operations, and ownership at the language level. **ABI compatibility** asks whether their compiled representations and calling sequences match on a specific target.

Consider a declaration:

~~~cpp
extern "C" int decode(const unsigned char*, unsigned long);
~~~

A C++ compiler can accept it. A C compiler can declare a corresponding function. A linker may locate a symbol named `decode`. But the interface is useful only if both sides also agree on the actual meaning and width of `unsigned long`, the treatment of null pointers, who owns the input storage, and how errors are represented.

For two components compiled for different targets, `unsigned long` might have different widths. For a dynamically linked library built with incompatible calling conventions, the symbols might match but argument registers might not. A C-compatible declaration is therefore necessary but not sufficient to define a binary protocol.

A useful production interface states its supported target ABI and toolchain assumptions. If interchange across several platforms is required, use exact-width integer types where available and appropriate, explicit byte serialization for durable formats, and an ABI version or capability negotiation mechanism for extensible function tables.

A C API is a powerful *lowest common denominator* for a C++ implementation, but it is not a universal cross-platform binary serialization standard.

## 3. Separate compilation brings the languages together

A mixed project typically consists of at least three conceptual pieces: a header expressing the shared interface, C source compiled under C rules, and C++ source compiled under C++ rules.

A small C header might contain:

~~~c
/* codec.h */
#ifndef CODEC_H
#define CODEC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int codec_decode(const unsigned char* data,
                 size_t length);

#ifdef __cplusplus
}
#endif

#endif
~~~

A C implementation can include this header and define the function. A C++ consumer includes the same header and calls it. When both object files obey the same target ABI, the linker can connect their declarations and symbols.

The `__cplusplus` macro is defined when compiling C++ and is not defined by a conforming C implementation compiling an ordinary C translation unit. This makes guarded language-linkage blocks a practical shared-header technique.

It is generally preferable to put these guards *inside the header itself* rather than requiring every C++ caller to wrap the include. The header's contract then remains explicit and self-contained.

### 3.1 Compile with the intended front end

A conceptual build is:

~~~sh
cc -std=c17 -c codec.c -o codec.o
c++ -std=c++20 -c app.cpp -o app.o
c++ app.o codec.o -o app
~~~

The final link often uses the C++ driver so it includes the appropriate C++ runtime and standard library dependencies. The exact commands depend on the toolchain, target operating system, and build system.

CMake naturally supports mixed languages:

~~~cmake
project(MixedCodec LANGUAGES C CXX)

add_library(codec STATIC codec.c)
add_executable(app app.cpp)
target_link_libraries(app PRIVATE codec)

set_target_properties(codec PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES)
set_target_properties(app PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)
~~~

Notice that selecting a C++ standard does not set a C source's language mode. A project can choose C17 for its C library and C++20 for its application without forcing C-only constructs through the C++ parser.

The [companion C-only program](05_c_things_not_in_cpp/c_things_not_in_cpp.c) is an important demonstration. Some of the differences it illustrates disappear or become extensions if the file is accidentally compiled as C++ with permissive flags. That masks the very portability boundary we want to understand.

## 4. Name mangling explains why overloads need special treatment

C++ supports overloads:

~~~cpp
void log(int);
void log(double);
~~~

A compiler must distinguish the two compiled functions. Most implementations decorate the external symbols with information about namespaces, parameter types, and other ABI-specific details. This is usually called **name mangling**.

C does not support C++-style function overloading by parameter type, so a conventional C API publishes one symbol name per function. A function declared with C language linkage in C++ participates in the platform's corresponding C-linkage convention:

~~~cpp
extern "C" int codec_version(void);
~~~

The C language-linkage specification does more than ask the compiler to delete a few characters from a symbol name. It specifies language linkage for eligible names and function types, under the applicable implementation and standard rules. Its usual practical effect is to make a function accessible through the target's C calling convention and symbol naming.

However, **C language linkage is not a promise that the parameter types are themselves C-compatible**. The following C++ declaration can be written, but it is not a usable ordinary C source interface:

~~~cpp
extern "C" void send(std::string message);
~~~

C does not know `std::string`. More importantly, its construction, layout, allocator, and destruction protocols are C++ implementation details. C linkage does not convert a C++ object into a C record.

### 4.1 Inspect symbols rather than guessing

The [name-mangling example](01_name_mangling_and_overload/name_mangling_and_overload.cpp) can be examined with the native object-file tools:

~~~sh
nm -C example.o
objdump -t example.o
~~~

On ELF systems, `readelf -Ws` and `objdump -T` are useful for symbol tables and dynamic exports. On Windows, `dumpbin /symbols` and `dumpbin /exports` serve similar purposes. On macOS, `nm` and `otool` are commonly used.

The spelling of mangled C++ symbols is platform-specific. Itanium-style ABIs and MSVC's ABI differ; even identical source can have different decorated names. A demangler is a diagnostic aid, not a compatibility guarantee.

This is why a stable plugin entry point often uses a deliberately simple C symbol such as `codec_get_api` rather than exposing an overloaded C++ function or an entire class hierarchy.

## 5. A shared header must be valid in both languages

A cross-language public header has two audiences, and each must be able to parse it under its own language rules.

A common pattern is:

~~~c
#ifndef DEVICE_API_H
#define DEVICE_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct device_handle device_handle;

device_handle* device_create(void);
void device_destroy(device_handle*);

int device_write(device_handle*,
                 const uint8_t* data,
                 size_t length);

#ifdef __cplusplus
}
#endif

#endif
~~~

The opaque type is intentionally incomplete. C callers can declare pointers to it without knowing its representation. C++ can define the actual `device_handle` structure privately in a source file and include a rich internal implementation object.

The header avoids `std::string`, references, templates, overloaded functions, constructors, and exceptions because a C compiler cannot express those interfaces.

It also avoids exposing layout decisions unnecessarily. If the representation of `device_handle` changes from a C++ struct containing a vector to a class containing a reference-counted device driver, the C header need not change as long as the handle's public operations preserve their contract.

### 5.1 Keep C++ convenience separate

A C++ wrapper can consume the C header and provide a better native C++ interface:

~~~cpp
#include "device_api.h"
#include <memory>
#include <span>

struct DeviceDeleter {
    void operator()(device_handle* p) const noexcept {
        device_destroy(p);
    }
};

using DeviceHandle =
    std::unique_ptr<device_handle, DeviceDeleter>;
~~~

This is a useful boundary: the C API remains a stable, explicit lifecycle interface, while C++ callers enjoy deterministic destruction through RAII.

A constructor or factory can call `device_create`, verify the returned pointer, and place it in `DeviceHandle`. A method can accept a `std::span<const std::uint8_t>` and pass `data()` plus `size()` to the C function. None of these C++ facilities needs to be exposed to the C client.

This pattern is not merely a workaround for C's lack of classes. It makes the ABI surface small and preserves freedom to change the internal C++ implementation.

## 6. The difference between pointers and ownership

C gives programmers allocation functions such as `malloc`, `calloc`, `realloc`, and `free`. C++ adds `new`, `delete`, constructors, destructors, smart pointers, and containers with ownership semantics.

It is unsafe to treat these allocation families as interchangeable. Storage allocated with `new` must be released using the matching `delete` form, not `free`. Storage allocated with `malloc` is normally released with `free` under the corresponding allocator contract. For arrays, object construction, class-specific allocation functions, and sized or aligned deallocation, the matching requirements become more elaborate.

A C++ object can also require its destructor to run before its storage is released. Merely freeing a block that contains a live non-trivial C++ object is not a valid substitute for proper destruction.

For a C-facing API, the most reliable rule is **the library that creates a resource defines how the client destroys it**:

~~~c
typedef struct codec_handle codec_handle;

codec_handle* codec_create(void);
void codec_destroy(codec_handle*);
~~~

The caller does not need to know whether `codec_create` ultimately uses `new`, a custom arena, an object pool, or a reference-counted backend. It only needs the public lifetime contract.

### 6.1 Why allocator matching matters even when both sides use C

A C ABI does not guarantee that all linked components share the same heap implementation, runtime library, or allocation domain. This is especially visible on Windows when different CRT configurations are involved, but similar problems can occur with plugins, custom allocators, and allocator-wrapping libraries elsewhere.

Passing a buffer from a library to a caller and saying “just free it” is safe only if the API explicitly guarantees that the caller's `free` is the correct deallocator for that allocation on supported configurations.

A more robust API pairs the allocation with a library-provided release function:

~~~c
int codec_get_output(codec_handle*,
                     unsigned char** output,
                     size_t* length);

void codec_free_output(unsigned char*);
~~~

Now the library controls allocation and deallocation even if its internal allocator changes. The API should also define what happens on failure: whether output parameters are cleared, whether null is accepted by the release function, and whether partial results can be returned.

### 6.2 Ownership is not just a pointer type

A `void*` or opaque handle communicates representation hiding, not ownership. The API must state whether a pointer is owned, borrowed, shared, nullable, or valid only during a callback.

An input pointer to a byte buffer is often borrowed for the duration of a synchronous call. A returned pointer to immutable internal data may remain valid only until the next API call. A returned opaque handle may require explicit destruction.

When these lifetimes are unspecified, clients tend to infer them from familiar APIs. That is fragile because the same C pointer spelling can express several incompatible ownership models.

## 7. Callbacks are functions plus environments plus time

A C callback often consists of a function pointer and a context pointer:

~~~c
typedef void (*codec_callback)(void* context, int status);

void codec_set_callback(codec_handle*,
                        codec_callback,
                        void* context);
~~~

The callback pointer names executable behavior. The `context` pointer lets it recover state without requiring C to understand C++ closure objects.

C++ capturing lambdas are objects with stored state. They cannot implicitly convert to ordinary function pointers because a function pointer has nowhere to store the captured environment.

~~~cpp
int offset = 5;

auto callback = [offset](int x) {
    return x + offset;
};

// cannot pass callback as int(*)(int)
~~~

A noncapturing lambda can convert to a function pointer in C++, but its generated function's language linkage and compatibility with a C callback type have cross-language subtleties. Where the API explicitly requires a C-linkage callback, a named `extern "C"` trampoline is the conservative design.

### 7.1 A C-linkage trampoline

The C-facing callback can recover a C++ object from an opaque context:

~~~cpp
struct Listener {
    void on_status(int);
};

extern "C" void listener_trampoline(void* context,
                                     int status) noexcept
{
    auto* listener = static_cast<Listener*>(context);
    if (!listener)
        return;

    try {
        listener->on_status(status);
    } catch (...) {
        // Log or record an error; never unwind through C.
    }
}
~~~

The example assumes that the pointer passed as `context` really points to a live `Listener` and that the callback ABI matches the declaration. The runtime check for null does not establish the object's type or lifetime.

The host can register the trampoline together with the listener pointer. When invoked, the function recovers the state and dispatches into ordinary C++ methods.

### 7.2 Lifetime is the real callback hazard

The most important callback question is often not *how* it is called but *when*.

Does registration retain the callback until explicitly unregistered? Can it be called concurrently from several threads? Can callbacks still arrive after unregistration begins? Can a callback run during `codec_destroy`? Is the context allowed to point to a temporary object?

An implementation that stores `context = &local` and invokes the callback after the function returns has a dangling pointer. Converting the callback to a different function-pointer type cannot repair this.

A robust API should specify a quiescence point. For example, a successful `codec_unregister_callback` might guarantee that no callback is running and no new callback will begin after return. That promise often requires synchronization and can affect whether unregistration is callable from inside a callback itself.

These are protocol decisions that a language-linkage specifier cannot make.

### 7.3 ABI and calling conventions

A callback can have a different calling convention from an ordinary function on some targets, especially older 32-bit Windows ABIs. C and C++ declarations must agree about how arguments are passed and who performs stack cleanup where relevant.

An opaque `void*` does not eliminate this requirement. It only provides a language-neutral way to carry state; the callback function pointer itself must match the ABI defined by the public header.

## 8. Exceptions are not a C error channel

A C++ function can throw. A C caller cannot portably catch a C++ exception using C language constructs. Even if a particular mixed-language toolchain unwinds successfully through selected C frames, relying on that behavior as a portable API contract is dangerous.

Every exported C entry point should define an error policy and prevent unexpected C++ exceptions from escaping:

~~~cpp
extern "C" int codec_decode(codec_handle* handle,
                             const unsigned char* data,
                             size_t length) noexcept
{
    try {
        if (!handle || (!data && length != 0))
            return -1;

        return do_decode(*handle, data, length);
    } catch (const std::bad_alloc&) {
        return -2;
    } catch (...) {
        return -3;
    }
}
~~~

The statuses above are illustrative. A production header should define named codes, stable numeric values, and any error-description retrieval API.

Using `noexcept` on the C++ implementation is appropriate when no exception is allowed to escape. But a shared C header cannot contain an unconditional C++ `noexcept` token and remain valid C. Either omit the implementation detail from the shared declaration while ensuring compatible language rules, or conditionally expand a C++-only exception specifier where the project requires it and can test consistency.

### 8.1 Preserve useful diagnostics without crossing the boundary

A C status code often loses context compared with a rich C++ exception. The library can keep an internal error object, log a diagnostic, or expose a C-compatible function that copies an error message into a caller-provided buffer.

~~~c
int codec_last_error(codec_handle*,
                     char* output,
                     size_t capacity,
                     size_t* required);
~~~

This interface still needs a clear policy: whether the message is UTF-8, how truncation is reported, whether the error is per-handle or per-thread, and when it is invalidated.

Using a caller-owned buffer is often safer than returning `const char*` to internal mutable storage, although an internal pointer can be acceptable if its lifetime is documented precisely.

### 8.2 RTTI and virtual objects are internal implementation details

A C API should not require callers to understand C++ `dynamic_cast`, type identification, virtual tables, or exception types. These are part of a C++ implementation and ABI environment, not ordinary C source-language facilities.

The C wrapper may use RTTI, polymorphism, and exceptions internally. It can then expose stable handles, integer status codes, and buffer APIs externally. This separation makes it possible to evolve the C++ implementation while preserving a narrow interoperability contract.

## 9. Structure layout: when compatible-looking types are not equivalent

A C record and a C++ standard-layout struct can often have compatible layouts under a shared target ABI:

~~~c
/* public header */
typedef struct device_packet {
    uint32_t id;
    uint8_t mode;
} device_packet;
~~~

That is a useful starting point. It is not a proof of compatibility across all compilers, flags, platforms, or versions.

The layout depends on alignment, padding, integer type widths, and ABI conventions. Adding a field can change `sizeof`, member offsets, and alignment. Packing pragmas or attributes can change them further. A structure containing pointers is generally not a portable disk or network representation across architectures.

For a public ABI, test layout explicitly:

~~~cpp
static_assert(std::is_standard_layout_v<device_packet>);
static_assert(offsetof(device_packet, id) == 0);
static_assert(sizeof(device_packet) >=
              sizeof(std::uint32_t) + sizeof(std::uint8_t));
~~~

The final assertion is illustrative and does not specify a complete ABI. Real tests should use expected sizes and offsets defined for each supported target, and the C build should contain corresponding checks using the facilities of its selected standard.

C++'s `std::is_standard_layout` is relevant but not sufficient by itself. A standard-layout class can still have platform-dependent padding and representation details. It also does not guarantee that every C compiler and C++ compiler agree on all extension-controlled packing conventions.

### 9.1 Bit-fields are especially unsuitable as wire-format contracts

A C and C++ header can both contain bit-field syntax:

~~~c
struct flags {
    unsigned ready : 1;
    unsigned mode : 3;
};
~~~

Their allocation order, storage units, alignment, and packing are implementation-dependent. Even on one compiler, changing target architecture or packing options can alter the object layout.

Use explicit masks and shifts over known-width integers to encode durable or network formats. Bit-fields may still be useful as in-process representations under a deliberately constrained ABI, but should not be assumed to describe a portable stream of octets.

### 9.2 Use size/version fields for extensible records

A long-lived C ABI often needs record evolution:

~~~c
typedef struct codec_options {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t flags;
    uint32_t reserved;
} codec_options;
~~~

The caller can provide the size of the layout it knows, while the library validates that it can safely read the required prefix. This can support additive evolution without interpreting memory beyond the caller's allocation.

The contract must specify how unknown flags are handled, whether reserved fields must be zero, and what happens when the caller's record is smaller or larger than the library's current version.

Versioned records cannot compensate for fundamentally incompatible pointer width or calling conventions. They are tools for controlled evolution *within* a supported ABI family.

### 9.3 Serialization is not structure copying

For a file or network protocol, even an ABI-stable in-memory struct is generally the wrong format. Raw memory contains host byte order and possible padding. Padding bytes may be indeterminate, and object representations should not be used as arbitrary protocol data without an explicit guarantee.

A serializer should encode each field with a specified width, byte order, and semantics. A decoder should validate lengths, ranges, and versioning before constructing application objects.

The distinction between an ABI struct and a serialized wire record becomes especially important in embedded systems where two processors share messages but differ in alignment and endianness.

## 10. The `void*` conversion difference

C's allocation functions return `void*`, and C ordinarily permits implicit conversion of that pointer to an object pointer type:

~~~c
#include <stdlib.h>

int* values = malloc(100 * sizeof *values);
~~~

C++ does not permit the same implicit conversion. A cast would make the assignment syntactically possible, but modern C++ usually has a better resource abstraction:

~~~cpp
std::vector<int> values(100);
~~~

or, when a dynamic array with exclusive ownership is appropriate:

~~~cpp
auto values = std::make_unique<int[]>(100);
~~~

The difference is not merely about casts. `malloc` allocates storage without invoking arbitrary C++ constructors, and `free` does not invoke destructors. C++ object lifetime rules are richer than the allocation of raw bytes.

When wrapping a C library that necessarily allocates through `malloc`, use a matching deleter or the library's provided free function. A typed RAII wrapper can make that pairing automatic without pretending that C allocation is equivalent to C++ `new`.

## 11. Names, tags, and C's `struct` conventions

C separates structure tags from ordinary identifiers. A declaration such as

~~~c
struct Packet {
    int id;
};
~~~

introduces a tag that is used as `struct Packet` unless a typedef or other declaration supplies an ordinary type alias.

A common C idiom is:

~~~c
typedef struct Packet {
    int id;
} Packet;
~~~

C++ generally allows the class name to be used directly as a type name after the definition. The typedef pattern remains useful in a shared C/C++ header because it is understood by both languages.

These naming differences become tricky around macros and generated bindings. A declaration that compiles as C++ may rely on class-name lookup not available in C. Conversely, a C identifier that coexists with a structure tag can conflict with C++'s treatment of the corresponding names.

Opaque tags are a particularly effective shared design:

~~~c
typedef struct engine_handle engine_handle;
~~~

The tag is incomplete in the public header. Both languages can use pointers to it, while the implementation remains free to define the actual structure privately.

## 12. Boolean values, null pointers, and literal types

C and C++ have related but distinct treatments of booleans and null pointer constants. C++ has `bool` as a core language type, with `true` and `false` as keywords. Older C standards use `_Bool` with `<stdbool.h>` providing familiar macro spellings; C23 evolved the language's spelling rules.

A portable C ABI often uses an explicit integer status or flag type rather than exposing assumptions about the ABI representation of a language-specific boolean. A function can document that zero means false and nonzero means true, or define named status codes.

C++11 introduced `nullptr` with type `std::nullptr_t`, which avoids ambiguities associated with historical `NULL` macros. C23 also introduces a `nullptr` facility, but similarity of spelling does not establish that arbitrary C and C++ overload or conversion rules coincide.

In shared headers, use an explicit null pointer convention and avoid macros whose expansion depends on language-specific overload resolution.

### 12.1 Why overloading makes `NULL` particularly risky

Consider a C++ overload set:

~~~cpp
void select(int);
void select(void*);
~~~

Depending on the definition of `NULL`, a call with `NULL` can select an integer overload or become ambiguous. `nullptr` communicates pointer intent in C++:

~~~cpp
select(nullptr); // selects pointer overload
~~~

A C-facing function cannot be overloaded in the same manner. That is another reason a shared C interface generally relies on distinct function names and simple parameter contracts.

## 13. Arrays, variable lengths, and compound literals

C99 permits variable-length arrays (VLAs) in applicable contexts:

~~~c
/* C99 */
void work(size_t n)
{
    int scratch[n];
    process(scratch, n);
}
~~~

ISO C does not prescribe a particular stack allocation mechanism for the array, and support has qualifications across C editions and implementations. The corresponding variable-length array declaration is **not standard C++**. GCC and Clang may accept some forms as C++ extensions, but such acceptance does not make them portable C++20.

For C++, a dynamic owner such as `std::vector<T>` is usually a clearer choice:

~~~cpp
void work(std::size_t n)
{
    std::vector<int> scratch(n);
    process(scratch.data(), scratch.size());
}
~~~

For small bounded temporary storage, `std::array` or a fixed-capacity container may be suitable. For non-owning function parameters, C++20 `std::span` can describe a pointer-plus-length view without creating an allocation.

### 13.1 C compound literals are not C++ temporaries

C99 compound literals can construct unnamed objects with language-specific storage duration and lifetime rules:

~~~c
/* C99 */
struct Point { int x, y; };
struct Point p = (struct Point){ .x = 2, .y = 3 };
~~~

Standard C++ does not have C99 compound literals as such. A C++ braced temporary or aggregate initialization may express a similar value, but the object lifetime and reference-binding rules are not identical.

On GNU C++, a compiler may accept compound-literal-looking syntax as an extension. A pointer obtained from such a temporary must not be assumed to follow the lifetime rules of the corresponding C compound literal.

When translating code, preserve the intended owner and lifetime rather than copying punctuation.

### 13.2 Designated initializers have different constraints

C99 supports designated initializers with features such as array designators. C++20 introduced designated initializers for aggregates, but their rules are more restrictive and follow the declared member order.

~~~cpp
struct Point {
    int x;
    int y;
};

Point p{.x = 2, .y = 3}; // C++20
~~~

A C99 array designation such as `[3] = 42` is not ordinary C++20 aggregate initialization syntax. Nor can a C++ designated initializer arbitrarily reorder members in ways C may permit.

A shared header with aggregate types is one thing; a shared source file relying on designated initialization syntax is another. Code generators should target the selected language rather than assume one universal form.

## 14. `restrict` and optimization contracts

C99 introduced `restrict` as a pointer qualification mechanism used to express important aliasing assumptions within a defined scope of execution. It can permit stronger optimization when a function is guaranteed to access certain objects through restricted pointer paths.

~~~c
/* C99 */
void add_arrays(float* restrict output,
                const float* restrict a,
                const float* restrict b,
                size_t n);
~~~

The actual `restrict` contract is more subtle than saying the pointers “never alias.” It involves how objects are accessed through restrict-qualified lvalues during executions of associated blocks. A caller that violates the contract can invalidate the callee's language-level assumptions.

There is **no portable ISO C++ `restrict` keyword** in C++20, C++23, or the relevant C++26 model. GCC/Clang and MSVC offer vendor spellings such as `__restrict__` or `__restrict`, but their exact behavior and accepted locations are implementation extensions.

A mixed-language header can conditionally hide a restrict spelling from a C++ compiler:

~~~c
#if defined(__cplusplus)
#  define CODEC_RESTRICT
#else
#  define CODEC_RESTRICT restrict
#endif
~~~

This is a simplification suitable only when the project explicitly accepts losing that qualifier on the C++ side. In production, choose the C edition, compiler family, and extension policy deliberately; some public headers use compiler-specific qualifiers to preserve optimization information across languages.

In either language, the API should document real disjointness requirements. Optimization annotations do not replace the semantic contract that callers must satisfy.

## 15. C11 atomics and C++11 atomics are not universal ABI twins

C11 introduced atomic types and operations through `_Atomic` and `<stdatomic.h>`. C++11 introduced `std::atomic<T>` and related operations through `<atomic>`.

Both languages can target the same hardware instructions and often interoperate under carefully constrained platform conventions. The standards do not, however, guarantee that `_Atomic int` and `std::atomic<int>` have identical type identity, object representation, initialization requirements, or alignment on every implementation.

It is therefore risky to place an atomic object directly in a supposedly portable C/C++ shared structure and assume both languages can operate on it interchangeably.

A safer boundary exports synchronized operations:

~~~c
typedef struct counter_handle counter_handle;

void counter_increment(counter_handle*);
unsigned counter_read(const counter_handle*);
~~~

The C++ implementation may store a `std::atomic<unsigned>`, while the C interface expresses only operations with documented concurrency semantics.

This also permits the implementation to change its synchronization strategy without breaking the ABI. Atomics are not merely a field layout issue; callers need to know the memory-ordering and publication guarantees of the exported operations.

### 15.1 `volatile` is not a cross-language synchronization substitute

Both C and C++ have volatile-qualified objects, but neither permits `volatile` to stand in for a general inter-thread atomic protocol in C++. A volatile read or write is not automatically a synchronization operation, and data races remain possible.

For memory-mapped hardware, volatile may have platform-specific roles that must be coordinated with barriers and the device memory model. For concurrent software, use proper atomic or locking interfaces with explicit lifetime and ordering contracts.

A shared C header containing a volatile field does not settle these questions.

## 16. Static initialization, constructors, and the path to `main`

C and C++ applications may use the same operating-system process entry machinery, but C++ has additional language-level initialization and destruction duties.

A C++ namespace-scope object can run a constructor before `main`:

~~~cpp
struct Logger {
    Logger();
    ~Logger();
};

Logger global_logger;
~~~

The C++ runtime coordinates initialization and eventual destruction under language and platform rules. A C program has static object initialization, but ordinary C structures do not have C++ constructors or destructors.

This distinction matters for mixed-language libraries. Loading a C++ shared object may trigger initialization of C++ global objects even when its public API is C-compatible. Unloading the library may also trigger destructor-related work under platform conventions.

A C caller may have no idea that importing a library initializes thread pools, registries, or allocators before its first explicit API call.

### 16.1 The initialization-order trap

Across translation units, dynamic initialization order can be difficult to control. A C++ global constructor calling a C API that itself depends on another dynamically initialized C++ object may invoke the second component too early.

A function-local static can express initialization on first use:

~~~cpp
Registry& registry()
{
    static Registry instance;
    return instance;
}
~~~

Since C++11, initialization of such a local static is synchronized across threads. That does not make subsequent registry mutation thread-safe, nor does it guarantee that its destructor can safely call arbitrary objects during process shutdown.

For a shared library used through C, an explicit `library_initialize`/`library_shutdown` protocol can be clearer than relying entirely on implicit global constructors. The library can still use RAII internally, but the foreign caller is given a lifecycle it can control.

### 16.2 `main` is not an ordinary exported callback

The C and C++ standards define hosted program startup and termination in their own ways. A C++ implementation may need runtime support for constructors, exception handling, thread-local objects, and static destruction. Linking a mixed executable with an inappropriate driver or runtime configuration can omit required support.

A toolchain's C++ compiler driver commonly arranges the proper libraries during final linking. A custom embedded startup or freestanding build must explicitly account for whichever parts of the language runtime are used.

This is why build configuration is not an afterthought in C/C++ interoperability. The final link step may need C++ runtime dependencies even when the public API contains only C functions.

## 17. What C++ brings that a C API should not expose directly

C++ templates, classes, inheritance, destructors, exceptions, operator overloads, references, and standard-library containers provide strong abstractions within a C++ program. They are not ordinary C source-language constructs.

A C API should not export a virtual C++ class and expect a C caller to understand its vtable, inheritance adjustments, or destructor rules. Nor should it expect C code to instantiate a template or handle a C++ exception object.

The correct response is not to abandon those features inside the implementation. It is to separate the implementation's rich internal types from the public boundary.

For example, a C++ codec can internally use:

~~~cpp
class Codec {
public:
    explicit Codec(Config);
    std::vector<std::byte> decode(std::span<const std::byte>);
private:
    std::unique_ptr<Backend> backend_;
};
~~~

Its C façade can expose:

~~~c
typedef struct codec_handle codec_handle;

codec_handle* codec_create(const codec_config*);
int codec_decode(codec_handle*,
                 const unsigned char* input,
                 size_t input_size,
                 unsigned char* output,
                 size_t output_capacity,
                 size_t* output_size);
void codec_destroy(codec_handle*);
~~~

The caller sees an opaque handle, bytes, sizes, and error codes. The library controls object construction, allocation, exception translation, and destruction.

### 17.1 Design for resource exhaustion and partial failure

A useful façade should specify what happens when allocation fails, the output buffer is too small, decoding fails after partial work, or a callback throws internally.

Do output parameters remain unchanged on failure, or are they reset? Can the caller retry with a larger buffer? Is the handle still valid after an error? Can two threads call `codec_decode` on one handle concurrently? These details decide whether a library can be integrated safely into long-running applications.

The C ABI cannot infer them from syntax. Document them and test them as part of the interface.

## 18. An opaque handle does not have to cost performance

There is a temptation to expose C++ object layout directly for speed. In reality, an opaque handle API need not impose a significant cost relative to the operations it performs.

A handle is often just a pointer to an implementation object. A call through the C ABI incurs an ordinary call boundary, but the implementation can still use templates, SIMD, parallelism, specialized allocation, and compiler intrinsics internally. For substantial codec, database, or network operations, the work performed usually dominates the call overhead.

Where the hot path truly consists of nanosecond-scale operations, the overhead can matter. The architecture can offer batched operations, caller-provided buffers, preallocated sessions, and fewer transitions across the boundary rather than exposing internal class layouts.

For example, replacing one call per byte with one call per packet or batch often has a much larger effect than micro-optimizing the ABI trampoline itself.

The lesson is to profile at the intended granularity. A stable C ABI and a high-performance implementation are compatible when the operation boundaries are designed thoughtfully.

## 19. Strings are protocols, not `std::string` objects

A frequent interoperability failure begins with the assumption that a C string and a C++ string are interchangeable.

C commonly passes a pointer to a null-terminated character sequence:

~~~c
int device_open(const char* name);
~~~

C++ can pass `std::string::c_str()` for the duration required by the API, but the pointer does not transfer ownership. If the C function stores the pointer, the C++ string's lifetime and invalidation rules become relevant.

For binary data and strings that can contain embedded null bytes, pointer-plus-length is often more appropriate:

~~~c
int device_open_name(const char* bytes, size_t length);
~~~

This still needs an encoding contract. Does the interface expect UTF-8, a platform-specific multibyte encoding, raw bytes, or some other representation? What constitutes a valid name? Will the library copy the bytes, or borrow them?

A C++ wrapper can accept `std::string_view` and forward data plus length for a synchronous copy-in API. It must not assume that the string view owns the underlying bytes or that the C library will copy them unless promised.

Similarly, a C API returning a `const char*` must specify whether the pointer is static, handle-owned, thread-local, invalidated by the next call, or individually allocated and released with a matching function.

These details matter far more than the convenience of using one type name at the boundary.

## 20. Callbacks, C++ closures, and language runtime bridges

Callbacks deserve special emphasis in native extensions because they often cross not just C and C++, but a managed runtime boundary as well.

A Python extension may use the Python C API to invoke a Python callable. A JNI native library may hold a Java reference and call a method through a `JNIEnv*`. A C# application may pass a delegate to native code through a platform invoke mechanism. Each case adds runtime-specific rules for thread attachment, reference lifetime, exception translation, and reentrancy.

A stable C façade can make the native ABI understandable, but it does not replace the requirements of the other runtime. A JNI callback may need a valid thread-local `JNIEnv*` obtained through the supported attach mechanism. A Python callback generally needs to respect the interpreter and GIL policy of the selected Python runtime. A C# delegate may need rooting or an appropriately managed lifetime to prevent its native callback thunk from becoming invalid.

The common architecture is still the same: an exported function pointer or function table, a context or handle, a clear registration/unregistration protocol, and a defined error channel. Additional runtime bridges sit above it.

This is a useful reason to keep the lowest ABI narrow. A clean C interface can serve multiple languages even though each binding layer has different higher-level semantics.

## 21. C23 and C++23: similar spellings, separate rules

C23 and C++23 both modernize their respective languages, and several spellings are now familiar to developers of both. That does not make their type systems or source programs interchangeable.

For example, boolean and null-pointer facilities have changed across C editions, while C++ has long had its own `bool`, `nullptr`, overload resolution, and reference-binding rules. A macro or declaration using one of these tokens may still have different type or conversion behavior in the two languages.

Shared headers should explicitly declare their supported C standard and C++ standard. The build should verify both modes independently rather than using one compiler command and assuming the language front ends interpret the header identically.

Compiler feature-test macros are also language-specific. A C++ `__cpp_*` feature macro is not a substitute for testing C23 support in a C compiler. If a public header needs conditional spellings, the conditional should be documented and tested under the actual compiler configurations it claims to support.

The most reliable approach remains conservative: use a small shared ABI vocabulary and keep modern language features in each language's own adapter.

## 22. C++20 modules cannot erase the C header boundary

C++20 modules provide a different source-organization mechanism from textual includes. A C++ module can wrap calls into a C library and export a more expressive C++ interface:

~~~cpp
// Illustrative module interface
module;
#include "device_api.h"

export module device.wrapper;

export int query_version()
{
    return device_version();
}
~~~

This requires the C header to be available in the appropriate global module fragment and a compatible C implementation to be linked. The module system does not reinterpret C source as C++ or make the C library's ABI portable.

Header units, where supported, also have their own treatment of macros and build artifacts. A C header that relies on textual macro configuration may not behave identically when imported through a different build mechanism.

The migration path is straightforward: keep the stable C-facing header valid C, implement the C library in its intended language, and expose a modern C++ module or wrapper to C++ consumers. The wrapper owns the translation of strings, errors, handles, and callbacks.

A module interface is a source-level convenience and encapsulation boundary. It is not a universal binary compatibility protocol.

## 23. C++26 reflection and generated bindings

C++26 static reflection is relevant to tooling that wants to inspect C++ declarations and generate adapters. In principle, reflection can reduce repetitive binding code, generate metadata, or help validate that a public interface matches a schema.

But reflection does not define a universal binary representation of a C++ class, nor does it make a `std::vector` layout stable across compilers or library versions. A generated wrapper is only as sound as the ABI and lifetime rules it implements.

For a cross-language binding generator, a stable schema should describe exported functions, field widths, structure versions, ownership, nullability, callback behavior, and error handling. C++ reflection may help extract some information from C++ declarations, but it cannot infer every semantic promise or turn an undocumented allocator relationship into a safe contract.

Treat generated bindings as code requiring tests, not as proof of interoperability.

## 24. Diagnosing mixed-language failures systematically

The first question in an interoperability failure should be **which boundary was crossed incorrectly?**

A compile error may indicate that a shared header contains C++-only syntax, a C feature excluded from ISO C++, or a type name interpreted differently. A link error may indicate missing C language linkage, an object file compiled under the wrong language, an omitted library, or a disagreement about symbol names. A runtime crash may indicate calling-convention mismatch, structure layout disagreement, incorrect deallocation, dangling callback state, or an exception escaping through an unsupported ABI boundary.

A reliable diagnostic workflow starts by compiling the shared header in isolation with the intended C and C++ compilers. Then inspect symbols in each object file, compare the actual signatures and compile options, and run an integration test that links C and C++ objects built under the supported modes.

On GCC/Clang, common warning settings include `-Wall -Wextra -Wpedantic -Wconversion`. A strict ISO language mode such as `-std=c17` or `-std=c++20` helps distinguish standard facilities from GNU extensions; `-pedantic-errors` can reveal accidental dependencies. MSVC `/W4 /permissive-` and `/analyze` provide useful C++-side checks.

ABI mismatches are not all visible to sanitizers. AddressSanitizer can catch invalid frees, use-after-free, and many buffer errors on executed paths. UndefinedBehaviorSanitizer can detect selected invalid operations. ThreadSanitizer can reveal races in callback or shared-handle code, where supported. None establishes cross-toolchain ABI compatibility or proves that an exception never escapes the wrong boundary.

### 24.1 Inspect binaries and compiler assumptions

Use `nm`, `readelf`, `objdump`, `dumpbin`, and platform linker maps to confirm names and exports. Build with multiple supported configurations, including different optimization levels and LTO where relevant. Explicitly test pointer width, structure alignment, and field offsets.

Compiler options such as structure packing, `-fshort-enums`, calling-convention attributes, standard-library ABI settings, and exception/RTTI settings can affect the binary contract. They are not merely performance switches.

`-fno-strict-aliasing` weakens some optimization assumptions but does not legalize invalid object lifetimes or arbitrary pointer reinterpretation. `-fwrapv` may define a compiler-specific signed-overflow behavior but cannot be treated as a portable C/C++ ABI rule.

For extensions such as `__restrict__`, `__declspec(dllexport)`, or visibility attributes, use small compatibility wrappers with documented feature tests. Avoid copying platform annotations into a supposedly portable shared header without an explicit support matrix.

### 24.2 Test failure and teardown paths

A successful codec call is the easiest case. The most valuable integration tests exercise allocation failure, empty and malformed buffers, truncation, null handles, repeated destroy attempts if defined, concurrent callbacks, callback unregistration, plugin unload, and error-message lifetimes.

Also test that every resource allocated by the library is released through the correct exported function. A library may work correctly on Linux and fail on Windows because allocation and deallocation occur through incompatible CRT heaps; such bugs are contract violations, not necessarily mistakes in ordinary C syntax.

A cross-language API should be tested as a public product: header compilation, linking, ABI inspection, normal use, failure handling, and shutdown.

## 25. The standards timeline

**C++98/03 and C89/C90.** C and C++ already shared extensive syntax and the practical ability to communicate through C-style APIs. Nevertheless, their type systems, name lookup, overload resolution, constructors, exceptions, and conversions differed. C++ language linkage was essential for exposing conventional C symbols from C++ translation units.

**C99.** C gained variable-length arrays, compound literals, designated initializers, and `restrict`. Several features had no equivalent ISO C++ syntax, or acquired different forms in later C++ standards. This widened the set of valid C sources that were not valid standard C++ sources.

**C++11 and C11.** C++11 brought move semantics, smart pointers, lambdas, and standardized atomics into the mainstream C++ model, while C11 standardized atomics and threading facilities in a distinct language model. Better internal tools made C++ wrappers safer but did not make their object types ABI-compatible with C.

**C++14 and C++17.** Further C++ language and library evolution strengthened implementation-side RAII and generic programming. The conservative C façade—opaque handles, plain records, and explicit operations—remained the most widely portable interface.

**C++20.** `std::span`, concepts, and modules offered clearer native C++ wrappers and source organization. C++20 designated initializers existed but did not simply copy the C99 rules. Modules did not remove the need to compile C sources in C mode.

**C23 and C++23.** The two languages introduced and refined overlapping vocabulary, including null and boolean spelling in C23, while retaining distinct type and conversion rules. Build and feature tests must continue to target the actual language standard.

**C++26.** Static reflection and related compile-time features can assist binding generation, but the ABI boundary still requires specified layouts, calling conventions, ownership, error contracts, and versioning.

The historical lesson is not that C++ steadily absorbs C. It is that both languages evolve, and the interoperability layer must remain deliberately conservative where binary compatibility is important.

## 26. A practical migration strategy

When modernizing an existing C library for C++ use, replacing every C structure and function with a C++ class is rarely the best first step. Preserve the tested C ABI and add a C++ ownership layer on top. The wrapper can transform status codes into exceptions or result objects, convert pointer-plus-length buffers into spans, and give resource handles deterministic destruction.

When exporting a new C++ engine to C, reverse the process. Keep the C++ classes private, choose an opaque handle type, and expose create/operate/destroy functions. Make every failure path return a defined status. Convert strings through an explicit encoding policy. Design callback registration as a lifetime protocol rather than a pair of raw pointers with unspecified timing.

For embedded or high-frequency use, avoid unnecessary per-item boundary crossings. Batch operations, allow caller-owned buffers, and document alignment and aliasing preconditions instead of exporting raw C++ object internals merely to reduce call overhead.

For plugin and cross-language deployment, version the ABI, test layout and symbol exports, and make allocator ownership unambiguous. Keep a compatibility matrix that names supported target architectures, compiler families, C and C++ modes, and runtime configurations.

The migration is successful not when the header contains the fewest `#ifdef` directives, but when a client can use the API correctly without knowing how the implementation was compiled.

## 27. Working rules

The interface between C and C++ is a meeting point of source languages, object models, compiler conventions, and runtime lifetimes. Similar syntax can make it easy to forget that each layer imposes its own guarantees.

Use the C compiler for C-only semantics and the C++ compiler for C++ semantics. Keep a shared header independently valid in both languages. Treat `extern "C"` as a language-linkage tool, not a promise about arbitrary C++ object layout. Represent stable resources through opaque handles and paired creation/destruction functions. Keep exceptions, RTTI, templates, and C++ standard-library objects behind the façade. Specify byte widths, encoding, alignment, callback lifetime, and error behavior.

Above all, distinguish a **portable language rule** from a **target ABI convention** and from an **application-level promise**. A structure that happens to have matching offsets on one compiler is not yet a validated public ABI. A callback that happens not to fire after unregistration in one test is not yet a lifetime guarantee. A C-compatible symbol name is not a safe cross-language interface if its ownership and failure rules are unspecified.

> C and C++ interoperability works best when the boundary is intentionally smaller than either language. The implementation can remain expressive; the contract must remain precise.

## Further reading

The primary language references are the documentation for [C++ language linkage](https://en.cppreference.com/w/cpp/language/language_linkage), [C++ declarations and `extern`](https://en.cppreference.com/w/cpp/language/storage_duration), [fundamental types](https://en.cppreference.com/w/cpp/language/types), [C compound literals](https://en.cppreference.com/w/c/language/compound_literal), [C variable-length arrays](https://en.cppreference.com/w/c/language/array), [C restrict qualification](https://en.cppreference.com/w/c/language/restrict), and [C++ standard-layout types](https://en.cppreference.com/w/cpp/named_req/StandardLayoutType).

For binary-level details, consult the [Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html), the [Microsoft PE/COFF format documentation](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format), and the actual compiler's calling-convention and runtime-library documentation. The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) provide useful principles for ownership, resource management, and interfaces.

The six companion examples and their [CMake integration](CMakeLists.txt) can be used to inspect compiler front-end differences and emitted symbols. Build the C-only program as C, the C++ programs as C++, and test the shared contracts under more than one supported toolchain.

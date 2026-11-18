# Chapter 10. Linking: Translation Units, ODR, and Binary Interfaces

The compiler can accept a source file without knowing whether its referenced functions exist anywhere in the final program. The linker can combine object files without proving that every class definition is consistent across all translation units. A shared library can export a symbol with the expected name while accepting arguments according to a different ABI. These are not contradictions. They are consequences of the layered C++ program model.

Linking is where independently compiled source must become one coherent program. To understand it, we need to distinguish source-level names, declarations and definitions, language linkage, one-definition-rule constraints, object-file symbols, exported binary interfaces, and runtime loading. The categories overlap, but they are not interchangeable.

This chapter follows the repository's seven companion builds: [translation-unit basics](01_basics/main.cpp), [inline and ODR examples](02_odr_inline/odr_header.hpp), [template instantiation](03_templates/templ.hpp), [initialization order](04_init_order/main.cpp), [shared-library visibility](05_visibility/visibility.h), [Windows DLL interfaces](06_windows_dll/dll_api.h), and [runtime plugin loading](07_dlopen/plugin_api.h). Their individual build scripts and the [chapter CMake configuration](CMakeLists.txt) are part of the experiments. The baseline is C++20, with later developments discussed separately.

## 1. Translation units: why one source file is not the whole program

Consider a function declared in a header:

~~~cpp
// operations.hpp
int calculate(int);
~~~

A caller can compile against that declaration:

~~~cpp
// main.cpp
#include "operations.hpp"

int main()
{
    return calculate(4);
}
~~~

The definition may exist elsewhere:

~~~cpp
// operations.cpp
#include "operations.hpp"

int calculate(int x)
{
    return 2 * x;
}
~~~

An ordinary Unix-like build separates compilation and linking:

~~~sh
c++ -std=c++20 -c main.cpp -o main.o
c++ -std=c++20 -c operations.cpp -o operations.o
c++ main.o operations.o -o app
~~~

The compiler translates each source file using its reachable declarations. The linker connects the emitted reference from `main.o` to the definition in `operations.o`. Omitting the latter object usually gives an undefined reference, even though the caller compiled successfully.

A **translation unit** is the result of the language's translation phases applied to a source file, including included headers and macro expansion. Header text is incorporated separately into every translation unit that includes it. It is not compiled once into a common global object just because the same header is used repeatedly.

This matters if a header contains a definition:

~~~cpp
// dangerous.hpp
int calculate(int x) { return 2 * x; }
~~~

Include it in two translation units and the final program may have two definitions of one externally linked function where the language permits only one. A linker will commonly diagnose duplicate symbols, but compiler/linker acceptance is not the definition of language correctness.

The principal division of labor is: a **declaration** allows code to refer to an entity; a **definition** supplies the entity; the **link/build graph** ensures the definition is present in the required binary image.

## 2. Scope, linkage, and symbol visibility are different

**Scope** controls where a name can be found through C++ lookup. **Linkage** controls whether separate declarations denote the same entity. **Symbol visibility** controls exposure of compiled symbols under a particular binary format and platform.

~~~cpp
namespace telemetry {
    int samples = 0;
}
~~~

The variable has namespace scope and ordinarily external linkage. Another source file cannot name it without a suitable declaration, yet that declaration need not occur in the same source file. Whether the variable becomes an exported symbol of a shared library is a separate platform policy.

This distinction is essential because developers often say “global” to mean four different things: namespace scope, static storage duration, external linkage, or dynamic-library export. They do not follow from one another automatically.

### 2.1 Internal linkage: separate identity in each translation unit

A namespace-scope `static` definition has internal linkage:

~~~cpp
static int attempts = 0;
~~~

If the same header definition is included in two translation units, the result is normally two distinct `attempts` objects. That is suitable for deliberately local implementation state, but wrong for a counter intended to be shared across the whole program.

An unnamed namespace is the idiomatic modern mechanism for keeping helper entities within a translation unit:

~~~cpp
namespace {
    int attempts = 0;

    void reset_attempts()
    {
        attempts = 0;
    }
}
~~~

It can contain types and overloads as well as variables. “Internal linkage” does not mean that code outside the translation unit cannot access the memory through a pointer passed to it; linkage is about identity *by name*, not protection of storage.

### 2.2 External linkage: several declarations, one entity

A header can declare a variable defined once in a source file:

~~~cpp
// state.hpp
extern int connections;

// state.cpp
int connections = 0;
~~~

The intended externally linked entity is the same across the participating translation units. An `extern` declaration ordinarily does not define a variable without an initializer, but an initialized `extern` variable declaration can be a definition:

~~~cpp
extern int count;      // declaration
// extern int count = 0; // definition; use as an alternative
~~~

The word `extern` is not a universal guarantee of “declaration only.”

### 2.3 Names with no linkage

Local automatic variables, parameters, labels, aliases, and several other name categories do not participate in cross-unit linkage identity:

~~~cpp
void first(int value)
{
    int local = value;
}

void second(int value)
{
    int local = value * 2;
}
~~~

The repeated spelling `local` refers to unrelated objects. Their addresses can still be passed to other functions; again, name linkage and object lifetime are separate properties.

## 3. The three roles of `static`

At namespace scope, `static` commonly gives internal linkage. Inside a function, a static local has static storage duration:

~~~cpp
unsigned next_id()
{
    static unsigned counter = 0;
    return ++counter;
}
~~~

As a class data-member specifier, `static` means that the member belongs to class-level state rather than being stored separately in each object:

~~~cpp
struct Connection {
    static unsigned created;
};
~~~

These meanings should not be compressed into the phrase “shared variable.” A function-local static has block scope but long lifetime; a class static member may have external linkage; a namespace-scope static has internal linkage. None of them is automatically safe for concurrent mutation.

The historical overloading is why a review should explicitly identify *scope*, *storage duration*, and *linkage* for every interesting object rather than infer all three from one keyword.

## 4. The one-definition rule is a whole-program invariant

C++ permits certain definitions to occur in multiple translation units, but only under precise conditions. This is the **one-definition rule** (ODR).

An ordinary non-inline externally linked function or global object that is odr-used requires its appropriate definition, ordinarily once in the program. Conversely, class definitions, templates, inline functions, and inline variables may appear in several translation units if the standard's consistency requirements are satisfied.

~~~cpp
// api.hpp
int process(int); // declaration

// api.cpp
int process(int x)
{
    return x + 1;
}
~~~

That is the conventional declaration/definition split. A non-inline function body in a widely included header can lead to multiple definitions.

An inline header function is intentionally different:

~~~cpp
// math.hpp
inline int triple(int x)
{
    return 3 * x;
}
~~~

Identical qualifying definitions in multiple translation units are permitted and designate the same entity under the relevant rules. The implementation may use weak symbols or COMDAT sections to make this practical; those object-file techniques are not the source-language guarantee.

### 4.1 Identical-looking behavior does not excuse differing definitions

Imagine:

~~~cpp
inline int capacity()
{
#if defined(FAST_MODE)
    return 32;
#else
    return 16;
#endif
}
~~~

If two translation units compile different bodies under what is meant to be the same externally linked inline function, the program can violate the ODR. The linker may select one definition and appear to succeed. LTO may reveal the inconsistency later, but the original defect existed before optimization.

The rule is stricter than “these definitions compute the same result.” Corresponding definitions must satisfy token and lookup consistency rules, with specific language exceptions. It is possible for the same header text to violate the ODR if dependent lookup resolves differently in separate translation units.

Class layout mismatches are especially dangerous. If one library sees a public `struct` with different packing or macro-selected fields from another library, the symbol names may still link while the two sides disagree on memory layout. A successful link does not validate the ABI.

## 5. Constants, external declarations, and inline variables

A namespace-scope const object in C++ commonly has internal linkage by default:

~~~cpp
// constants.hpp
const int capacity = 4096;
~~~

Each translation unit including the header can have its own object. If the value is folded into constant expressions, the duplicated identity may be invisible; taking its address can expose it.

To declare one external object, use a header declaration and a single source definition:

~~~cpp
// constants.hpp
extern const int capacity;

// constants.cpp
extern const int capacity = 4096;
~~~

That split gives a single externally linked object, but a client seeing only the declaration cannot generally treat the value as a constant expression because its initializer is unavailable there.

C++17 inline variables provide a header-friendly alternative:

~~~cpp
inline constexpr int capacity = 4096;
~~~

The inline definition can appear in multiple translation units and identify one object under the ODR. Inline variables need not be immutable:

~~~cpp
inline std::atomic<unsigned> active_sessions{0};
~~~

That facility is convenient, but it neither eliminates concurrency requirements nor guarantees one universally shared object across independent dynamic-library images on every platform. If several plugins must share one registry or counter, pass an explicit host-owned handle or provide an exported accessor rather than assuming inline state automatically unifies across loaders.

## 6. Headers, self-sufficiency, and build graphs

A header should compile when included by a minimal translation unit, with its required standard declarations already included:

~~~cpp
#ifndef LIBRARY_API_HPP
#define LIBRARY_API_HPP

#include <cstdint>
#include <string_view>

std::uint32_t hash(std::string_view);

#endif
~~~

A conventional include guard is portable. `#pragma once` is widely implemented but remains nonstandard; unusual filesystem aliases and generated headers can matter to how an implementation identifies one physical header.

Headers should normally declare ordinary externally linked functions and variables rather than define them. Inline definitions, template definitions, types, and other language-permitted entities are important exceptions. Avoid including ordinary headers inside functions or classes, since their declarations can land in unintended scopes.

The build graph must make the provider target available to the consumer. In CMake, source compilation and linking are separate responsibilities:

~~~cmake
add_library(operations STATIC operations.cpp)
add_executable(app main.cpp)
target_link_libraries(app PRIVATE operations)
~~~

A caller can include `operations.hpp` correctly and still get an undefined reference if the library target is not linked. Conversely, linking the library does not supply missing source-level declarations. CMake's `PUBLIC`, `PRIVATE`, and `INTERFACE` usage requirements control propagation of include directories, compile definitions, and dependencies.

ABI-affecting compile options should be consistent across the target graph. One target built with incompatible packing or enum-width settings can make a seemingly correct call violate the binary contract.

## 7. Templates require available definitions or explicit instantiations

A function template is a pattern, not necessarily one prebuilt function:

~~~cpp
// templ.hpp
template<class T>
T twice(T x)
{
    return x + x;
}
~~~

When a consuming translation unit instantiates `twice<int>`, the compiler generally needs the template definition. A template declaration alone may allow type checking but fail to produce the specialization required for linking.

An alternative is to compile selected specializations centrally:

~~~cpp
// templ.hpp
template<class T>
T twice(T);

// templ.cpp
#include "templ.hpp"

template<class T>
T twice(T x)
{
    return x + x;
}

template int twice<int>(int);
~~~

The explicit instantiation definition supplies `twice<int>` to clients that link the corresponding object. Other types are unavailable unless further instantiations or visible definitions are provided.

An explicit-instantiation declaration can reduce redundant client code generation:

~~~cpp
extern template int twice<int>(int);
~~~

This tells participating translation units to rely on the supplied specialization rather than performing that instantiation themselves under the relevant rules. It can improve compile times in large systems, but shifts responsibility to the binary dependency and can affect optimization opportunities.

The essential insight is that a template's source definition and an instantiated specialization's linker symbol serve different roles.

## 8. Initialization order is not source-file order

Global objects can be zero-initialized, constant-initialized, or dynamically initialized. Dynamic initialization may execute constructors and other functions before the program reaches its ordinary entry point.

~~~cpp
int counter = 0;
std::string greeting = make_greeting();
~~~

Within a translation unit, many ordered dynamic initializations follow definition order. Across translation units, the ordering rules are more complex. A global initializer in one unit can depend on a global object in another unit that has not yet been initialized. This is the **static initialization order fiasco**.

The related shutdown problem occurs when a static object's destructor accesses another object already destroyed.

### 8.1 Construct-on-first-use and C++11 thread safety

A function-local static can control the first initialization through actual use:

~~~cpp
Registry& registry()
{
    static Registry instance;
    return instance;
}
~~~

Since C++11, concurrent initialization attempts are synchronized. If initialization throws, a later entry may retry. Recursive reentry while initialization of the same static is still underway is undefined behavior.

The idiom can replace cross-unit startup dependencies with a more explicit call dependency. It does not automatically make subsequent operations on the registry thread-safe, nor does it solve every shutdown-order hazard.

### 8.2 C++20 `constinit`

The declaration

~~~cpp
constinit int startup_mode = 42;
~~~

requires static initialization under the applicable rules. It does **not** mean the variable is const. A later change to the initializer that would require unsupported dynamic initialization becomes a compile-time problem instead of an unexpected startup dependency.

`constexpr` and `constinit` address different concerns: constant-expression usability and constness on one hand, enforcement of static initialization on the other.

Thread-local objects add another lifetime dimension. If a thread still owns an object whose destructor resides in an unloaded plugin, shutdown can become unsafe even if startup was correctly ordered.

## 9. Symbol names are ABI-specific, not standard C++

C++ allows namespaces and overloads:

~~~cpp
namespace net {
    void send(int);
    void send(std::string_view);
}
~~~

Toolchains commonly encode function names, namespaces, parameter types, and other information into symbol names. This is **name mangling**. There is no universal ISO C++ mangling scheme. Itanium-style C++ ABIs are common on many Unix-like systems, while MSVC uses a different system on Windows.

Inspect what the toolchain actually emitted:

~~~sh
nm -C object.o
readelf -Ws library.so
objdump -T library.so
~~~

On macOS, `nm` and `otool` offer related inspection. On Windows, `dumpbin /symbols` and `dumpbin /exports` reveal COFF symbols and DLL exports. `c++filt` can demangle supported C++ symbols.

A demangled name is a diagnostic aid, not proof of interoperable calling conventions or class layouts. Even identical-looking source signatures can have different ABI requirements when compiled with different runtimes or target options.

## 10. C language linkage and ABI boundaries

C++ can specify C language linkage:

~~~cpp
extern "C" int plugin_version();
~~~

On common platforms this usually avoids C++ name mangling for the symbol and requests the implementation's C language-linkage treatment. It is useful for C interoperability, foreign runtimes, assembly, and runtime loader discovery.

It does not make arbitrary C++ parameter types C-compatible:

~~~cpp
// C++ can declare this, but C cannot express std::string:
extern "C" void consume_cpp_string(std::string);
~~~

The source-language and ABI contracts must be designed explicitly. A C-facing header can use conditional linkage:

~~~cpp
#ifdef __cplusplus
extern "C" {
#endif

int plugin_version(void);
int plugin_process(const unsigned char* data,
                   unsigned long size);

#ifdef __cplusplus
}
#endif
~~~

Validate that header with both C and C++ compilers if the API promises both. Agree on parameter widths, alignment, ownership, and callback signatures, not merely exported names.

Exceptions should not be assumed to propagate through arbitrary foreign-language or C stack frames safely. Translate exceptions into documented status results at a C-facing entry point:

~~~cpp
extern "C" int query(int* output) noexcept
{
    try {
        if (!output)
            return -1;
        *output = cpp_query();
        return 0;
    } catch (...) {
        return -2;
    }
}
~~~

The error policy and cleanup contract belong to the public interface. C linkage does not supply them.

## 11. Visibility on ELF and Mach-O platforms

External linkage and dynamic symbol export are separate properties. A shared library may contain externally linked C++ functions that the project does not wish to expose to its consumers.

On supported GCC/Clang targets, a common pattern is hidden default visibility and explicit public exports:

~~~cpp
#if defined(__GNUC__) || defined(__clang__)
#  define LIB_API __attribute__((visibility("default")))
#else
#  define LIB_API
#endif

extern "C" LIB_API int library_version();
~~~

Compiler options such as `-fvisibility=hidden` can reduce accidental exports and symbol interposition. The exact semantics depend on the object format and loader.

Exporting an ordinary function is a relatively narrow contract. Exporting a full C++ class can expose virtual tables, RTTI, class layout, exception types, inline methods, and allocator conventions. Stable cross-toolchain plugin boundaries often keep C++ implementation types private and export simple versioned functions instead.

Weak definitions, COMDAT folding, and ELF symbol interposition are implementation mechanisms, not exceptions to the ISO C++ ODR. A linker choosing one of two inconsistent inline definitions does not make that source program valid.

## 12. Windows DLL import and export

Windows PE/COFF commonly uses explicit DLL import/export declarations:

~~~cpp
#if defined(_WIN32)
#  if defined(BUILDING_MY_DLL)
#    define LIB_API __declspec(dllexport)
#  else
#    define LIB_API __declspec(dllimport)
#  endif
#else
#  define LIB_API
#endif
~~~

The library-building target defines `BUILDING_MY_DLL`; consumers do not. The same public header can then declare:

~~~cpp
extern "C" LIB_API int library_version();
~~~

A conventional Windows link often uses an import library to satisfy references at build time, while the DLL itself must be available at runtime. Linking the import library does not guarantee the matching DLL will be present or that its version is compatible.

Calling conventions can matter, particularly on 32-bit Windows targets where stack cleanup and register passing conventions may differ. Modern 64-bit Windows platforms have different conventions; do not assume that an old `__stdcall` declaration always represents a distinct machine-level call sequence.

An exported API should define allocator ownership, exception behavior, type widths, and runtime-library compatibility. C++ standard-library objects exchanged across DLL boundaries can impose toolchain configuration requirements that a narrow C-compatible API often avoids.

## 13. Dynamic loading: linking with a lifetime

A runtime plugin system can load modules explicitly. POSIX-style systems provide `dlopen`, `dlsym`, and `dlclose`; Windows provides `LoadLibrary`, `GetProcAddress`, and `FreeLibrary`.

~~~cpp
#include <dlfcn.h>

void* handle = dlopen("./plugin.so", RTLD_NOW);
if (!handle) {
    // consult dlerror()
}
~~~

A successful load is not enough. The application must locate a compatible exported entry point:

~~~cpp
using VersionFn = int(*)();

dlerror(); // clear previous error
void* symbol = dlsym(handle, "plugin_version");
const char* error = dlerror();

if (error == nullptr) {
    auto version = reinterpret_cast<VersionFn>(symbol);
    // call only when the real signature and ABI agree
}
~~~

POSIX specifies useful symbol-conversion behavior for `dlsym` on supported platforms; ISO C++ does not universally guarantee interchangeability of arbitrary object and function pointer representations.

A found symbol proves that the name exists, not that the host has the correct function prototype or calling convention.

### 13.1 Design a versioned plugin contract

One robust pattern exports a small function table:

~~~cpp
extern "C" {

struct PluginApi {
    unsigned abi_version;
    void* (*create)(void* host_context);
    int (*run)(void* instance);
    void (*destroy)(void* instance);
};

const PluginApi* plugin_get_api(unsigned requested_version);

}
~~~

Production systems must specify exact widths as appropriate, structure alignment, ABI negotiation, error behavior, allocation/free responsibility, and thread-safety rules. A version field by itself does not solve backward compatibility; structure-size negotiation and optional-function handling may also be necessary.

The host should own the module lifecycle explicitly: load, negotiate, create, operate, stop callbacks and workers, destroy instances, and unload. Do not unload code while callbacks, plugin-defined destructors, suspended coroutines, or thread-local objects may still depend on it. Loader reference counts and deferred unloading are platform details, not substitutes for program ownership rules.

## 14. Static archives and shared libraries have different resolution models

A static archive is commonly a collection of object files with an index. A linker may extract only members needed to satisfy unresolved references. On some Unix linkers, library ordering matters because an archive processed before its unresolved references are known may contribute no members.

A shared library remains a separate runtime image with its own dependencies and symbol-resolution behavior. An executable can link successfully and still fail on startup because a DLL or shared object cannot be found or a transitive dependency is missing.

CMake provides different target models—`STATIC`, `SHARED`, `MODULE`, `OBJECT`, and `INTERFACE`—because these build and deployment contracts are not interchangeable.

Dependency inspection tools include `ldd` on some Linux systems, `otool -L` on macOS, and `dumpbin /dependents` on Windows. Use appropriate non-executing inspection methods for untrusted binaries.

The library form should follow the intended lifecycle: linked implementation, replaceable shared runtime, or explicitly loaded plugin. File extensions alone are not an architectural description.

## 15. C++20 modules add module linkage

Named C++20 modules introduce a further linkage category for appropriate entities: **module linkage**. Such names can be shared among translation units attached to the same named module while remaining distinct from ordinary externally linked entities available everywhere.

~~~cpp
// telemetry.cppm, a module interface
export module telemetry;

int internal_counter = 0;

export int read_counter()
{
    return internal_counter;
}
~~~

The example's `internal_counter` belongs to the module under the applicable attachment and linkage rules. It is not just an unnamed-namespace variable and is not automatically an exported external symbol for unrelated binaries.

Modules improve source-level reachability and dependency structure. They require a build graph for compiled module interface artifacts, which are typically toolchain/configuration-specific. The global module fragment, often used for textual includes, has distinct attachment rules.

A module export is not the same thing as a shared-library symbol export, and modules do not standardize C++ ABI layout or name mangling. A project can have excellent module encapsulation and still need an explicit binary compatibility policy.

## 16. LTO can reveal existing ODR failures

Link-time optimization retains additional compiler representation across translation units and permits cross-unit inlining, devirtualization, constant propagation, and dead-code elimination.

It can also expose an inconsistency that separate compilation concealed. If two units compile different definitions of what should be the same inline function or class, LTO may diagnose a violation or optimize under assumptions that make the program behave unexpectedly. The bug was in the inconsistent definitions, not in LTO.

Object-file mechanisms including weak symbols, COMDAT sections, and platform-specific symbol merging can make a broken program appear linkable. They do not weaken the source-language ODR.

For performance-critical libraries, test representative builds with and without LTO and verify that all public headers see consistent ABI-affecting compile definitions.

## 17. Diagnosing linkage errors methodically

A linker error is easier to solve when classified by cause.

An **undefined reference** may mean a missing implementation object, an incorrect library dependency, an unmatched namespace/signature, internal rather than external linkage, or a template specialization never instantiated.

A **multiple-definition** failure often indicates a header defining a non-inline external function or object, the same object linked twice, or inconsistent definitions emitted across units.

A **runtime loading failure** can mean a missing shared object, incorrect search path, missing transitive dependency, absent export, or wrong symbol version.

An **ABI mismatch** may compile and link but fail at execution because the caller and provider disagree on object layout, calling convention, exception runtime, alignment, or allocation policy.

An efficient investigation is evidence-driven:

1. Compare the exact declaration the caller compiled with the definition provided by the library. Check namespace, language linkage, cv/ref and exception details where relevant, calling convention, and macro configuration.
2. Identify which object file or archive ought to provide the symbol; inspect it with `nm`, `readelf`, `objdump`, `otool`, or `dumpbin`.
3. Examine the final linker command, static archive ordering, import libraries, and CMake target dependencies.
4. For dynamic libraries, inspect exports and loader dependencies separately from ordinary object-file symbols.
5. For suspected ODR violations, compare preprocessed units and relevant ABI-affecting compiler options, then test with LTO and available ODR diagnostics.

GCC/Clang `-Wall -Wextra -Wpedantic` and relevant declaration warnings catch some source defects. MSVC `/W4 /permissive- /analyze` provides corresponding diagnostics. Linker map files, GNU-style `--no-undefined`, symbol tracing, MSVC `/VERBOSE:LIB`, and export inspection answer more targeted linking questions.

AddressSanitizer and UndefinedBehaviorSanitizer can expose memory symptoms of invalid plugin lifetimes or ABI mismatches. They do not prove complete ODR compliance or ABI portability.

### 17.1 ABI-affecting compiler flags are not incidental

Options including `-fvisibility=hidden`, `-fshort-enums`, packing controls, RTTI/exception settings, and standard-library ABI configuration can affect compiled interfaces. They must be coordinated across components sharing types and runtime behavior.

`-fno-strict-aliasing` weakens some optimizer assumptions but does not legalize invalid object lifetimes. `-fwrapv` offers compiler-specific signed-overflow behavior, not an ISO C++ guarantee. `-std=gnu++20` can accept extensions beyond strict `-std=c++20` mode.

Treat such flags as part of a documented build and binary-compatibility contract, not merely toggles for speed or warning suppression.

## 18. How linking evolved from C++98 to C++26

**C++98/03:** Traditional multi-translation-unit compilation, internal and external linkage, headers, the ODR, templates, inline functions, C language linkage, and implementation-specific dynamic linking formed the original model.

**C++11:** Thread-safe initialization of block-scope static objects improved lazy construction and eliminated an important initialization race, without changing the need for correct global lifetime dependencies.

**C++14:** Incremental language improvements continued, while template-heavy and header-only libraries relied on the same ODR consistency requirements.

**C++17:** Inline variables made it possible to define appropriate single program-level objects in headers under the ODR's multiple-definition rules, reducing the need for manual `extern`/single-source patterns.

**C++20:** Named modules introduced module linkage and a new source-dependency architecture. `constinit` made the static-initialization guarantee explicit for eligible variables.

**C++23:** Additional language and library facilities continued to operate under the established linkage and ABI framework; they did not eliminate platform-specific symbol and library interfaces.

**C++26:** Reflection and further metaprogramming capabilities may help generate and inspect interfaces, but cannot automatically make independently built C++ libraries ABI-compatible. Build graphs, layout contracts, and runtime loader behavior remain explicit engineering responsibilities.

## 19. Managed-language intuition and C++ binary boundaries

Java, Python, and C# frequently present packages, modules, or assemblies as runtime objects with managed metadata and loading behavior. C++ traditionally separates source declarations from compiled implementations much earlier, and an exported library symbol need not carry portable runtime type metadata.

A header declaration is not a loaded implementation. A namespace is not a DLL visibility policy. A namespace-scope variable defined in a header may create several internal-linkage objects or a multiple-definition failure rather than one runtime singleton.

Named C++ modules improve source organization, but compiled module interface artifacts are not universally portable managed bytecode. C++ plugin authors must still decide the calling convention, data layout, allocation policy, exception behavior, and module lifetime.

The most productive migration habit is to identify the **source API**, **binary provider**, and **runtime lifecycle** separately. Then ask which one owns any object exchanged across those boundaries.

## 20. Final perspective

A C++ declaration begins as a source-language name. Scope controls how it is found; linkage controls which declarations identify the same entity; the ODR constrains definitions. Compilation emits code and references. Linking resolves them into binary images. Export policy decides what a dynamic consumer can find, and ABI rules decide whether that consumer can call the result safely. Initialization and object lifetime decide when such a call is valid.

Most difficult bugs confuse two stages. A header declaration is mistaken for a definition. A static object is mistaken for a shared global. An inline function with inconsistent bodies is assumed harmless because the linker chooses one. A found export is mistaken for ABI compatibility. A plugin is unloaded while outstanding callbacks still need its code.

A useful review therefore asks: **Which entity is named? Where is its definition? Which compiled component provides it? What ABI connects its users? And when is it initialized and safe to access?**

> Linking is not simply the final build command. It is the agreement between independently compiled parts that makes the complete program valid.

## Further reading

For precise source rules, consult [definitions and the ODR](https://en.cppreference.com/w/cpp/language/definition), [storage duration and linkage](https://en.cppreference.com/w/cpp/language/storage_duration), [inline functions and variables](https://en.cppreference.com/w/cpp/language/inline), [language linkage](https://en.cppreference.com/w/cpp/language/language_linkage), [templates](https://en.cppreference.com/w/cpp/language/templates), [initialization](https://en.cppreference.com/w/cpp/language/initialization), and [modules](https://en.cppreference.com/w/cpp/language/modules).

For toolchains, the [CMake target linking reference](https://cmake.org/cmake/help/latest/command/target_link_libraries.html), the [Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html), and [Microsoft PE/COFF documentation](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format) cover critical implementation details.

The [companion CMake projects](CMakeLists.txt) make these ideas concrete. Build and inspect them under multiple configurations, compare exported symbols, and test loader lifetime before treating a binary interface as stable.

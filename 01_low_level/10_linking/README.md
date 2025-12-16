## Linking. External Declarations and the Program Model

> C-style linking. Linkage and function pointers.
> One Definition Rule (ODR). Translation units.
> Name mangling, ABI boundaries, symbol visibility, dynamic linking.

Linking is the phase where **separately compiled translation units** are combined into a single program or library.
Most subtle C++ bugs at scale are **linking bugs**, not syntax or type-system bugs.

---

## Translation Units

A **translation unit (TU)** is the result of preprocessing a source file:

* One `.cpp` file
* Plus all headers it includes (recursively)
* After macro expansion and conditional compilation

Each TU is compiled **independently**, then linked together.

> The compiler never sees the whole program at once (unless LTO is enabled).

---

## Linkage Types

### 1. Internal linkage

Entities with **internal linkage** are visible *only within their own translation unit*.

```cpp
static int i;   // internal linkage
```

* Each TU gets its **own copy**
* Same name in another TU refers to a **different entity**
* Common for:

  * `static` globals
  * entities in anonymous namespaces
  * `const` namespace-scope variables (by default, in C++)

---

### 2. External linkage

Entities with **external linkage** refer to the *same entity across all translation units*.

```cpp
int g;          // external linkage by default
extern int g;   // declaration only
```

* One definition in the entire program
* Accessible across TUs

#### Anonymous namespace (important subtlety)

```cpp
namespace {
    int i;
}
```

* `i` has **internal linkage**
* Not reachable from other TUs
* Preferred over `static` in modern C++

> In C++11 and later, anonymous namespaces are the **recommended way** to express internal linkage.

---

### 3. No linkage

Some names do **not participate in linkage at all**:

* Local variables
* Function parameters
* Labels
* Enumerators
* Typedef / `using` names
* Template parameters

They are **purely compile-time or local constructs**.

---

## Scope vs Linkage (do not confuse)

| Concept     | Meaning                                             |
| ----------- | --------------------------------------------------- |
| **Scope**   | Where a name can be *written*                       |
| **Linkage** | Whether two declarations refer to the *same entity* |

C scopes:

* File scope
* Block scope
* Function prototype scope
* Function scope

C++ adds:

* Namespace scope
* Class scope

> Scope is a **syntax rule**, linkage is a **program-wide semantic rule**.

---

## The One Definition Rule (ODR)

The **ODR** governs what may be defined where:

### Objects and non-inline functions

* **Exactly one definition** in the entire program

### Types, templates, inline functions

* One definition **per translation unit**
* All definitions must be **identical**

Violations lead to:

* Multiple-definition linker errors
* Or worse: **undefined behavior** if the linker merges incompatible definitions

---

## `static` keyword: historical overload

`static` has **two unrelated meanings**:

1. **Class scope**
   → static data member (shared by all objects)

2. **Namespace/global scope**
   → internal linkage (historical "`intern`" keyword)

```cpp
static int x;   // internal linkage (global)
```

> Prefer **anonymous namespaces** instead of `static` at namespace scope.

---

## `const` and linkage

In **C++ (unlike C)**:

```cpp
const int ci = 42;   // internal linkage by default
```

To make it externally visible:

```cpp
extern const int ci;   // declaration
```

Or, since **C++17**, the modern solution:

```cpp
inline constexpr int ci = 42;
```

✔ header-safe
✔ ODR-safe
✔ single entity program-wide

---

## Headers and Inclusion Rules

Best practices:

* Use `#include` **only at namespace/global scope**
* Prefer `#pragma once` over macro guards
  (de-facto supported by all major compilers)
* Headers should be **self-sufficient**
* Headers should **declare**, not define (unless `inline`, templates, or header-only)

---

## Global Initialization Order

There is **no guaranteed order** of initialization of global objects across TUs.

This causes the infamous **static initialization order fiasco**.

### Correct pattern (since C++11)

```cpp
int& instance()
{
    static int value = 0;
    return value;
}
```

* Construct-on-first-use
* Thread-safe since C++11

### C++20 improvement: `constinit`

```cpp
constinit int g = 42;
```

* Enforces **static initialization**
* Prevents accidental dynamic initialization
* Does **not** imply constness

---

## `extern "C"` and ABI Boundaries

```cpp
extern "C" void c_function();
```

`extern "C"` specifies:

* **No C++ name mangling**
* C ABI-compatible symbol
* Required for:

  * C / C++ interop
  * `dlsym`, `GetProcAddress`
  * Assembly, Fortran, foreign runtimes

Rules:

* Declarations and definitions must **agree**
* You cannot mix C and C++ linkage for the same entity
* Only affects **linkage**, not type checking

Wrapping headers:

```cpp
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#ifdef __cplusplus
}
#endif
```

---

## Name Mangling

C++ encodes:

* Function names
* Namespaces
* Classes
* Parameter types
* cv-qualifiers

into **mangled symbol names**.

This enables overloading, but breaks cross-language linking.

Use tools to inspect:

* `nm -C`
* `objdump`
* `dumpbin`
* `readelf`
* `otool`

---

## Inline Functions and Variables

### Inline functions

* May appear in multiple TUs
* ODR-merged into one entity

### Inline variables (C++17)

```cpp
inline int counter = 0;
```

* Exactly one program-wide object
* Header-safe
* Replaces many old `extern` patterns

---

## Templates and Linking

Templates are **instantiated at use sites**.

Common pitfalls:

* Template defined in `.cpp` only → **undefined reference**
* Fixes:

  * Move definition to header
  * Or use **explicit instantiation**

```cpp
template int foo<int>(int);
```

---

## Symbol Visibility (Shared Libraries)

On ELF / Mach-O systems (GCC/Clang):

```cpp
__attribute__((visibility("default")))
```

Typical pattern:

* Compile with `-fvisibility=hidden`
* Explicitly export API symbols

Benefits:

* Faster linking
* Smaller dynamic symbol tables
* Better encapsulation

---

## Windows DLL Import / Export

```cpp
__declspec(dllexport)
__declspec(dllimport)
```

Usually wrapped in macros:

```cpp
#ifdef BUILDING_DLL
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
```

Different from ELF:

* Symbol visibility is **opt-in**
* Import libraries participate in linking

---

## Dynamic Linking at Runtime

POSIX example:

* `dlopen`
* `dlsym`
* `dlclose`

Requires:

* C linkage symbols
* Stable ABI
* Careful lifetime management

---

## Facts about Linking

```
extern "C" {
// ...
}
```

* This technique is commonly used to produce a C++ header from a C header
* Alternatively, conditional compilation (12.6.1) can be used to create a common C and C++ header:

```
#ifdef __cplusplus
extern "C" {...}
```

* A name with C linkage can be declared in a namespace (std::printf)
* A variable defined outside any function (that is, global, namespace, and class static variables) is initialized before main() is invoked
* Often, a function returning a reference to static is a good alternative to a global variable
* The initialization of a local static is thread-safe (42.3.3)
* The quick_exit() function is like exit() except that it does not invoke any destructors
* Register functions to be invoked by quick_exit() using at_quick_exit()
* Exit possibilities
  * std::_Exit (abort): doesn't execute static destructors or flush critical IO and soes not call handler
  * std::exit: executes static destructors and flushes critical IO and call handler if exists
  * std::quick_exit: doesn't execute static destructors, but does flush critical IO and call handler if exists

---

## How Linking Differs from "Imports" in Other Languages

Developers coming from **Java, Python, C#, JavaScript, Go**, or similar languages often expect linking to behave like importing a package or module.

That mental model does **not** apply to C++.

### Other languages

* Imports are:

  * Runtime or VM-managed
  * Name-based
  * Order-independent
  * Dependency-resolved automatically
* Symbols are:

  * Objects
  * Namespaced
  * Dynamically discoverable
* Initialization order is defined and managed by the runtime

### C++

* Linking is:

  * A **build-time** process
  * Based on **object files**, not source files
  * Sensitive to compilation units and order
* Symbols are:

  * Raw linker symbols
  * ABI-encoded
  * Often invisible or duplicated unless controlled
* Initialization order:

  * Partially undefined
  * Toolchain- and platform-dependent
  * The programmer's responsibility

> In C++, there is no package loader, no module registry, and no runtime safety net.
> Linking is **mechanical**, **static**, and **unforgiving** — by design.

---

## Toolchain Notes

Useful compiler / linker flags:

* `-c` – compile only
* `-Wl,-Map=map.txt` – link map
* `-fno-common` – catch tentative definitions
* `-ffunction-sections -Wl,--gc-sections` – dead stripping
* `-flto` – link-time optimization

---

# Namespaces (C++98 → C++20)

> Logical structure. Name lookup. Linkage interaction.
> Namespaces are not modules, not packages, and not a visibility mechanism.

Namespaces are a **compile-time name organization tool**.
They exist to structure programs, prevent naming conflicts, and enable large-scale library composition.

---

## Purpose of namespaces

* Use namespaces to express logical structure (C++ Standard 14.3.1)
* Prevent name collisions across large codebases and libraries
* Allow independent development and composition of components
* Enable controlled API evolution (inline namespaces)

Namespaces **do not**:

* affect object layout
* control linkage by themselves
* isolate macros
* enforce dependencies
* provide runtime encapsulation

---

## Linkage and namespaces

* A name that can be used in translation units different from the one in which it was defined is said to have **external linkage**
* A name that can be referred to only in the translation unit in which it is defined is said to have **internal linkage**

### Default linkage rules in namespace scope

By default, the following have **internal linkage** in namespace scope:

* `const` objects
* `constexpr` objects
* type aliases
* anything declared `static`

This differs from C, where `const` objects have external linkage by default.

```cpp
const int x = 10;        // internal linkage in C++
extern const int y = 20; // explicitly external
```

---

## Unnamed (anonymous) namespaces

* An unnamed namespace can be used to make names local to a compilation unit
* The effect of an unnamed namespace is very similar to that of internal linkage

```cpp
namespace {
    int helper;
    void internal_function();
}
```

### Important details

* Entities technically have external linkage
* But they are **unreachable** from other translation units
* Preferred modern replacement for `static` globals
* Applies to variables, functions, classes, templates

---

## Named namespaces and namespace merging

Namespaces can be **defined in multiple places** and are merged by name:

```cpp
namespace my {
    int a;
}

namespace my {
    int b;
}
```

### Properties

* Namespaces can be reopened across translation units
* Widely used in the standard library
* No ordering or dependency constraints

---

## Namespace aliases

Namespace aliases provide shorter or version-neutral access:

```cpp
namespace very_long_vendor_library {
    void f();
}

namespace lib = very_long_vendor_library;
```

### Common use cases

* Readability
* Vendor prefixes
* Version abstraction
* Gradual refactoring

---

## `using` declarations and `using` directives

### `using` directive

```cpp
using namespace std;
```

* Brings **all names** into scope
* Can cause ambiguities
* Must **never** appear in headers
* Acceptable only in small, local scopes

### `using` declaration (preferred)

```cpp
using std::vector;
```

* Imports a single name
* Participates in overload resolution
* Safer and explicit

> Prefer `using-declaration` whenever possible.

---

## Argument-Dependent Lookup (ADL)

Namespaces participate in **argument-dependent lookup** (Koenig lookup):

```cpp
namespace math {
    struct vec {};
    void normalize(vec&);
}

math::vec v;
normalize(v); // found via ADL
```

### Why ADL exists

* Enables clean operator syntax
* Avoids verbose qualification
* Fundamental to STL design

### Pitfalls

* Can introduce surprising overloads
* Requires careful namespace design
* Overusing ADL can reduce clarity

---

## Namespaces and operator overloading

Operators should usually be **free functions in the same namespace as their operands**:

```cpp
namespace math {
    vec operator+(vec, vec);
}
```

### Why not member operators?

* Breaks symmetry
* Blocks implicit conversions
* Disables ADL

---

## Inline namespaces (C++11+)

* The `inline` specifier makes a nested namespace the **default meaning** of the enclosing namespace
* This is primarily used for **API and ABI versioning**

```cpp
namespace lib {
    inline namespace v2 {
        void f();
    }
    namespace v1 {
        void f();
    }
}
```

```cpp
lib::f();      // v2
lib::v1::f();  // explicit
```

### Key facts

* Inline namespaces affect name lookup **and ABI**
* Widely used in standard library implementations
* Enable backward compatibility without breaking source code

---

## Headers, includes, and namespaces

* Spaces are significant within the `< >` or `" "` of an include directive
* The absence of a `.h` suffix does **not** imply anything about how the header is stored

  * `<map>` is often stored as `map.h` internally
* For each C standard-library header `<X.h>`, there is a corresponding C++ header `<cX>`

```cpp
#include <string.h>  // C header
#include <cstring>  // C++ header (preferred)
```

---

## C linkage and namespaces

* We can specify a linkage convention to be used in an `extern` declaration
* `extern "C"` specifies C linkage (no name mangling)

```cpp
extern "C" void c_function();
```

### Linkage blocks

This construct, commonly called a **linkage block**, can be used to enclose a complete C header:

```cpp
extern "C" {
#include <string.h>
}
```

### Key rules

* C functions must be declared with C linkage to be callable from C++
* If a function has multiple linkage declarations, they must agree
* Declaring a function as both C and C++ linkage is an error

---

## Namespaces vs packages in other languages

Developers coming from **Java, Python, C#, JavaScript, Go** should note:

### Other languages

* Imports are:

  * runtime-aware
  * dependency-controlled
  * isolated
  * module-backed
* Namespaces/packages:

  * control visibility
  * control loading
  * participate in runtime semantics

### C++

* Namespaces are:

  * compile-time only
  * purely lexical
* They:

  * do not load code
  * do not isolate macros
  * do not control linkage
  * do not prevent ODR violations

> **Namespaces organize names.
> Linking organizes symbols.
> Modules (C++20) organize interfaces.**

---

## Practical guidelines

* Use namespaces aggressively in libraries
* Never use `using namespace` in headers
* Prefer anonymous namespaces to `static`
* Place operators in the namespace of their operands
* Use inline namespaces for versioning
* Do not confuse namespaces with modules

---

## Namespaces Facts

* Use namespaces to express logical structure; 14.3.1
* The inline specifies that the nested namespace is the default meaning of sub-namespace. This could be useful for the namespace versionig
* A name that can be used in translation units different from the one in which it was defined is said to have external linkage
* A name that can be referred to only in the translation unit in which it is defined is said to have internal linkage
* By default, const objects (7.5), constexpr objects (10.4), type aliases (6.5), and anything declared static (6.3.4) 
  in a namespace scope have internal linkage
* An unnamed namespace can be used to make names local to a compilation unit
* The effect of an unnamed namespace is very similar to that of internal linkage
* spaces are significant within the < > or " " of an include directive
* The absence of a .h suffix does not imply anything about how the header is stored
  A header such as `<map>` is usually stored as a text file called map.h in some standard directory
* For each C standard-library header `<X.h>`, there is a corresponding standard C++ header `<cX>`
* We can specify a linkage convention to be used in an extern declaration
* This construct, commonly called a linkage block, can be used to enclose a complete C header to make a header suitable for C++ use

### At-Exit Functions

* The quick_exit() function is like exit() except that it does not invoke any destructors
* Register functions to be invoked by quick_exit() using at_quick_exit()
* Exit possibilities
  * std::_Exit (abort): doesn't execute static destructors or flush critical IO and soes not call handler
  * std::exit: executes static destructors and flushes critical IO and call handler if exists
  * std::quick_exit: doesn't execute static destructors, but does flush critical IO and call handler if exists

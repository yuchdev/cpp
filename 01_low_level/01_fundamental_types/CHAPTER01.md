# Chapter 01. Fundamental Types: Values, Representations, and Contracts

A type in C++ is more than a description of how many bytes occupy memory. It determines which values may exist, which operations are meaningful, how expressions are converted, and what a programmer can safely assume at an interface. These distinctions are particularly important for types that appear almost too simple to require explanation.

This chapter examines `bool`, `std::nullptr_t`, and `void`, and then considers `std::byte`, a library type that deliberately does *not* belong to the fundamental types. Together they illustrate a recurring theme of low-level C++: the semantics of a value, the representation chosen by an implementation, and the meaning assigned to it by an application are three different things.

## 1. Boolean Values and the Arithmetic Type System

The fundamental type `bool` has precisely two values, `false` and `true`. This does not make it independent of arithmetic. C++ retains a long-standing conversion rule under which a `bool` undergoes integral promotion to `int` in arithmetic expressions. Thus:

```cpp
bool a = true;
bool b = true;
auto sum = a + b; // int, with the value 2
```

There is nothing ambiguous about the result: the operands are promoted, and addition is performed on integers. The surprising part is conceptual. Two truth values have been used to compute a count. That may be intentional, but the program should make the change of domain apparent rather than assume that a boolean expression always remains boolean.

When an arithmetic value is converted *to* `bool`, zero becomes `false` and any nonzero value becomes `true`. Pointers can also be contextually converted to `bool`, as in `if (p)`; an ordinary implicit conversion to `bool` is permitted in certain contexts, while copy-initialization such as `bool b = p;` is not generally permitted because the pointer-to-bool conversion is explicit. An object can therefore be used as a condition without making it a good choice for a general-purpose boolean API.

### 1.1 Values are not representations

Although `bool` has only two valid values, its representation is implementation-dependent. `sizeof(bool)` is at least one byte and commonly equals one, but that does not imply that it uses exactly one bit, nor does it specify the bit patterns used for valid values. A program should not interpret arbitrary serialized bytes as a `bool` or manufacture a `bool` by placing an invalid object representation in memory. The fact that an observed build prints `0` or `1` is not a portable representation contract.

This distinction also explains why a collection of boolean values is not automatically bit-packed. If storage efficiency matters, it should be an explicit design decision: a bit field, `std::bitset`, or a purpose-built packed representation may be appropriate.

Formatting adds a more visible representation choice. By default, streams render boolean values numerically. The `std::boolalpha` manipulator requests textual output and persists as a stream formatting state until changed:

```cpp
#include <iostream>

std::cout << true << '\n';                    // 1
std::cout << std::boolalpha << true << '\n';  // true
```

Logging systems may make the same `1`/`0` choice unless configured otherwise. A readable diagnostic format is not evidence of an object's in-memory representation.

### 1.2 References, proxies, and generic code

A genuine `bool&` is a reference to a real `bool` object. Integral promotion does not itself make that reference unsafe. The practical difficulty is that generic code may assume an expression producing a boolean-like result denotes such an object, when it actually denotes a temporary or proxy. Returning a reference to a temporary is dangerous regardless of the type, and a proxy cannot be substituted indiscriminately for `bool&`.

The most familiar example is `std::vector<bool>`. Unlike ordinary `std::vector<T>` instantiations, this specialization may pack elements into bits. Individual bits are not addressable `bool` objects, so element access returns a proxy capable of reading and modifying a bit:

```cpp
#include <vector>

std::vector<bool> flags{true};
auto bit = flags[0];  // proxy object, not bool&
bool value = flags[0]; // an independent bool value
```

The proxy can refer back to the packed storage; a copied proxy therefore does not behave like a copied `bool`. This specialization is a historical space optimization with substantial consequences for generic algorithms and reference semantics. The issue is not simply that it behaves unusually, but that the usual expectations for `vector<T>::reference` do not hold.

Overload resolution provides another reminder that `bool` participates in the wider type system:

```cpp
void f(int);
void f(bool);

f('a'); // f(int): char undergoes integral promotion
```

A small change to the overload set or the argument type can alter which overload is preferred. The relevant rule is the ranking of implicit conversion sequences, not the apparent semantic resemblance of the argument to a truth value.

### 1.3 A truth value is not a domain model

Parameters such as `bool ownership`, `bool mode`, or `bool success` frequently conceal multiple domain concepts behind the same type. Two consecutive boolean arguments are especially difficult to interpret at a call site. A scoped enumeration makes the alternatives visible:

```cpp
enum class Ownership { borrowed, transferred };

void attach(Resource&, Ownership);
```

Not every boolean requires an enumeration. A predicate that actually answers a yes-or-no question is naturally represented by `bool`. The distinction is between a truth value and a choice whose alternatives have names and policies.

Finally, `std::atomic<bool>` allows a boolean flag to participate in inter-thread synchronization. It *may* be lock-free, but that is a property of the implementation and target, not a guarantee associated with its small value domain. Lock-freedom can be queried; memory ordering and the correctness of the protocol still have to be designed. Hardware may implement the operations with ordinary atomic loads/stores, read-modify-write instructions, or another mechanism. No specific bit-test instruction is promised.

## 2. Null Pointers and the Type of `nullptr`

Before C++11, programmers commonly used `0` or `NULL` to express a null pointer. Both conventions invite confusion because the syntax also belongs to the integer domain. C++11 introduced `nullptr` to distinguish the intent to supply a null pointer from the intent to supply an integer.

Strictly speaking, `nullptr` is not a pointer object. It is a prvalue of the fundamental type `std::nullptr_t` (also obtainable as `decltype(nullptr)`). A null pointer value of a suitable pointer type can be formed from it; the literal itself does not have a pointer type.

```cpp
void f(int);
void f(char*);

f(nullptr); // f(char*)
f(NULL);    // may select f(int) or be ambiguous, depending on NULL
```

The first call chooses the pointer overload because `nullptr` converts to `char*`, not to `int`. The behavior of the second call depends on how the implementation defines `NULL`, which is why `nullptr` should be preferred in new C++ code.

### 2.1 A type with one value

There is only one value of `std::nullptr_t`. It can be copied, passed, stored in a named object, and used as a function argument. Its existence as a real type is useful to overload resolution:

```cpp
#include <cstddef>

void accept(std::nullptr_t);
void accept(void*);

accept(nullptr); // exact match: accept(std::nullptr_t)
```

A template can likewise recognize an explicitly supplied null literal:

```cpp
template<class T>
void inspect(T);

inspect(nullptr); // T is std::nullptr_t
```

That makes it possible to specialize an API for an explicit null argument, rather than guessing from the integer value zero. Such distinctions can also participate in SFINAE constraints or C++20 concepts. Of course, an overload can distinguish the *argument type*, not reliably infer the caller's intention in all cases; a typed null pointer such as `static_cast<int*>(nullptr)` has type `int*`.

A null pointer constant can be converted to an object pointer, a function pointer, or a pointer-to-member of an appropriate type:

```cpp
struct C { int member; };

int* p = nullptr;
void (*function)() = nullptr;
int C::* member = nullptr;
```

The pointer-to-member conversion is especially useful because a pointer-to-member is not an ordinary object pointer. An arbitrary integer expression does not acquire these conversions merely because its runtime value happens to be zero.

### 2.2 What may be compared and what may be addressed?

The null pointer literal is not an integer or a boolean. In particular, `nullptr == false` is ill-formed; `nullptr` cannot be used as a general substitute for zero. In a contextual conversion to `bool`, however, a null pointer state is false. For a portable pointer comparison, use a pointer expression on the other side:

```cpp
int* p = nullptr;
bool is_null = (p == nullptr); // well-formed
```

Avoid presenting `nullptr == 0` as an idiomatic or portable comparison; comparison rules and historical defect resolutions make integer-zero examples misleading. The unambiguous formulation compares a pointer with `nullptr`.

Nor can the literal itself be addressed:

```cpp
// auto* address = &nullptr; // ill-formed
```

A *named object* of type `std::nullptr_t` is a different matter: it is an object and has an address. This is not a contradiction. Literals are not automatically addressable lvalues.

Null pointer expressions are particularly useful in constant evaluation:

```cpp
constexpr int* p = nullptr;
static_assert(p == nullptr);
```

No runtime sentinel or implementation-specific bit pattern is needed. The guarantee concerns the null pointer *value*, not the bytes with which an implementation represents it.

## 3. `void`: The Absence of a Value

The fundamental type `void` represents the absence of a value. It does not contain a distinguished value analogous to `false`, zero, or a null pointer; it has no objects and no valid object representation.

```cpp
// void v;       // ill-formed: cannot define a void object
// sizeof(void); // ill-formed in standard C++
```

The most familiar use is a function that does not return a value:

```cpp
void report();
```

That declaration is different from one returning an object whose value happens to mean “no result.” `void` also appears as a cast target for deliberately discarding a value, and as the pointed-to type of `void*`. These uses are related by their absence of a specific object-value type, but they should not be conflated.

### 3.1 Generic object pointers are not typeless pointers

An object pointer can be converted to `void*`, retaining enough information to convert back to the original object-pointer type. To use the object through the recovered pointer, its actual type and lifetime must justify the access:

```cpp
int x = 42;
void* erased = &x;
int* restored = static_cast<int*>(erased);
int value = *restored;
```

This is useful at C interfaces, allocation boundaries, memory-management facilities, and other low-level abstractions. It is not a substitute for an interface that knows its data model. At higher levels, templates, `auto`, `std::any`, or `std::variant` often express the intended abstraction more precisely.

C++ does not define arithmetic on `void*`. A `void` object has no size, so advancing such a pointer by “one element” has no standard meaning:

```cpp
void* p = nullptr;
// ++p; // ill-formed in standard C++
```

Some GNU modes accept `void*` arithmetic as an extension, effectively treating the step as a byte. That behavior is nonportable. When working with an object representation, use `unsigned char*` or `std::byte*` with valid bounds and lifetime.

A `void*` is specifically a generic **object** pointer. Standard C++ does not generally permit an implicit conversion from a function pointer to `void*`, and pointers-to-members form a separate category:

```cpp
void function();
void (*fp)() = &function;
// void* raw = fp; // not a standard implicit conversion
```

### 3.2 Discarded values and expressions

The conventional cast to `void` explicitly signals that an expression is evaluated for its effects while its result is discarded:

```cpp
(void)side_effect();
```

A void expression can also serve as an operand of a comma expression, under the relevant grammar and operand rules. The important observation is that the expression can be evaluated without producing an object value for later use.

Return types can participate in describing overloads, but C++ cannot overload functions *only* by return type. The following ordinary overload set is distinguished by its parameter lists:

```cpp
void f();
int f(int);

f();   // calls void f()
f(42); // calls int f(int)
```

It would be incorrect to attribute the selection to `void` itself.

### 3.3 `std::void_t` and compile-time detection

C++17 supplied the library alias template `std::void_t`, which converts a well-formed sequence of template type arguments to `void`. Its conceptual definition is simple:

```cpp
template<class...>
using void_t = void;
```

Its usefulness lies in *substitution*. An ill-formed dependent type in a substitution context can make a specialization inapplicable rather than immediately rejecting the whole program:

```cpp
#include <type_traits>

template<class T, class = void>
struct has_value_type : std::false_type {};

template<class T>
struct has_value_type<T, std::void_t<typename T::value_type>>
    : std::true_type {};
```

If `T::value_type` denotes a type, the partial specialization can match. Otherwise substitution fails, and the primary template applies. This is the familiar detection idiom, used to discover optional members and valid expressions at compile time. Concepts and `requires` expressions provide more direct ways to express many such requirements in C++20, but understanding `void_t` remains valuable when reading earlier generic libraries.

## 4. `std::byte`: Giving Object Representation Its Own Vocabulary

`std::byte` is not a fundamental type. It was introduced by the C++17 library as a scoped enumeration, declared in `<cstddef>`, to represent raw bytes without assigning them the semantics of integers.

```cpp
#include <cstddef>

std::byte b{0xFF}; // valid if the value fits the underlying type
// int n = b;      // ill-formed: no implicit conversion to int
```

The first initialization is valid: direct-list-initialization of `std::byte` from a suitable integer constant expression is permitted when the value is representable. Treating it as ill-formed would confuse the restriction on *implicit arithmetic conversion* with the rules for initialization.

A `std::byte` can be combined using the supported bitwise operators:

```cpp
std::byte flags{0};
flags |= std::byte{0x01};
auto shifted = flags << 1;
```

Ordinary arithmetic such as addition is not supplied. When a numeric interpretation is necessary, request one explicitly with `std::to_integer`:

```cpp
int bits = std::to_integer<int>(flags);
```

This is the central benefit: an expression that manipulates representation is visibly different from one that computes a number. `unsigned char` continues to be useful and has special language privileges for examining object representations, but `std::byte` better communicates the absence of arithmetic intent.

For a live object, its representation may be inspected through a pointer to `std::byte`, subject to bounds and the rules governing accessible object representations:

```cpp
int object = 42;
auto* raw = reinterpret_cast<std::byte*>(&object);
// Inspect sizeof(object) bytes; do not treat them as portable serialized data.
```

Such access is among the exceptions to ordinary type-based aliasing restrictions. It does not suspend object lifetime rules, justify writing arbitrary byte sequences into any object, or guarantee a stable representation between platforms. A valid memory view is not automatically a valid serialization format.

The conceptual gap between “integer” and “raw storage” deserves a name. `std::byte` fills it, and by preventing accidental arithmetic it makes many low-level interfaces more self-documenting.

## 5. From Language Rule to Engineering Contract

The cases above differ in detail, but they expose the same design problem. The language guarantees some properties; implementations choose representations and optimization strategies; applications impose requirements that neither the language nor the ABI can infer.

A boolean can represent a predicate, but it cannot by itself explain an ownership protocol. A null pointer value can mean “no object,” but it does not manage the lifetime of an object once present. `void*` can carry an object address without preserving its static type. `std::byte` can expose raw storage without making its contents into a portable file format.

A useful review asks three questions. First, *what does the language guarantee* about the operation? Second, *which observations belong only to a particular implementation*—the ABI, memory representation, optimizer, or diagnostic behavior? Third, *which promises must the application supply*—valid input ranges, lifetimes, ownership, thread synchronization, and cross-interface conversions?

The answers lead to a few enduring rules. Use `bool` for truth and make domain transitions explicit when arithmetic promotions are involved. Use `nullptr` rather than the integer-zero conventions of legacy pointer code. Treat `void` as absence of an object value, not as an unknown object type. Use `std::byte` when memory must be manipulated as representation rather than as a number.

The common failures are now easy to recognize. A `bool` silently promoted into arithmetic obscures what is being counted. `NULL` in an overloaded call may select an integer overload. Ordinary arithmetic on `std::byte` is intentionally unavailable; convert with `std::to_integer` when a number is actually required. And `void*` arithmetic is not standard C++, even if a permissive compiler accepts it.

> A valid low-level operation requires a language-level contract, not merely a plausible machine-level result.

When reviewing code, it is therefore worth checking whether the implementation-dependent assumptions have been identified, whether conversion and lifetime preconditions are explicit, whether boundary cases can be tested without invoking undefined behavior, and whether the API's types encode the intended contract as far as practicable.

## 6. Diagnostics, Toolchains, and Extensions

Compiler diagnostics are an important part of enforcing these distinctions, but no warning configuration can reconstruct an application's semantic requirements. A warning-free build cannot establish that a `bool` is an appropriate domain type, that a cast pointer denotes a live object, or that a byte sequence is suitable for transmission across machines.

For GCC and Clang, a productive starting point is:

```text
-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
```

For MSVC, `/W4 /permissive-` provides broad diagnostics while reducing acceptance of nonconforming constructs. GCC and Clang differ in the warnings they implement and in the patterns each one detects. Higher warning levels are a way of finding questions worth investigating, not a proof of correctness.

Runtime tools address a different layer. AddressSanitizer (`-fsanitize=address`, or MSVC `/fsanitize=address`) can detect many invalid memory accesses and lifetime-related faults. GCC and Clang's UndefinedBehaviorSanitizer (`-fsanitize=undefined`) instruments a range of undefined operations, although its coverage is necessarily incomplete. Static analysis through clang-tidy, cppcheck, and MSVC `/analyze` can identify suspicious conversions, lifetimes, and interface patterns without executing the relevant paths. These tools complement testing rather than replace contract design.

### 6.1 Options that affect semantics

Language mode matters. The difference between `-std=c++20` and `-std=gnu++20` is not just a spelling convention: GNU dialects may admit extensions, including nonstandard expressions that would be rejected in a stricter ISO mode. Prefer ISO modes for examples intended to teach portable behavior, and consider `-pedantic-errors` for stricter conformance checks.

Several compiler switches have particularly far-reaching consequences. `-fno-strict-aliasing` restricts certain alias-based optimization assumptions but does **not** create objects or repair invalid lifetimes. `-fwrapv` specifies wrapping behavior for signed overflow under the toolchain's documented contract; it should not be treated as an ISO C++ guarantee. `-ffast-math` permits floating-point transformations incompatible with strict numerical assumptions, so it should generally be excluded from examples that rely on reproducibility or the floating-point environment. These switches are not interchangeable performance knobs: each changes what a particular toolchain is permitted to assume.

### 6.2 Useful extensions and their limits

Clang's `__builtin_assume` and MSVC's `__assume` communicate optimizer assumptions without checking them. C++23's `[[assume]]`, where implemented, provides a standard spelling but likewise requires that its premise be valid. GCC and Clang offer `__builtin_object_size` to estimate available storage in certain situations; an explicit extent passed through an API remains more dependable for reasoning about its contract. Attributes written with `__attribute__` or `__declspec` can express vendor-specific layout and linkage properties, but standard attributes should be preferred when they fit the requirement.

Portability improves when extensions are hidden behind small wrappers and selected with feature tests such as `__cpp_*` macros rather than guessed solely from a compiler version. The C++ Core Guidelines provide a useful vocabulary for reviewing preconditions, and Microsoft's Guidelines Support Library (GSL) supplies non-owning and bounds-oriented interface types. Compiler Explorer helps compare generated code when a performance claim depends on the target and compiler. None of these resources substitutes for a precise type and lifetime contract.

The practical strategy is layered: expressive interfaces first, warnings and static analysis next, targeted tests and sanitizer runs where appropriate, and carefully isolated extensions when a measured requirement justifies them.

## 7. The Standards Timeline

These facilities evolved at different times. The distinction is important because code can depend on a very old language rule and a much newer library vocabulary at the same time.

**C++98 and C++03.** The established model already included `bool` and `void`, integral promotions, object pointers, and the familiar integer-zero null pointer conventions. Those conventions explain much of the legacy code still encountered today.

**C++11.** The introduction of `nullptr` and `std::nullptr_t` separated a dedicated null pointer literal from integer zero, improving overload resolution and template deduction. The underlying meanings of `bool` and `void` were not replaced.

**C++14.** There was no comparable fundamental change to the four categories discussed here; refinements elsewhere in the language did not remove the established conversion rules.

**C++17.** The library introduced `std::byte` as a representation-oriented type. The standard also added `std::void_t` to support compile-time detection idioms.

**C++20.** Concepts and `requires` expressions made many generic constraints more direct, and expanded constant-evaluation facilities improved compile-time reasoning around such types.

**C++23.** The core semantics discussed here remained substantially stable, while other language and library facilities, including the `[[assume]]` attribute, extended the surrounding engineering toolbox.

**C++26.** New library facilities may continue building on the same scalar and object-model foundations. Whether an individual proposal or facility is implemented must be checked against the applicable standard and the toolchain's actual support; a compiler's brand or version is not sufficient evidence.

A useful compatibility statement therefore gives the language or library version required by each feature, and distinguishes standard guarantees from implementation extensions.

## 8. Moving from Java, Python, or C#

Programmers arriving from managed environments may recognize the surface syntax of booleans, null-like values, and generic data while expecting different failure and lifetime rules. The most important adjustment is to translate a runtime expectation into an explicit C++ type, ownership model, or precondition.

Java has a primitive `boolean` and the reference value `null`; Python has `bool` and the object `None`; C# has `bool` and `null` for applicable types. None is an exact equivalent of the distinct C++ roles played by `bool`, `std::nullptr_t`, `void`, and `std::byte`. Python's `bool` is also a subclass of `int`, but that fact should not be mistaken for C++'s integral promotion rules.

Managed-language invalid operations often raise exceptions or are subject to runtime checks. In C++, an invalid expression may be rejected at compile time, produce a specified result, have unspecified behavior, or invoke undefined behavior, depending on the operation. A successful local test establishes what one execution happened to do; it cannot establish portability or eliminate undefined behavior.

Lifetime is equally different. Java and C# ordinarily use garbage-collected object reachability. Python implementations commonly use reference counting and/or garbage collection, with details depending on the implementation. C++ combines automatic storage duration, dynamic allocation, ownership-aware library types, and explicitly non-owning pointers, references, and views. Merely holding a non-owning handle does not extend the pointee's lifetime.

At interfaces, prefer a meaningful scoped enumeration to a boolean that encodes several modes, `nullptr` to an integer sentinel for missing pointers, templates or `std::variant`/`std::any` to unnecessary `void*` type erasure, and a proper owning container or checked API when bounds and lifetime must be managed. `std::byte` is useful when raw memory representation is genuinely part of the task. In each case, choose the mechanism for the guarantee it offers—not because it superficially resembles a construct from another language.

## 9. Further Reading

For exact language rules and library contracts, consult the C++ standard for the chosen revision and the corresponding reference material:

- [C++ language types](https://en.cppreference.com/w/cpp/language/types)
- [`std::byte`](https://en.cppreference.com/w/cpp/types/byte)
- [`std::nullptr_t`](https://en.cppreference.com/w/cpp/types/nullptr_t)

The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) complement these references with interface and lifetime guidance. Compiler manuals and implementation feature-test results are the appropriate sources when an example deliberately relies on an extension.

The general lesson is straightforward but demanding: a type tells us what operations the language allows; it does not automatically tell us whether those operations express the program's purpose. Good low-level C++ makes both contracts visible.

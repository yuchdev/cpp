# 05. Pointers, References, Alignment, and Object Lifetime in Modern C++

Pointers look simple because their surface syntax is old:

~~~cpp
int value = 42;
int* p = &value;
std::cout << *p;
~~~

But in modern C++, the difficult part is rarely the `*` or `&` syntax. The difficult part is understanding **what the pointer is allowed to mean**.

A pointer participates in several overlapping models:

* the C++ object and lifetime model;
* array bounds and pointer arithmetic;
* type accessibility and aliasing;
* alignment;
* ownership and resource lifetime;
* iterator and range semantics;
* ABI-specific representation;
* integer/address conversions;
* function and member-pointer calling conventions;
* concurrency and synchronization;
* optimizer assumptions about provenance, lifetime, and aliasing.

This chapter is aimed at experienced C++ developers. It concentrates on the places where "a pointer is just an address" is an actively misleading mental model.

The examples in this directory are:

1. [`01_pointer_facts/pointer_facts.cpp`](01_pointer_facts/pointer_facts.cpp) — pointer categories, representation, arithmetic, aliasing, ownership, object lifetime, `std::span`, `std::to_address`, and selected C++23 facilities.
2. [`02_compare_pointers/compare_pointers.cpp`](02_compare_pointers/compare_pointers.cpp) — equality, relational comparison, total pointer ordering, and C++20 three-way comparison.
3. [`03_alignment/alignment.cpp`](03_alignment/alignment.cpp) — padding, over-alignment, `std::assume_aligned`, aligned allocation, and the distinction between storage and lifetime.
4. [`04_references/reference_facts.cpp`](04_references/reference_facts.cpp) — references, lifetime extension, forwarding, ranges lifetime modeling, and C++23 reference utilities.

The repository currently builds as C++20. C++23-only examples are feature-tested and compiled when the standard library provides them.

---

# 1. A pointer is not merely an integer containing an address

On ordinary x86-64 machines, an object pointer often *looks* exactly like a 64-bit virtual address. That implementation fact is useful when debugging, but it is not the C++ semantic model.

Consider:

~~~cpp
int x = 42;
int* p = &x;
~~~

The useful facts about `p` include:

* its type is `int*`;
* it points to the live `int` object `x`;
* dereferencing it accesses that object;
* arithmetic on it is constrained by the array-object rules;
* its alignment is suitable for `int`;
* its lifetime as a usable pointer to `x` ends when `x`'s lifetime ends.

The numeric bit pattern of the pointer is only one implementation detail.

This distinction becomes important on architectures with:

* segmented address spaces;
* capability pointers;
* pointer authentication;
* tagged pointers;
* multiple address spaces;
* non-flat function-pointer representations;
* metadata associated with pointer bounds or permissions.

It also matters to optimizers even on ordinary x86-64.

## 1.1 Pointer categories are distinct

C++ has several categories that programmers casually call "pointers":

~~~cpp
int* object_pointer;

void (*function_pointer)();

int Class::* data_member_pointer;
void (Class::* member_function_pointer)();
~~~

They are not interchangeable types.

In particular, a pointer-to-member is not necessarily an address at all.

Given:

~~~cpp
struct Base1 { int a; };
struct Base2 { int b; };

struct Derived : Base1, Base2 {
    int c;
};
~~~

a pointer-to-member may need enough information to adjust `this` before finding the selected subobject. A pointer to a virtual member function can require still more ABI machinery.

Therefore:

~~~cpp
sizeof(void*)
sizeof(void (*)())
sizeof(int Derived::*)
sizeof(void (Derived::*)())
~~~

need not be equal.

Never serialize a pointer-to-member by pretending it is a `void*` or integer address.

## 1.2 Object pointers and `void*`

A pointer to an object type can be converted to `void*`:

~~~cpp
int x = 1;

int* ip = &x;
void* vp = ip;
~~~

and converted back to the original pointer type:

~~~cpp
int* again = static_cast<int*>(vp);
~~~

This is one reason C APIs use `void*` as an erased object-pointer type.

But `void` has no size and no objects of type `void`, so this is ill-formed in standard C++:

~~~cpp
++vp;
*vp;
~~~

GNU C/C++ modes may provide `void*` arithmetic as an extension. Portable C++ does not.

## 1.3 A function pointer is not an object pointer

This is not a portable implicit conversion:

~~~cpp
void f();

void* p = &f; // ill-formed
~~~

Some implementations support explicit conversions between function pointers and object pointers as an extension or conditionally-supported behavior, often because the ABI uses compatible machine representations.

Do not turn that ABI convenience into a generic C++ assumption.

If an API needs to carry a callback, use the appropriate function-pointer type, callable wrapper, or an API-defined opaque representation.

## 1.4 Null pointer value does not mean "all bits zero" as a language rule

`nullptr` produces a null pointer value when converted to a pointer type:

~~~cpp
int* p = nullptr;
~~~

C++ specifies the semantic value, not a universal physical bit pattern.

On mainstream systems, null object pointers are usually represented by zero bits. Portable serialization, memory initialization, and low-level protocols should not infer the language rule from that convention.

For example, `calloc` zeroes bytes. The C++ language does not define "all-bits-zero object representation" as the universal representation of every null pointer type.

---

# 2. `nullptr`, null pointer constants, and overload resolution

Before C++11, null pointers were commonly written as:

~~~cpp
f(0);
f(NULL);
~~~

The problem is that `0` is also an integer.

Suppose:

~~~cpp
void f(int);
void f(int*);
~~~

Then:

~~~cpp
f(0);
~~~

calls the integer overload.

`nullptr` has its own type:

~~~cpp
decltype(nullptr) // std::nullptr_t
~~~

and is designed for null pointer conversion:

~~~cpp
int* p = nullptr;
void (*fp)() = nullptr;
int C::* mp = nullptr;
~~~

## 2.1 `std::nullptr_t` is not an integer type

~~~cpp
static_assert(
    !std::is_integral_v<std::nullptr_t>
);
~~~

This matters in generic code.

A template that accepts integral sentinels should not accidentally classify `nullptr` as an integer.

## 2.2 `nullptr` is a prvalue, not an object whose address you can take

This is ill-formed:

~~~cpp
auto p = &nullptr;
~~~

But an object of type `std::nullptr_t` can exist:

~~~cpp
std::nullptr_t n = nullptr;
auto* pn = &n;
~~~

The distinction is the same one encountered with other literal expressions: the literal itself is not a named object.

## 2.3 Null is not dangling

These are very different states:

~~~cpp
int* null = nullptr;
~~~

versus:

~~~cpp
int* dangling;

{
    int x = 42;
    dangling = &x;
} // x dies here
~~~

The first pointer explicitly represents "no object".

The second retains a pointer value associated with an object whose lifetime has ended.

Setting an owning or observing pointer to `nullptr` after invalidation can be a useful defensive convention, but C++ does not automatically null dangling pointers.

---

# 3. Pointer arithmetic is array arithmetic

One of the most important rules in low-level C++ is:

> pointer arithmetic is defined in terms of array objects, not arbitrary numeric addresses.

Given:

~~~cpp
int a[10];

int* p = &a[3];
int* q = p + 4;
~~~

`q` points to `a[7]`.

The increment is scaled by the pointed-to type:

~~~cpp
++p;
~~~

moves from one `int` element to the next `int` element.

This does not mean "add `sizeof(int)` to an integer address" in the language model, even though that is the machine instruction a conventional implementation may ultimately use.

## 3.1 A non-array object behaves like an array of one for limited pointer arithmetic

For pointer arithmetic purposes, a pointer to a complete object can participate in the one-element-array model.

Given:

~~~cpp
int x;
int* p = &x;
~~~

forming:

~~~cpp
int* one_past = p + 1;
~~~

is permitted.

Dereferencing `one_past` is not.

## 3.2 One-past is valid to form, not valid to dereference

For:

~~~cpp
int a[4];
~~~

this pointer value is valid:

~~~cpp
int* end = a + 4;
~~~

It is essential to C and C++ half-open ranges:

~~~text
[first, last)
~~~

But:

~~~cpp
*end
~~~

is undefined behavior.

This distinction is the foundation of standard iterators.

## 3.3 Going before the beginning is not a harmless temporary

Do not reason like this:

~~~cpp
int* p = a - 1; // "I won't dereference it"
++p;
~~~

The invalid pointer arithmetic already occurred when `a - 1` was evaluated.

The fact that the numeric machine address might be representable does not make the C++ pointer operation valid.

## 3.4 Pointer subtraction is constrained to one array object

This is defined:

~~~cpp
int a[100];

auto distance = &a[80] - &a[10];
~~~

The result has type:

~~~cpp
std::ptrdiff_t
~~~

and value `70`.

This is not a portable address-distance operation:

~~~cpp
int a;
int b;

auto distance = &b - &a; // not a valid generic way to measure memory distance
~~~

For unrelated objects, pointer subtraction is undefined.

## 3.5 `ptrdiff_t` is signed for a reason

Array differences can be negative:

~~~cpp
&a[2] - &a[7] // -5
~~~

Therefore pointer subtraction returns `std::ptrdiff_t`, a signed integer type.

`std::size_t`, by contrast, is an unsigned type suitable for object sizes.

Do not mechanically convert iterator/pointer differences to `size_t` before considering whether a negative value is possible.

## 3.6 Even one array can theoretically be too large for `ptrdiff_t`

A subtle standard-library edge case: the size of an object is expressed in `size_t`, while pointer difference is `ptrdiff_t`.

Those types need not have identical positive ranges.

On conventional 64-bit systems both are usually 64 bits, with `size_t` able to represent larger positive values than `ptrdiff_t`.

Extremely large arrays therefore expose representability questions that ordinary code never encounters.

---

# 4. Arrays are not pointers

This declaration:

~~~cpp
int a[10];
~~~

declares an array object.

It does **not** declare a pointer.

In many expressions, an array undergoes array-to-pointer conversion:

~~~cpp
int* p = a;
~~~

so programmers casually say "an array is a pointer".

That shortcut causes bugs.

## 4.1 `sizeof` sees the array

~~~cpp
int a[10];

static_assert(sizeof(a) == 10 * sizeof(int));
~~~

After decay:

~~~cpp
int* p = a;

static_assert(sizeof(p) == sizeof(int*));
~~~

Those are completely different quantities.

## 4.2 Taking the address preserves the array type

~~~cpp
int a[10];

auto p1 = a;   // int*
auto p2 = &a;  // int (*)[10]
~~~

`p1 + 1` advances by one `int`.

`p2 + 1` advances by one whole array of ten `int` objects.

This distinction is useful in multidimensional-array code and low-level APIs.

## 4.3 Function parameters written as arrays are adjusted to pointers

These declarations denote the same function parameter type:

~~~cpp
void f(int a[100]);
void f(int* a);
~~~

The `100` does not make the parameter carry a runtime or compile-time extent.

If extent matters, modern C++ has better interfaces.

## 4.4 Use `std::span` when the API means "contiguous sequence"

C++20:

~~~cpp
void process(std::span<int> values)
{
    for (int& value : values) {
        ++value;
    }
}
~~~

Callers can pass:

~~~cpp
int a[10];
std::array<int, 10> b;
std::vector<int> c;

process(a);
process(b);
process(c);
~~~

`std::span` is non-owning. Conceptually, a dynamic-extent span contains a pointer plus an extent.

That makes it much more expressive than:

~~~cpp
void process(int* p, std::size_t n);
~~~

while retaining essentially the same low-level cost model.

## 4.5 A span does not solve lifetime

This is crucial.

`std::span` does not own the data.

A span can dangle exactly as a raw pointer can:

~~~cpp
std::span<int> bad()
{
    std::vector<int> v{1, 2, 3};
    return v; // returned span dangles
}
~~~

The type improves interface semantics and bounds propagation; it does not magically extend the underlying object's lifetime.

---

# 5. C++20 contiguous iterators and `std::to_address`

Older generic code often assumed:

> contiguous iterator == raw pointer.

That was never a good abstraction.

C++20 formally introduced the `std::contiguous_iterator` concept.

A contiguous iterator guarantees that its elements occupy contiguous storage, but its iterator type does not have to be `T*`.

For example:

~~~cpp
std::span<int> s = ...;

auto it = s.begin();
~~~

Portable code should not require:

~~~cpp
static_assert(std::is_same_v<decltype(it), int*>);
~~~

Instead, C++20 provides:

~~~cpp
int* p = std::to_address(it);
~~~

## 5.1 `std::to_address` is not just `&*it`

The old idiom:

~~~cpp
&*it
~~~

has problems:

* it conceptually dereferences the iterator;
* it cannot be used for a one-past iterator;
* proxy/reference behavior can complicate generic code;
* fancy pointer types need their own address extraction semantics.

`std::to_address` expresses the intended operation directly.

For raw pointers:

~~~cpp
int* p = ...;

static_assert(std::to_address(p) == p);
~~~

For pointer-like types, `pointer_traits` can participate.

## 5.2 Fancy pointers matter in allocators and specialized memory systems

A "pointer" in generic library code may represent memory in:

* shared memory;
* GPU/device memory;
* persistent memory;
* segmented memory;
* custom heaps.

Allocator-aware code should therefore be cautious about assuming:

~~~cpp
Allocator::pointer == T*
~~~

Modern library abstractions deliberately leave room for fancy pointer types.

---

# 6. Pointer comparison is more subtle than comparing addresses

Inside one array, ordering is intuitive:

~~~cpp
int a[10];

assert(&a[2] < &a[7]);
~~~

The higher-subscript element compares greater.

The one-past pointer also participates:

~~~cpp
assert(&a[9] < a + 10);
~~~

The interesting case is unrelated objects.

## 6.1 Equality and ordering answer different questions

Given:

~~~cpp
int x;
int y;

int* px = &x;
int* py = &y;
~~~

equality comparison is meaningful:

~~~cpp
px == py
px != py
~~~

Relational comparison:

~~~cpp
px < py
~~~

does not provide a portable "which numeric address is lower?" abstraction for unrelated complete objects. Depending on the relationship between the pointed-to objects, the standard's pointer comparison rules can leave the relational result unspecified.

This is not the same thing as saying the expression is necessarily undefined behavior.

That distinction matters.

## 6.2 Use `std::less` when you need a total pointer order

The standard library provides a strict total ordering for pointers:

~~~cpp
std::less<>{}(px, py)
~~~

This is useful for:

* ordered containers;
* pointer-keyed indexing;
* implementation-independent generic ordering.

It is specifically designed to provide an ordering even where the built-in relational operators do not give the total order you need.

C++20 ranges provide:

~~~cpp
std::ranges::less{}(px, py)
~~~

with corresponding pointer total-order semantics.

## 6.3 C++20 `<=>` does not turn pointers into portable integers

For pointers where the language defines ordering:

~~~cpp
int a[4];

auto order = &a[0] <=> &a[3];
~~~

the result expresses the ordering naturally.

But `<=>` does not create a universal numeric-address abstraction for arbitrary pointers.

Three-way comparison is new syntax and comparison machinery; it does not erase the underlying pointer model.

## 6.4 Pointer ordering is not ownership ordering

Two `shared_ptr` objects can have ownership relationships that are not captured by comparing the raw pointer returned by `get()`.

For ownership-based ordering, `std::owner_less` exists.

This matters with aliasing `shared_ptr` instances:

~~~cpp
struct S {
    int x;
    int y;
};

auto owner = std::make_shared<S>();
std::shared_ptr<int> px(owner, &owner->x);
std::shared_ptr<int> py(owner, &owner->y);
~~~

`px` and `py` contain different stored pointers but share the same ownership control block.

Raw address identity and ownership identity are separate concepts.

---

# 7. Pointer-to-integer conversion: useful diagnostic tool, poor identity model

`<cstdint>` may provide:

~~~cpp
std::intptr_t
std::uintptr_t
~~~

These types are optional.

If the implementation cannot provide an integer type with the required pointer-conversion capability, it need not define them.

## 7.1 `uintptr_t` is not "the pointer type without the star"

This can be useful:

~~~cpp
std::uintptr_t bits =
    reinterpret_cast<std::uintptr_t>(p);
~~~

Typical uses include:

* diagnostics;
* hashing on known platforms;
* low-level runtime implementation;
* ABI code;
* hardware/register interfaces;
* extracting known tag bits under a platform contract.

But do not conclude that the result is a portable physical or virtual address suitable for arithmetic.

## 7.2 Integer round-trip and arbitrary integer fabrication are different operations

A pointer converted to a sufficiently capable integer type and converted back under the specified conditions can preserve the original pointer value.

That does not imply:

~~~cpp
auto p = reinterpret_cast<int*>(0x12345678);
~~~

produces a pointer that can legally be dereferenced.

An integer bit pattern does not create an object, begin a lifetime, satisfy alignment, map memory, or establish an API's access rights.

## 7.3 Never serialize process pointers as durable identity

A raw address normally becomes meaningless across:

* process restart;
* ASLR relocation;
* another machine;
* another process;
* allocator reuse;
* object destruction.

Serialization should use stable identifiers, offsets within a defined format, handles, or protocol-level references.

## 7.4 Pointer tagging is platform code

Many runtimes exploit alignment:

~~~text
...xxxx000
~~~

and use low bits for tags.

This can be effective, but it depends on more than `alignof(T)`.

Potential complications include:

* capability architectures;
* memory tagging;
* sanitizers;
* pointer authentication;
* garbage collectors;
* ABI rules;
* provenance-aware optimization.

Treat tagged pointers as a platform/runtime abstraction with explicit tests and encapsulation, not as a generic C++ trick.

---

# 8. Alignment: address divisibility is only half the story

Every complete object type has an alignment requirement:

~~~cpp
alignof(T)
~~~

An object of type `T` must be placed in storage satisfying that requirement.

## 8.1 Padding is observable through size and offsets

Consider:

~~~cpp
struct S {
    char c;
    int i;
    double d;
};
~~~

The naïve payload size is:

~~~text
1 + sizeof(int) + sizeof(double)
~~~

but `sizeof(S)` can be larger because padding may appear:

* between members;
* after the last member.

Trailing padding ensures that elements of:

~~~cpp
S array[100];
~~~

are each correctly aligned.

## 8.2 The class alignment need not be described as "exactly the largest member alignment"

For an ordinary struct, the largest natural member alignment often determines the struct's alignment.

But:

~~~cpp
struct alignas(64) S {
    int x;
};
~~~

is an immediate counterexample to the simplistic rule.

Better:

> a class must be aligned sufficiently for its members, and its alignment can be strengthened.

## 8.3 `alignas` can request stronger alignment

~~~cpp
struct alignas(64) CacheLine {
    std::byte data[64];
};
~~~

Possible reasons include:

* SIMD instructions;
* DMA/hardware requirements;
* cache-line placement;
* false-sharing mitigation;
* lock-free data structures;
* custom allocators.

But `alignas(64)` is not itself a guarantee that your type occupies one unique hardware cache line in every relevant system topology.

Hardware cache-line assumptions are separate from C++ object alignment.

## 8.4 Over-aligned dynamic allocation changed significantly in C++17

Modern ordinary `new` supports over-aligned object types through aligned allocation machinery when required.

That means:

~~~cpp
struct alignas(64) X {
    int value;
};

auto p = std::make_unique<X>();
~~~

is expected to obtain suitably aligned storage in conforming C++17+ implementations.

Legacy custom allocators and C allocation functions deserve extra scrutiny.

## 8.5 `malloc` and over-alignment are not interchangeable assumptions

C allocation functions provide alignment suitable for the types guaranteed by their contract, but code requiring extended alignment should use an API that explicitly guarantees the requested alignment.

Depending on the environment, that might be:

* aligned `operator new`;
* an allocator;
* `std::aligned_alloc`;
* platform APIs.

Always match the corresponding deallocation mechanism.

## 8.6 C++20 `std::assume_aligned` does not align memory

This:

~~~cpp
T* aligned =
    std::assume_aligned<32>(p);
~~~

is a promise to the implementation that `p` already satisfies the alignment.

It is not:

* a check;
* an allocation;
* a pointer-adjustment operation.

If the promise is false, you have violated the function's precondition and given the optimizer invalid information.

This is similar in spirit to other optimizer-facing assertions: powerful when the invariant is already guaranteed, dangerous when used as wishful thinking.

## 8.7 Aligning every member can waste enormous space

Compare:

~~~cpp
struct alignas(16) Vec4 {
    float x;
    float y;
    float z;
    float w;
};
~~~

with:

~~~cpp
struct WeirdVec4 {
    alignas(16) float x;
    alignas(16) float y;
    alignas(16) float z;
    alignas(16) float w;
};
~~~

The first asks for one 16-byte-aligned aggregate.

The second may force each individual scalar onto a 16-byte boundary, producing a much larger object.

Alignment is a layout requirement, not a generic "make SIMD faster" switch.

---

# 9. Storage, object lifetime, and pointer validity are separate concepts

One of the deepest changes in modern low-level C++ education is the move away from thinking:

> memory contains bytes, therefore I can cast those bytes to `T*` and use them as a `T`.

The language has a more precise object model.

You need to distinguish:

1. storage exists;
2. storage is sufficiently large;
3. storage is correctly aligned;
4. an object of the required type has begun its lifetime there;
5. access through the chosen glvalue/pointer type is permitted.

Passing one condition does not automatically establish the others.

## 9.1 Aligned byte storage does not automatically mean "live T"

~~~cpp
alignas(T) std::byte storage[sizeof(T)];
~~~

This gives suitable storage.

It does not universally mean that an arbitrary non-implicit-lifetime `T` object is already alive there.

C++20 gives a direct lifetime-oriented API:

~~~cpp
T* p = std::construct_at(
    reinterpret_cast<T*>(storage),
    constructor_arguments...
);
~~~

Later:

~~~cpp
std::destroy_at(p);
~~~

This vocabulary is preferable to manually spelling placement construction/destruction in generic code.

## 9.2 After destruction, the address can remain while the object is gone

~~~cpp
std::destroy_at(p);
~~~

does not erase the bytes or necessarily change the numeric pointer representation.

But using:

~~~cpp
p->member
~~~

after the object's lifetime has ended is invalid.

This is the clearest demonstration of why:

> pointer == address

is insufficient.

The address can still exist while the object semantics no longer do.

## 9.3 Placement new can reuse storage

Classic low-level code uses:

~~~cpp
T* p = ::new (storage) T(args...);
~~~

The storage may previously have held another object.

This appears in:

* allocators;
* containers;
* variants;
* object pools;
* arenas;
* optional-like types;
* embedded systems.

Correctness depends on both the old and new object lifetimes, not merely storage capacity.

## 9.4 `std::launder` is specialized, not a routine "make cast legal" tool

C++17 introduced:

~~~cpp
std::launder(p)
~~~

for particular situations where a new object is created in storage and the optimizer's object-identity assumptions require obtaining a usable pointer to that new object.

It is not:

* a strict-aliasing escape hatch;
* a misalignment repair;
* a way to legalize arbitrary casts;
* a substitute for beginning object lifetime.

If you find yourself adding `std::launder` experimentally until code "works", the lifetime model should be revisited first.

## 9.5 C++23 `std::start_lifetime_as` makes an important low-level operation explicit

C++23 adds `std::start_lifetime_as` and related facilities for suitable implicit-lifetime types.

Conceptually, they address a common systems-programming need:

> I have suitably aligned storage containing an object representation; establish an object of type `T` in that storage under the facility's rules.

This is especially relevant to:

* binary I/O;
* memory-mapped storage;
* shared-memory layouts;
* packet parsing;
* serialization infrastructure.

It does not mean arbitrary serialized bytes automatically form a valid value of every C++ type.

Representation validity, endianness, padding, invariants, and pointer-containing types remain separate issues.

---

# 10. Aliasing: `reinterpret_cast` changes a type, not the access rules

This is one of the most common low-level misconceptions:

~~~cpp
float f = 1.0f;
int* p = reinterpret_cast<int*>(&f);
std::cout << *p;
~~~

The cast can produce a pointer expression of the requested type.

That does **not** imply that dereferencing it is a permitted way to read the `float` object.

The optimizer uses type-access rules to reason about which expressions can refer to the same stored value.

Violating those assumptions can produce code that appears to work at `-O0` and changes at `-O2`.

## 10.1 Character and byte types are special for object representation

C++ allows inspecting an object's representation through character/byte access.

Modern C++20 code can make the intent clearer with:

~~~cpp
std::span<const T> object{&value, 1};
auto bytes = std::as_bytes(object);
~~~

Now the code says exactly what it means:

> view the representation bytes.

## 10.2 `std::bit_cast` is for representation conversion

C++20:

~~~cpp
std::uint32_t bits =
    std::bit_cast<std::uint32_t>(f);
~~~

subject to the size and trivially-copyable requirements.

This is not numeric conversion.

Compare:

~~~cpp
static_cast<std::uint32_t>(f)
~~~

which converts the numeric value, with:

~~~cpp
std::bit_cast<std::uint32_t>(f)
~~~

which copies/interprets the object representation according to `bit_cast` semantics.

These operations answer entirely different questions.

## 10.3 `memcpy` remains a fundamental low-level tool

Before C++20, `memcpy` was often the portable representation-copy technique:

~~~cpp
std::uint32_t bits;
std::memcpy(&bits, &f, sizeof bits);
~~~

Compilers understand this idiom well and commonly optimize away the apparent memory copy.

Do not replace a well-defined `memcpy` with undefined pointer punning merely because the latter looks "lower level".

## 10.4 C++ and C aliasing folklore are not identical

Be careful importing rules from:

* C;
* GCC extensions;
* compiler documentation;
* kernel coding conventions.

For example, union-punning idioms have historically had different portability expectations in C and C++.

When writing standard C++, prefer the standard facilities that directly express representation transfer.

---

# 11. Endianness and pointer semantics are different topics

Suppose:

~~~cpp
std::uint32_t x = 0x01020304;
~~~

On a little-endian machine, the lowest-address byte is commonly `0x04`.

On a big-endian machine, it is commonly `0x01`.

C++20 exposes native byte order through:

~~~cpp
std::endian::native
~~~

from `<bit>`.

But endianness describes the byte representation of multi-byte scalar values. It does not mean "pointers increment backwards" or otherwise alter ordinary array indexing.

## 11.1 Network and file formats need explicit byte order

Never serialize a native integer by dumping its bytes unless the format explicitly uses native representation.

Portable formats should define:

* byte order;
* field width;
* signed representation where relevant;
* floating representation if floats are stored;
* padding policy;
* versioning.

C++23's `std::byteswap` is useful for byte-order conversion of integer values:

~~~cpp
auto swapped = std::byteswap(value);
~~~

It does not by itself define a serialization format.

---

# 12. Ownership is not encoded by raw-pointer syntax

These two declarations look identical:

~~~cpp
Widget* a;
Widget* b;
~~~

but one program might intend:

~~~text
a owns a dynamically allocated Widget
b merely observes a Widget owned elsewhere
~~~

The type system does not distinguish those meanings.

This ambiguity is a major source of lifetime bugs.

## 12.1 Prefer ownership types for ownership

Modern C++ normally expresses exclusive dynamic ownership with:

~~~cpp
std::unique_ptr<T>
~~~

and shared ownership with:

~~~cpp
std::shared_ptr<T>
~~~

A raw pointer then usually means:

> non-owning observer, nullable unless the API says otherwise.

This convention is not a language rule, but it dramatically improves readability.

## 12.2 `unique_ptr` is not "a pointer with automatic delete"

It is an ownership object with:

* move-only semantics;
* configurable deleter;
* specialization for arrays;
* well-defined destruction behavior.

Its pointer type can even be customized through the deleter's `pointer` member.

That is another reason generic code should not assume every pointer-like owner internally uses a raw `T*`.

## 12.3 `shared_ptr` usually contains two conceptual pointers

A `shared_ptr<T>` participates in two identities:

* the **stored pointer** returned by `get()`;
* the **control block** that determines ownership.

The aliasing constructor makes the distinction explicit.

This allows a `shared_ptr` to keep one object alive while exposing a pointer to a subobject.

Therefore:

~~~cpp
p.get() == q.get()
~~~

does not answer every ownership question.

## 12.4 `weak_ptr` breaks ownership cycles

If:

~~~text
A owns B
B owns A
~~~

through `shared_ptr`, neither reference count reaches zero.

`weak_ptr` represents a non-owning link into a shared ownership graph.

The deeper lesson is not "always use smart pointers". It is:

> model ownership topology explicitly.

## 12.5 Raw pointer can be the correct type

Raw pointers remain appropriate for:

* non-owning optional observation;
* iterator-like traversal;
* C API boundaries;
* hardware/MMIO abstractions under a platform contract;
* allocator/runtime implementation;
* intrusive data structures;
* low-level serialization machinery;
* explicit pointer arithmetic.

"Modern C++" does not mean eliminating `T*`. It means not making `T*` carry ownership/lifetime semantics that a better type could express.

---

# 13. `new`/`delete`: allocation and object lifetime are related but not identical

A new-expression conceptually combines several operations:

* obtain storage;
* create an object in that storage;
* initialize the object;
* produce a pointer.

A delete-expression ends object lifetime and releases storage through the corresponding mechanism.

Understanding the combination matters when implementing allocators and containers.

## 13.1 Match `new` with `delete` and `new[]` with `delete[]`

~~~cpp
T* p = new T;
delete p;
~~~

and:

~~~cpp
T* p = new T[n];
delete[] p;
~~~

Mixing the forms is undefined behavior.

Do not rely on destructors being trivial to make mismatched deallocation acceptable.

## 13.2 `new T[0]` is well-formed

This surprises many programmers:

~~~cpp
T* p = new T[0];
delete[] p;
~~~

No array elements exist, but the allocation/deallocation expression is valid.

Do not require the returned pointer to be non-null as a portable semantic property. What matters is that the result is handled according to the `new[]`/`delete[]` contract.

## 13.3 `delete nullptr` is harmless

~~~cpp
T* p = nullptr;
delete p;
~~~

does nothing.

This is one reason cleanup code normally does not need:

~~~cpp
if (p) {
    delete p;
}
~~~

But in modern application code, explicit `delete` should already be uncommon outside ownership abstractions.

## 13.4 Allocation failure and construction failure are different stages

Ordinary throwing `new` reports allocation failure with `std::bad_alloc`.

If allocation succeeds but the constructor throws, the language invokes the matching deallocation function for the just-obtained storage.

This is one of the reasons a new-expression is safer than manually spelling allocation + placement construction without an RAII guard.

---

# 14. C++23 `std::out_ptr`: safer interop with pointer-to-pointer C APIs

A classic C API looks like:

~~~cpp
resource* r = nullptr;
int create_resource(resource** out);
~~~

C++ smart pointers do not naturally expose `T**` because allowing arbitrary code to overwrite their internal stored pointer would violate ownership invariants.

Historically, interop often looked like:

~~~cpp
T* raw = nullptr;

if (c_api(&raw) == success) {
    owner.reset(raw);
}
~~~

This is easy to get wrong when APIs have complicated success/failure rules.

C++23 adds adapters including:

~~~cpp
std::out_ptr
std::inout_ptr
~~~

for suitable smart-pointer/C-API interoperability.

For example:

~~~cpp
std::unique_ptr<resource, deleter> owner{nullptr, deleter{}};

c_api(std::out_ptr(owner));
~~~

The facility is valuable because it expresses a very specific ownership transition instead of exposing the smart pointer's internals.

This is a good example of modern C++ adding a **pointer facility** without adding a new pointer syntax.

---

# 15. References are aliases, but "references are just const pointers" is wrong

A reference is often implemented using an address internally, but the language semantics differ.

~~~cpp
int x = 1;
int& r = x;
~~~

`r` is an alias for `x`.

## 15.1 A reference is not reseatable

~~~cpp
int x = 1;
int y = 2;

int& r = x;
r = y;
~~~

The final line assigns `y`'s value to `x`.

It does not make `r` refer to `y`.

## 15.2 A reference is expected to refer to an object/function

C++ has no null-reference value analogous to a null pointer value.

Code such as:

~~~cpp
int& r = *static_cast<int*>(nullptr);
~~~

does not create a useful optional reference. It invokes invalid behavior in the attempt to form/use the reference.

If absence is meaningful, use a type that models absence.

Common choices include:

~~~cpp
T*
std::optional<std::reference_wrapper<T>>
~~~

depending on the API.

## 15.3 References are not objects in the ordinary type-system sense

You cannot declare:

~~~cpp
int& refs[10]; // array of references: ill-formed
int&* p;       // pointer to reference: ill-formed
~~~

Reference members and parameters may consume machine storage in an implementation, but `sizeof` does not expose a "reference object size".

For:

~~~cpp
int x;
int& r = x;
~~~

~~~cpp
sizeof(r)
~~~

means the size of the referred-to expression/type, not the size of some hidden implementation pointer.

---

# 16. Lifetime extension: memorize the exceptions, not just the slogan

A common rule is:

> binding a temporary to a const reference extends its lifetime.

That is useful but dangerously incomplete.

## 16.1 Local direct binding

This is the familiar good case:

~~~cpp
const std::string& s = std::string("hello");
~~~

The temporary string's lifetime is extended to the lifetime of `s`.

An rvalue reference can also extend a temporary's lifetime in appropriate direct-binding contexts:

~~~cpp
std::string&& s = std::string("hello");
~~~

## 16.2 Returning the reference does not pass the extension outward

This is wrong:

~~~cpp
const std::string& f()
{
    return std::string("temporary");
}
~~~

The temporary does not become immortal because the function returns a reference.

The caller receives a dangling reference.

## 16.3 Passing a temporary to a reference parameter does not extend it past the call

~~~cpp
void remember(const std::string& s);

remember(std::string("hello"));
~~~

The temporary survives through the full expression containing the call.

If `remember` stores `&s` for later use, that stored pointer/reference will dangle after the call expression completes.

## 16.4 A reference member can hide a lifetime problem

Types that store references should be reviewed as carefully as types that store raw pointers:

~~~cpp
struct View {
    const std::string& text;
};
~~~

The type says nothing about who owns the string.

Views are useful precisely because they are cheap and non-owning, but lifetime must be guaranteed externally.

---

# 17. Rvalue references: the declared type and expression category are different

This remains one of C++'s most important "looks wrong until internalized" rules:

~~~cpp
T&& r = ...;
~~~

The **type** of `r` is an rvalue-reference type.

But the expression:

~~~cpp
r
~~~

is an lvalue because it has a name.

Therefore:

~~~cpp
std::string&& r = make_string();

std::string a = r;            // copy
std::string b = std::move(r); // move construction selected if available
~~~

## 17.1 `std::move` does not move

`std::move` is essentially a cast that changes value category.

It says:

> this expression may be treated as an rvalue.

The selected constructor/operator performs the move.

Therefore:

~~~cpp
auto&& x = std::move(value);
~~~

has not by itself transferred any resource.

## 17.2 Moved-from does not mean destroyed

After a valid move operation, a standard-library object is generally still alive and valid, though often in an unspecified state unless a stronger postcondition is documented.

Its lifetime continues.

This is another place where lifetime and value state must not be conflated.

---

# 18. Reference collapsing and forwarding references

The collapse rules are:

~~~text
T&  &  -> T&
T&  && -> T&
T&& &  -> T&
T&& && -> T&&
~~~

The rule can be remembered as:

> if either layer is an lvalue reference, the result is an lvalue reference.

A parameter:

~~~cpp
template<class T>
void f(T&& x);
~~~

is a forwarding reference when `T` is deduced in the required context.

For an lvalue argument:

~~~cpp
int i;
f(i);
~~~

`T` deduces as `int&`, and:

~~~text
T&& -> int& && -> int&
~~~

For an rvalue:

~~~cpp
f(42);
~~~

`T` is `int` and the parameter is `int&&`.

## 18.1 `std::forward` preserves the caller's value category

Inside:

~~~cpp
template<class T>
void wrapper(T&& x)
{
    target(std::forward<T>(x));
}
~~~

`x` is a named lvalue expression.

`std::forward<T>` restores the category implied by deduction.

Using:

~~~cpp
target(std::move(x));
~~~

instead would incorrectly force lvalue arguments toward rvalue treatment.

## 18.2 `auto&&` can behave similarly

~~~cpp
auto&& a = lvalue; // lvalue reference after deduction
auto&& b = 42;     // rvalue reference
~~~

This pattern appears heavily in ranges and generic loops.

---

# 19. `decltype` and reference preservation

These two forms differ:

~~~cpp
int x = 0;

decltype(x)   // int
decltype((x)) // int&
~~~

The unparenthesized id-expression form has a special `decltype` rule that reports the declared type.

The parenthesized expression is analyzed by value category.

Since `(x)` is an lvalue:

~~~cpp
decltype((x))
~~~

is `int&`.

This distinction is essential in generic code.

## 19.1 `auto` return type can discard references

Suppose:

~~~cpp
int global;

auto f()
{
    return (global);
}
~~~

The deduced return type is `int`.

With:

~~~cpp
decltype(auto) f()
{
    return (global);
}
~~~

the return type is `int&`.

That may be exactly what you want—or a spectacular dangling-reference bug if the returned expression does not outlive the caller.

Reference preservation increases power and lifetime responsibility simultaneously.

---

# 20. C++20 ranges make some dangling-iterator bugs visible in the type system

Classic algorithms often return iterators.

If an algorithm is called on a temporary container, an iterator into that container would immediately dangle:

~~~cpp
auto it = /* find in temporary vector */;
~~~

C++20 ranges explicitly model this problem with:

~~~cpp
std::ranges::borrowed_range
std::ranges::dangling
~~~

For a non-borrowed temporary range:

~~~cpp
using R = decltype(
    std::ranges::find(
        std::vector<int>{1, 2, 3},
        2
    )
);

static_assert(
    std::is_same_v<R, std::ranges::dangling>
);
~~~

Instead of handing you a syntactically usable dangling iterator, the algorithm can return a marker type.

## 20.1 "Borrowed" does not mean ownership

A borrowed range means, roughly, that iterators obtained from the range can remain valid independently of the lifetime of the range object itself.

`std::span` is the classic example:

the span object can disappear while an iterator/pointer into the underlying array remains valid, provided the underlying array itself is still alive.

This separates:

* lifetime of the **range wrapper**;
* lifetime of the **elements/storage**.

That distinction is central to modern non-owning views.

---

# 21. C++23 `std::forward_like`: propagate cv/ref semantics from another type

Generic member access often wants:

> return this subobject with the same constness and value category as some enclosing object.

Before C++23, this frequently required verbose combinations of `std::forward`, casts, and type traits.

C++23 provides:

~~~cpp
std::forward_like<Like>(value)
~~~

For example:

~~~cpp
int x = 42;

static_assert(std::same_as<
    decltype(std::forward_like<const int&>(x)),
    const int&
>);

static_assert(std::same_as<
    decltype(std::forward_like<int&&>(x)),
    int&&
>);
~~~

It is especially useful in generic accessors and code related to explicit object parameters.

The facility does not move anything by itself, just as `std::move` does not move anything by itself. It produces an expression with corresponding cv/ref characteristics.

---

# 22. C++23 reference-from-temporary traits detect dangerous generic bindings

C++23 adds:

~~~cpp
std::reference_constructs_from_temporary
std::reference_converts_from_temporary
~~~

These traits are aimed at generic library code that needs to detect whether a reference initialization/conversion would bind to a temporary in a dangerous way.

For example, a wrapper that stores a reference can reject a constructor that would silently create a dangling member.

This is significant because lifetime safety cannot always be inferred from syntax inside a generic abstraction.

The language/library is gradually gaining vocabulary for expressing those hazards directly.

---

# 23. C++23 explicit object parameters ("deducing this") reduce cv/ref overload duplication

Traditional member APIs often need four overloads:

~~~cpp
T& value() &;
const T& value() const &;
T&& value() &&;
const T&& value() const &&;
~~~

C++23 explicit object parameters can make the object parameter itself visible to deduction.

That can reduce repeated overloads and pair naturally with `std::forward_like`.

This is relevant to a pointer/reference chapter because much of advanced C++ reference machinery exists to preserve:

* constness;
* lvalue/rvalue category;
* subobject lifetime relationships.

The feature is not "pointer syntax", but it directly affects how modern APIs return references and proxy objects.

---

# 24. Proxy references: not every `operator*` or `operator[]` returns `T&`

`std::vector<bool>` is the famous example:

~~~cpp
std::vector<bool> v(1);

auto x = v[0];
~~~

`x` is normally a proxy object, not a `bool&`.

The proxy represents access to one packed bit.

This general pattern appears in:

* bit containers;
* SIMD abstractions;
* database columns;
* compressed containers;
* transform/zip views;
* GPU memory abstractions.

Generic code should not assume:

~~~cpp
decltype(*it) == T&
~~~

Modern iterator concepts are designed with proxy references in mind.

This is another reason "iterator == pointer" is too narrow a model.

---

# 25. `std::reference_wrapper`: reference semantics in an object type

A true reference cannot be:

* default-constructed;
* reseated;
* stored directly as a container element type.

`std::reference_wrapper<T>` is an object that holds reference-like semantics:

~~~cpp
int x = 1;
int y = 2;

std::reference_wrapper<int> r = x;
r.get() = 10;

r = y;       // wrapper now refers to y
r.get() = 20;
~~~

This is useful for:

* containers of references;
* callbacks;
* algorithms;
* optional reference-like relationships.

It also makes a useful design distinction:

> language reference and reseatable reference-like handle are different abstractions.

---

# 26. `std::addressof`: even unary `&` can lie

C++ allows overloading unary `operator&` for class types.

Therefore:

~~~cpp
&object
~~~

does not necessarily invoke the built-in address-of operator.

Generic library code that needs the actual object address uses:

~~~cpp
std::addressof(object)
~~~

This looks obscure until you implement:

* allocators;
* containers;
* generic placement construction;
* serialization frameworks;
* intrusive data structures.

C++20 made `std::addressof` usable in more constant-evaluation contexts, but its core purpose predates C++20.

The lesson is broader:

> operator-looking syntax can invoke user code; library primitives sometimes exist specifically to bypass that customization.

---

# 27. Pointer invalidation: containers are a major source of dangling pointers

Suppose:

~~~cpp
std::vector<int> v{1, 2, 3};

int* p = &v[0];

v.push_back(4);
~~~

If `push_back` reallocates, `p` dangles.

The numeric address may even later be reused, making debugging deceptive.

## 27.1 `reserve` changes the invalidation story, not ownership

~~~cpp
v.reserve(1000);
~~~

can prevent reallocation while size remains within capacity.

It does not make arbitrary stored pointers permanently valid.

Operations such as destruction, assignment, swaps, erases, or later capacity growth still matter.

## 27.2 Different containers have different invalidation rules

For performance-sensitive C++, iterator/reference/pointer invalidation rules are part of the container's contract.

Do not generalize `vector` behavior to:

* `deque`;
* `list`;
* `map`;
* `unordered_map`;
* flat containers;
* custom small-vector types.

When storing observers into a container, invalidation semantics are as important as algorithmic complexity.

## 27.3 Small-string and small-vector optimization complicate address stability

A type can move from inline storage to heap storage as it grows.

Therefore:

~~~cpp
const char* p = s.data();
s += more_text;
~~~

may invalidate `p`.

Never infer address stability merely because an object itself has not moved.

---

# 28. Pointer stability and object identity are not the same as numeric-address stability

Allocators reuse addresses.

Consider:

~~~cpp
T* old = new T;
delete old;

T* fresh = new T;
~~~

`fresh` may have the same printed numeric address as `old`.

That does not mean the old object has returned.

It is a new lifetime and a new object.

This matters in:

* lock-free algorithms;
* ABA problems;
* caches keyed by raw addresses;
* debugging;
* sanitizers;
* object registries.

A numeric address can be reused while logical object identity changes.

---

# 29. The ABA problem: pointer equality can be true and still mislead a concurrent algorithm

In a lock-free stack, one thread may observe pointer value `A`.

Another thread can:

1. remove object A;
2. remove another object;
3. allocate/reinsert an object at address A.

The first thread sees the same pointer bits and concludes "nothing changed".

That is the ABA problem.

Solutions can involve:

* tagged/versioned pointers;
* hazard pointers;
* epoch reclamation;
* reference counting;
* specialized reclamation schemes.

This demonstrates a deep principle:

> pointer equality tells you about pointer values, not necessarily temporal object identity.

C++26 adds standardized hazard-pointer and RCU-related facilities, but even in C++20/23 the underlying issue is fundamental to pointer lifetime reasoning.

---

# 30. Atomic pointers support pointer operations, but do not solve reclamation

C++ provides:

~~~cpp
std::atomic<T*>
~~~

and atomic specializations for smart pointers in modern standards.

An atomic raw pointer can safely coordinate the pointer value itself according to the chosen memory order.

That does not automatically guarantee the pointed-to object's lifetime.

For example:

~~~text
thread A loads p
thread B removes and deletes *p
thread A dereferences p
~~~

The atomic load can be perfectly race-free while the dereference is a use-after-free.

Lock-free pointer algorithms therefore need both:

* atomic synchronization;
* safe memory reclamation.

Confusing the two is a classic concurrency bug.

---

# 31. `volatile T*` is not a synchronization primitive

Low-level developers often encounter:

~~~cpp
volatile std::uint32_t* register_ptr;
~~~

`volatile` tells the implementation that accesses are observable in ways ordinary memory accesses may not be.

It does **not** provide:

* atomicity;
* mutual exclusion;
* inter-thread happens-before;
* general CPU memory barriers.

For inter-thread synchronization, use the C++ atomic/mutex model.

For MMIO, follow the platform/compiler's hardware-access contract; `volatile` may be part of that contract, but C++ `volatile` alone does not specify a device-memory model.

---

# 32. Pointer provenance and optimizer reasoning

The exact formal treatment of pointer provenance has evolved through standardization work, but practical C++ already requires a stronger mental model than:

> same integer address means interchangeable pointer.

Optimizers reason about:

* which allocation/object a pointer came from;
* object lifetime;
* type-based aliasing;
* restrict-like implementation assumptions;
* escape analysis.

Round-tripping pointers through integers, fabricating pointers from arithmetic, and reusing stale pointers after lifetime changes can interact badly with those assumptions.

For portable application code:

* keep pointers as pointers;
* use array/iterator arithmetic for traversal;
* use standard lifetime APIs;
* use integers for pointers only when the platform/API specifically requires it.

---

# 33. String literals are arrays, not heap strings

This:

~~~cpp
"hello"
~~~

has array type and static storage duration.

In modern C++ the elements are not modifiable through the literal.

Use:

~~~cpp
const char* p = "hello";
~~~

not:

~~~cpp
char* p = "hello"; // ill-formed in modern C++
~~~

## 33.1 Literal concatenation happens at translation time

~~~cpp
const char* text =
    "one "
    "two "
    "three";
~~~

forms one literal sequence.

No runtime concatenation is required.

## 33.2 Raw strings change lexical escaping, not ownership/lifetime

~~~cpp
R"(C:\path\file)"
~~~

is still a string literal with static storage duration.

Raw-string syntax changes how characters are written in source.

It does not create a dynamic `std::string`.

## 33.3 C++20 changed the type of UTF-8 string literals

Since C++20:

~~~cpp
u8"text"
~~~

is an array of `const char8_t`.

It is not a `const char[]`.

This intentionally distinguishes UTF-8 code units from ordinary narrow characters.

Code written for C++17 that expects:

~~~cpp
const char* p = u8"text";
~~~

needs adjustment when compiled as C++20.

A typical non-owning view is:

~~~cpp
std::u8string_view text = u8"text";
~~~

---

# 34. Function pointers: callbacks, ABI boundaries, and conversions

A function pointer can be called:

~~~cpp
using Callback = int (*)(int);

int square(int x) {
    return x * x;
}

Callback cb = &square;
int y = cb(4);
~~~

The explicit `&` is optional in many such contexts:

~~~cpp
Callback cb = square;
~~~

## 34.1 Calling convention can be part of the ABI

On platforms with multiple calling conventions, function-pointer compatibility may involve more than parameter and return types at the machine level.

FFI and plugin boundaries should use explicitly documented ABI declarations.

## 34.2 Capturing lambdas do not convert to plain function pointers

A non-capturing lambda can convert to a compatible function pointer:

~~~cpp
auto lambda = [](int x) { return x + 1; };

int (*fp)(int) = lambda;
~~~

A capturing lambda needs state:

~~~cpp
int delta = 1;
auto lambda = [delta](int x) {
    return x + delta;
};
~~~

and therefore cannot generally be represented by a plain function pointer.

Use an appropriate callable abstraction or C-style context pointer pattern.

---

# 35. Pointer-to-member: syntax reveals that it is not an ordinary pointer

Given:

~~~cpp
struct S {
    int value;
    void f();
};
~~~

data-member pointer:

~~~cpp
int S::* pm = &S::value;
~~~

member-function pointer:

~~~cpp
void (S::*pf)() = &S::f;
~~~

Application uses special operators:

~~~cpp
S s;
S* p = &s;

s.*pm = 10;
p->*pm = 20;

(s.*pf)();
(p->*pf)();
~~~

The need for `.*` and `->*` is a clue: these values describe a member relative to an object, not simply a free-standing address.

## 35.1 Multiple inheritance can require adjustment

If a member originates in a base subobject, invoking it through a derived object may require adjusting the object pointer.

ABIs can encode such information in member-pointer values.

Therefore printing a pointer-to-member as though it were a `void*` is not meaningful portable debugging.

---

# 36. C++20 `std::bit_cast` versus pointer reinterpretation

Suppose you want the bit representation of a floating value.

Bad model:

~~~cpp
auto bits =
    *reinterpret_cast<std::uint32_t*>(&f);
~~~

Modern model:

~~~cpp
auto bits =
    std::bit_cast<std::uint32_t>(f);
~~~

The difference is not stylistic.

The first tries to access the existing `float` object through an incompatible `uint32_t` lvalue.

The second performs a defined representation conversion under `bit_cast`'s constraints.

This pattern applies throughout systems C++:

> when the operation is "copy representation", use a representation-copy facility; do not fake it as pointer aliasing.

---

# 37. Bounds safety: raw pointers do not carry an extent

Given only:

~~~cpp
void f(int* p);
~~~

the function cannot know from the pointer value alone:

* whether `p` points to one element or many;
* how many;
* whether it is null;
* whether the storage is writable beyond the first object;
* who owns the storage.

This is why modern APIs often prefer:

~~~cpp
std::span<int>
std::string_view
std::span<const std::byte>
~~~

for non-owning ranges.

## 37.1 A pointer plus a size can still be the correct ABI

At C boundaries or stable binary APIs:

~~~cpp
void f(const std::byte* data, std::size_t size);
~~~

may be exactly right.

Inside C++, wrap it quickly:

~~~cpp
std::span<const std::byte> bytes{data, size};
~~~

This creates a clear boundary between raw ABI representation and richer internal semantics.

---

# 38. `std::string_view` has the same lifetime class of hazard as `std::span`

A `string_view` is essentially a non-owning character range.

This is dangerous:

~~~cpp
std::string_view make_view()
{
    std::string s = "temporary";
    return s;
}
~~~

The returned view dangles.

The type is cheap precisely because it does not own.

The same design review question should be asked for:

* `T*`;
* `T&`;
* `std::span<T>`;
* `std::string_view`;
* iterators;
* subranges;
* reference wrappers.

> What object guarantees the lifetime of the referred-to storage?

---

# 39. Pointers and exceptions: RAII is the lifetime firewall

Consider:

~~~cpp
T* p = new T;

do_something_that_throws();

delete p;
~~~

If the middle call throws, cleanup is skipped.

With:

~~~cpp
auto p = std::make_unique<T>();

do_something_that_throws();
~~~

stack unwinding destroys the `unique_ptr`.

This is why RAII is more fundamental than "smart pointers are convenient".

RAII couples resource lifetime to an object lifetime that the language already unwinds correctly.

The same principle applies to:

* files;
* sockets;
* mutexes;
* mapped memory;
* database transactions;
* GPU resources.

---

# 40. C++ sequencing rules: avoid the obsolete phrase "evaluation order is undefined"

Older explanations often say:

> order of subexpression evaluation is undefined.

That wording mixes several distinct concepts.

Modern C++ distinguishes:

* **sequenced before**;
* **indeterminately sequenced**;
* **unsequenced**;
* **unspecified order**;
* **undefined behavior** caused by certain unsequenced conflicting side effects.

C++17 strengthened sequencing rules for several expression forms.

For pointer code, the practical rule remains:

> do not pack multiple dependent pointer increments/mutations into one clever expression unless the sequencing guarantee is explicit.

Prefer:

~~~cpp
auto old = p;
++p;
use(old, p);
~~~

over expressions whose correctness depends on remembering a subtle sequencing rule.

---

# 41. `const` qualification on pointers is two-dimensional

These are different:

~~~cpp
int* p;                // mutable pointer to mutable int
const int* p;          // mutable pointer to const int
int* const p = ...;    // const pointer to mutable int
const int* const p = ...; // const pointer to const int
~~~

A useful reading strategy is from the identifier outward.

## 41.1 Pointer conversion can add pointee constness

~~~cpp
int* p = ...;
const int* cp = p;
~~~

The reverse is not implicit:

~~~cpp
const int* cp = ...;
int* p = cp; // error
~~~

`const_cast` can remove qualification syntactically, but writing through the result is only valid if the underlying object is actually non-const.

## 41.2 Deep constness is not automatic

Consider:

~~~cpp
T* const*
const T**
~~~

Qualification conversion rules become intentionally restrictive because naïve covariance would allow writing a pointer to const data into a location later accessed as pointer-to-mutable.

This is the classic reason:

~~~cpp
Derived**
~~~

does not safely convert the way a programmer might expect from:

~~~cpp
Derived* -> Base*
~~~

Pointer indirection introduces mutation channels.

---

# 42. References and constness are not transitive ownership guarantees

A:

~~~cpp
const std::unique_ptr<T>&
~~~

prevents modification of the `unique_ptr` through that reference, but the semantics of access to `T` depend on the smart pointer's interface.

Similarly:

~~~cpp
T* const
~~~

makes the pointer itself non-reseatable, not the pointee const.

C++ constness is applied to types/expressions, not as a universal graph-wide "everything reachable is immutable" property.

This matters in API design and concurrency reasoning.

---

# 43. `memcpy`/`memcmp` and object representation pitfalls

Because objects occupy bytes, programmers sometimes write:

~~~cpp
bool equal(const S& a, const S& b)
{
    return std::memcmp(&a, &b, sizeof(S)) == 0;
}
~~~

This is not generally equivalent to memberwise equality.

Reasons include:

* padding bytes;
* multiple representations of equivalent values;
* pointer representation;
* floating signed zero;
* NaN representations;
* object invariants.

C++20 adds useful traits such as:

~~~cpp
std::has_unique_object_representations_v<T>
~~~

but even that trait does not magically make bytewise comparison the correct semantic equality for every domain.

Separate:

* object representation equality;
* C++ value equality;
* domain equality.

---

# 44. Alignment does not imply aliasing legality

Suppose storage is correctly aligned for `double`:

~~~cpp
alignas(double) std::byte storage[sizeof(double)];
~~~

That answers an alignment question.

It does not, by itself, answer:

* whether a `double` object is alive there;
* whether the bytes represent a valid `double`;
* whether a particular cast/access is permitted.

Low-level bugs often come from satisfying one property and assuming the others follow.

Think of the checklist:

~~~text
storage?
size?
alignment?
lifetime?
type-access permission?
bounds?
ownership/lifetime duration?
~~~

---

# 45. C++ "byte" is not guaranteed to be 8 bits

Pointer arithmetic on:

~~~cpp
char*
unsigned char*
std::byte*
~~~

advances by one C++ byte.

The number of bits in that byte is:

~~~cpp
CHAR_BIT
~~~

from `<climits>`.

C++ requires at least 8 bits, but historically and in specialist DSP/embedded environments a byte can contain more than 8 bits.

Therefore:

~~~cpp
sizeof(T)
~~~

is measured in C++ bytes, not necessarily octets.

Protocols defined in 8-bit octets need explicit-width representation assumptions.

This is particularly important when educational material says "pointer advances N bytes" and silently equates byte with octet.

---

# 46. `std::byte` is intentionally not a character or integer type

C++17 introduced:

~~~cpp
std::byte
~~~

to represent raw byte data without pretending the data is text or an ordinary integer.

It supports bitwise operations but not ordinary arithmetic.

That makes:

~~~cpp
std::span<std::byte>
~~~

a strong vocabulary type for raw storage/binary buffers.

Use:

~~~cpp
std::to_integer<unsigned>(b)
~~~

when you intentionally need a numeric value.

The type distinction helps prevent accidental arithmetic on raw representation.

---

# 47. Smart-pointer size is not universally one machine word

A common performance claim is:

> `unique_ptr` costs exactly one pointer.

Often true for an empty stateless deleter because implementations exploit empty-base/no-unique-address-like storage optimization.

Not a language guarantee.

A stateful deleter can increase the size:

~~~cpp
struct Deleter {
    int allocator_id;
    void operator()(T*) const;
};

std::unique_ptr<T, Deleter>
~~~

Likewise, `shared_ptr` commonly contains two machine pointers, but its exact representation is implementation-defined.

Measure when layout is part of a performance contract.

---

# 48. `[[no_unique_address]]` and pointer-adjacent zero-size state (C++20)

C++20 introduced:

~~~cpp
[[no_unique_address]]
~~~

which allows certain potentially-overlapping storage for empty member objects.

This is relevant to pointer wrappers, iterators, allocators, and deleters because policy objects can often be stored without increasing object size.

Example:

~~~cpp
template<class T, class Policy>
struct pointer_wrapper {
    T* p;
    [[no_unique_address]] Policy policy;
};
~~~

For an empty `Policy`, the wrapper may remain the size of one pointer.

Do not depend on an exact layout without an ABI contract, but understand why modern library wrappers can carry type-level policy at little or no runtime storage cost.

---

# 49. Incomplete types and pointers

Pointers are useful for breaking compilation dependencies because you can declare a pointer to an incomplete type:

~~~cpp
class Impl;

Impl* p;
~~~

You cannot generally:

~~~cpp
Impl object;
sizeof(Impl);
~~~

until `Impl` is complete.

This is the basis of the PImpl idiom.

## 49.1 `unique_ptr<Incomplete>` has a completeness subtlety

A class can contain:

~~~cpp
std::unique_ptr<Impl> impl_;
~~~

while `Impl` is incomplete in the header.

But destruction must eventually occur where the deleter can correctly delete a complete `Impl`.

This is why PImpl classes commonly define their destructor out-of-line in the implementation file after `Impl` is complete.

Pointer declarations are cheap; destruction semantics still need the complete type at the right point.

---

# 50. Recursive data structures require indirection because objects cannot contain themselves by value

This is impossible:

~~~cpp
struct Node {
    Node next;
};
~~~

The object would require infinite size.

This is possible:

~~~cpp
struct Node {
    Node* next;
};
~~~

or, for ownership:

~~~cpp
struct Node {
    std::unique_ptr<Node> next;
};
~~~

The pointer has finite size independent of the complete size of the eventual node object.

This is one of the fundamental structural reasons pointers exist beyond "dynamic allocation".

---

# 51. API design: pointer, reference, span, view, smart pointer, or value?

A useful modern vocabulary is:

| Type | Typical semantic message |
|---|---|
| `T` | own/copy/move a value |
| `T&` | required mutable borrowed object |
| `const T&` | required read-only borrowed object |
| `T*` | optional/reseatable borrowed object or low-level pointer |
| `std::span<T>` | borrowed contiguous sequence |
| `std::string_view` | borrowed character sequence |
| `std::unique_ptr<T>` | exclusive dynamic ownership |
| `std::shared_ptr<T>` | shared lifetime ownership |
| `std::weak_ptr<T>` | non-owning link into shared ownership |
| iterator/subrange | position/range in a traversal abstraction |

These are conventions, not absolute laws.

For example, a C ABI may legitimately use `T*` for required output parameters.

The important thing is that the interface's lifetime and ownership semantics be explicit and consistent.

---

# 52. Dangerous pointer idioms worth recognizing in review

Watch for:

~~~cpp
reinterpret_cast<T*>(integer)
~~~

unless platform code explicitly requires it.

Watch for:

~~~cpp
*reinterpret_cast<U*>(&object)
~~~

as probable aliasing/type-punning trouble.

Watch for:

~~~cpp
return local.data();
return std::string_view(local);
return std::span(local);
~~~

as likely dangling results.

Watch for:

~~~cpp
container.push_back(...);
// use old pointer/iterator/reference
~~~

when the operation may invalidate observers.

Watch for:

~~~cpp
delete raw;
~~~

when ownership is not locally obvious.

Watch for:

~~~cpp
std::move(x);
// assume x is destroyed
~~~

which confuses value state with lifetime.

Watch for:

~~~cpp
std::assume_aligned<N>(p)
~~~

without a proof of the alignment invariant.

Watch for:

~~~cpp
memset(&object, 0, sizeof object);
~~~

on non-trivial or representation-sensitive C++ types.

Watch for:

~~~cpp
memcmp(&a, &b, sizeof a)
~~~

as a substitute for semantic equality.

Watch for stored:

~~~cpp
const T&
T*
span<T>
string_view
iterator
~~~

whose owner is unclear.

---

# 53. C++20/23 pointer-related modernization map

For older low-level C++ code, these substitutions are often useful:

| Older idiom | Modern facility / question |
|---|---|
| pointer + length parameters | `std::span` (C++20) |
| assume iterator is raw pointer | `std::contiguous_iterator` + `std::to_address` (C++20) |
| pointer punning for representation | `std::bit_cast` (C++20) |
| manually view bytes | `std::as_bytes` / `std::as_writable_bytes` |
| placement-new helper code | `std::construct_at` (C++20) |
| optimizer-specific alignment cast | `std::assume_aligned` (C++20) |
| temporary-range iterator accidentally returned | ranges `borrowed_range` / `dangling` (C++20) |
| UTF-8 stored as ordinary `char*` by assumption | `char8_t` / `u8string_view` (C++20) |
| manually byteswap integers | `std::byteswap` (C++23) |
| C `T**` output + smart pointer reset choreography | `std::out_ptr` / `std::inout_ptr` (C++23) |
| awkward implicit-lifetime storage transition | `std::start_lifetime_as` (C++23, where applicable) |
| hand-built cv/ref propagation | `std::forward_like` (C++23) |
| generic reference may bind to temporary | reference-from-temporary traits (C++23) |
| duplicated cv/ref member overloads | explicit object parameters / deducing `this` (C++23) |

Modernization should not be mechanical.

A `span` cannot fix an owner that dies too early.

A `bit_cast` cannot make invalid bytes a valid object value.

`start_lifetime_as` cannot repair insufficient alignment.

`out_ptr` cannot repair a badly specified C ownership contract.

The newer facilities make the intended low-level operation more explicit; they do not remove its preconditions.

---

# 54. Standards timeline

## C++98/03

Core facilities included:

* object pointers;
* function pointers;
* pointers-to-member;
* lvalue references;
* `new`/`delete`;
* placement new;
* `void*` object-pointer conversion;
* array/pointer model;
* classic iterator abstractions.

Most lifetime and ownership conventions were expressed informally.

## C++11

Major changes relevant to this chapter:

* `nullptr` and `std::nullptr_t`;
* rvalue references;
* move semantics;
* forwarding references;
* `std::move` / `std::forward`;
* `unique_ptr` / `shared_ptr` / `weak_ptr`;
* `alignas` / `alignof`;
* stronger type traits;
* atomics including atomic pointers.

## C++14

Relevant evolution included:

* `decltype(auto)`;
* more flexible return-type deduction;
* generic lambdas;
* `make_unique`.

## C++17

Important low-level facilities:

* `std::byte`;
* `std::launder`;
* over-aligned dynamic allocation support;
* `std::void_t` and broader generic machinery;
* structured bindings;
* guaranteed copy elision in specified cases;
* `std::string_view`;
* `std::optional` and `std::variant` as alternatives to pointer/tagged-union idioms;
* `std::has_unique_object_representations`.

## C++20

This is a particularly important pointer/lifetime release:

* `std::span`;
* `std::bit_cast`;
* `std::endian`;
* `std::to_address`;
* `std::contiguous_iterator`;
* ranges and `std::ranges::dangling`;
* `std::construct_at`;
* `std::assume_aligned`;
* `char8_t` for UTF-8 code units;
* `[[no_unique_address]]`;
* atomic `shared_ptr`/`weak_ptr` specializations;
* concepts that can express pointer/iterator constraints directly.

## C++23

Notable additions include:

* `std::byteswap`;
* `std::out_ptr` / `std::inout_ptr`;
* `std::start_lifetime_as` and array variant;
* `std::forward_like`;
* `std::reference_constructs_from_temporary`;
* `std::reference_converts_from_temporary`;
* explicit object parameters ("deducing this");
* continued ranges/view improvements.

---

# 55. Review checklist for production pointer code

When reviewing low-level C++, ask:

| Question | Why it matters |
|---|---|
| Who owns the pointed-to object? | Raw pointer syntax does not encode ownership. |
| Can the pointer be null? | Optionality should be part of the API contract. |
| Can the owner move/reallocate/destroy the object? | Observers can silently dangle. |
| Is pointer arithmetic confined to one array object? | Arbitrary address arithmetic is not the C++ model. |
| Is one-past only compared/subtracted, never dereferenced? | One-past is not an element. |
| Are unrelated pointers being ordered with raw `<`? | Use the library total-order facility when a total order is required. |
| Is a function pointer being stored in `void*`? | Object and function pointers are distinct categories. |
| Is a member pointer being treated as an address? | It may encode adjustment/dispatch information. |
| Is `reinterpret_cast` followed by dereference? | The cast may not permit access through the new type. |
| Is representation transfer really wanted? | Use `bit_cast`, byte views, or `memcpy`. |
| Is the storage correctly aligned? | Misalignment is a separate failure mode. |
| Is an object of type `T` actually alive there? | Storage does not automatically imply lifetime. |
| Did placement construction replace an object? | Old pointers/references may need lifetime analysis. |
| Is `std::launder` being used as a generic fix? | It solves only specific object-identity cases. |
| Is `std::assume_aligned` backed by a real invariant? | It is a promise, not a check. |
| Does a `span`/`string_view` outlive its owner? | Views are non-owning. |
| Does a ranges result come from a temporary range? | C++20 borrowed-range rules matter. |
| Is an integerized pointer being used as durable identity? | Address values are process/lifetime dependent. |
| Is an atomic pointer assumed to keep the object alive? | Synchronization and reclamation are separate. |
| Is `volatile` being used for thread synchronization? | It is not a replacement for atomics. |
| Is bytewise equality used for objects with padding? | Representation equality may differ from value equality. |
| Does a UTF-8 API assume `u8""` is `const char*`? | Since C++20 it uses `char8_t`. |
| Does C interop expose `T**` into a smart pointer manually? | C++23 `out_ptr` may express the ownership transition better. |

---

# 56. Final mental model

The most useful advanced model is:

> A pointer is a typed value that participates in C++'s object, lifetime, bounds, aliasing, and memory models. Its machine address is only part of its meaning.

From that model, many otherwise obscure rules become coherent:

* pointer arithmetic is constrained by array objects;
* one-past pointers can exist without being dereferenceable;
* unrelated-pointer ordering needs special library support when a total order is required;
* function pointers and member pointers are not generic object addresses;
* integer conversion does not turn pointers into portable numeric identities;
* aligned storage does not automatically contain a live object;
* `reinterpret_cast` does not override aliasing/lifetime rules;
* an address can remain while an object lifetime has ended;
* a raw pointer can observe an object without owning it;
* `span` and `string_view` improve bounds/interface semantics without solving lifetime;
* ranges explicitly model some dangling results;
* smart pointers model ownership, not merely pointer syntax;
* atomics synchronize pointer values but do not by themselves reclaim objects safely;
* modern C++20/23 facilities increasingly expose low-level intent directly instead of relying on folklore and bit tricks.

For systems C++, pointer expertise is therefore less about remembering where to place `*` and more about answering four questions correctly:

1. **What object is this value allowed to designate?**
2. **Is that object alive for the entire use?**
3. **Is this access type/alignment/bounds-correct?**
4. **Who guarantees the object's lifetime and eventual cleanup?**

If those four answers are explicit, most pointer code becomes straightforward. If they are not, even code that prints the "right address" can already be wrong.

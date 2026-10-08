# 06. Arrays in Modern C++: Shape, Decay, Views, and Ownership

C++ arrays look deceptively primitive. Their syntax predates C++, they interoperate directly with C, and a raw array often seems to turn into a pointer as soon as it is used. That encourages a simplified mental model in which `T[N]` is “basically a `T*` with some storage behind it.”

That model is wrong in exactly the places where experienced C++ programmers get into trouble. A raw array is a distinct object type whose bound is part of the type. Array-to-pointer conversion is a context-dependent conversion that discards shape information. Multidimensional arrays are nested array objects, not pointer graphs. Dynamic arrays add a separate ownership problem. Modern C++ does not remove raw arrays; instead, it gives us vocabulary types—`std::array`, `std::span`, and C++23 `std::mdspan`—that preserve the useful parts of the array model without forcing every API to speak in pointer arithmetic.

For experienced C++ developers, the more important questions are:

* what exactly survives when an array crosses a function or template boundary;
* when a pointer still represents an array element and when shape information has already been lost;
* why contiguous multidimensional storage is not equivalent to `T**` or even to one flat `T*` range;
* which ownership and lifetime guarantees are encoded by `T[N]`, `std::array`, `std::span`, and `std::mdspan`;
* and which old array idioms become unnecessary or actively misleading in C++20/23.

This chapter follows the examples in this directory:

1. [`01_arrays_decay_and_overloads/arrays_decay_and_overloads.cpp`](01_arrays_decay_and_overloads/arrays_decay_and_overloads.cpp) — array-to-pointer conversion, `sizeof`, pointer-to-array types, and adjusted function parameters.
2. [`02_arrays_templates_and_traits/arrays_templates_and_traits.cpp`](02_arrays_templates_and_traits/arrays_templates_and_traits.cpp) — preserving array bounds in templates and inspecting array shape with type traits.
3. [`03_arrays_initialization_and_literals/arrays_initialization_and_literals.cpp`](03_arrays_initialization_and_literals/arrays_initialization_and_literals.cpp) — aggregate initialization, zero-filling, narrowing, string literal array types, and C++20 `char8_t`.
4. [`04_arrays_multidim_and_pointers/arrays_multidim_and_pointers.cpp`](04_arrays_multidim_and_pointers/arrays_multidim_and_pointers.cpp) — arrays of arrays, row types, pointer-to-array arithmetic, and C++23 `std::mdspan`.
5. [`05_arrays_memory_and_pitfalls/arrays_memory_and_pitfalls.cpp`](05_arrays_memory_and_pitfalls/arrays_memory_and_pitfalls.cpp) — `new[]`/`delete[]`, hidden array aliases, smart-pointer ownership, and polymorphic-array hazards.
6. [`06_modern_arrays/modern_arrays.cpp`](06_modern_arrays/modern_arrays.cpp) — `std::array`, `std::to_array`, `std::span`, ranges access, and byte views.

The repository currently builds as C++20. Examples that need C++23 are feature-tested so the baseline stays buildable.

---

## 1. A raw array is a type with shape, not a pointer with hidden length

The most important fact about C++ arrays is visible directly in the type system:

~~~cpp
int a[5];
~~~

`a` has type `int[5]`. The bound `5` is part of the type. A different bound gives a different type:

~~~cpp
static_assert(!std::is_same_v<int[5], int[6]>);
~~~

Nothing about this declaration creates a pointer object. The pointer appears only after a language conversion is applied in a context that requests it.

### Array-to-pointer conversion loses one level of shape

In most ordinary expressions, an lvalue or rvalue of array type is converted to a pointer to its first element:

~~~cpp
int a[5] = {1, 2, 3, 4, 5};

int* p = a;
assert(p == &a[0]);
~~~

This is called **array-to-pointer conversion**, traditionally “array decay.”

The conversion changes the type from:

~~~text
int[5]
~~~

to:

~~~text
int*
~~~

and therefore loses the bound. Once only `int*` remains, the language cannot recover whether the pointer originated from one element, five elements, five thousand elements, or no valid array at all.

This is why pointer-plus-length interfaces exist, and why C++20 `std::span` is such an important replacement.

### Decay is contextual rather than automatic

Several contexts deliberately preserve the array object.

With `sizeof`:

~~~cpp
int a[5];

static_assert(sizeof(a) == 5 * sizeof(int));
~~~

With address-of:

~~~cpp
auto p = &a;

static_assert(
    std::is_same_v<decltype(p), int (*)[5]>
);
~~~

With an array reference:

~~~cpp
int (&r)[5] = a;
~~~

With `decltype` on an unparenthesized id-expression:

~~~cpp
static_assert(
    std::is_same_v<decltype(a), int[5]>
);
~~~

These are not arbitrary exceptions. Each of these operations needs to reason about the array object itself rather than merely the address of element zero.

### `a` and `&a` can contain the same address bits while having different semantics

For:

~~~cpp
int a[5];
~~~

the expressions:

~~~cpp
a      // after decay: int*
&a     // int (*)[5]
~~~

normally produce pointer values that print as the same address. Their types, arithmetic, and valid dereference operations are different.

~~~cpp
int* p = a;
int (*pa)[5] = &a;

++p;   // next int
++pa;  // one-past the entire int[5] object
~~~

The step sizes differ because pointer arithmetic is expressed in units of the pointed-to type:

~~~text
int*        -> sizeof(int)
int (*)[5]  -> sizeof(int[5])
~~~

The address is not the whole meaning of a pointer. Its pointed-to type determines the array object within which arithmetic is defined.

### `sizeof` reveals whether shape has already been lost

At the declaration site:

~~~cpp
int a[10];

static_assert(sizeof(a) == 10 * sizeof(int));
~~~

Inside this function:

~~~cpp
void f(int a[])
{
    // a is not an array parameter object here
}
~~~

`a` is already a pointer.

That difference is not a quirk of `sizeof`. `sizeof` merely exposes that two different types are involved.

### Array syntax in a function parameter is adjusted to pointer syntax

These declarations declare the same parameter type:

~~~cpp
void f(int a[]);
void f(int a[10]);
void f(int* a);
~~~

Inside the function, the parameter is `int*`.

The written bound does not become a runtime precondition, compiler-checked contract, or carried array extent.

This is one of the oldest C compatibility rules still present in modern C++.

The practical consequence is simple:

> Do not use `T parameter[N]` syntax to communicate a checked size requirement in C++.

Use a reference-to-array when the exact bound belongs to the type:

~~~cpp
template<std::size_t N>
void process(int (&a)[N]);
~~~

or use C++20 `std::span` when the function needs a non-owning contiguous sequence:

~~~cpp
void process(std::span<int> values);
~~~

### References preserve array identity because they do not create a new array

Raw arrays are not passed by value. A reference parameter can nevertheless bind to the original array object:

~~~cpp
template<class T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept
{
    return N;
}
~~~

The compiler deduces both:

* the element type `T`;
* the bound `N`.

No runtime metadata is added. The size is recovered because it never left the type system.

### Raw arrays are not assignable objects

This is ill-formed:

~~~cpp
int a[3] = {1, 2, 3};
int b[3] = {4, 5, 6};

// a = b; // error
~~~

Raw arrays can be initialized, but they do not have array assignment semantics.

That contrasts sharply with `std::array`:

~~~cpp
std::array<int, 3> a{1, 2, 3};
std::array<int, 3> b{4, 5, 6};

a = b; // valid
~~~

This difference is one reason `std::array` is normally the better value type when a fixed-size sequence should behave like an ordinary object.

---

## 2. Templates can preserve array shape that ordinary parameters discard

Array-aware templates expose a useful property of C++'s type system: the size of a bounded raw array is compile-time type information.

### `T(&)[N]` is the canonical shape-preserving parameter

Given:

~~~cpp
template<class T, std::size_t N>
void inspect(T (&array)[N]);
~~~

the call:

~~~cpp
const int values[4] = {1, 2, 3, 4};
inspect(values);
~~~

deduces:

~~~text
T = const int
N = 4
~~~

The function therefore knows both element cv-qualification and bound without any separate argument.

This style is useful when:

* the function genuinely requires a built-in array;
* the bound affects template logic;
* interoperability with old APIs matters;
* or a teaching/example context needs to expose the type directly.

For ordinary APIs, `std::span<T, N>` often expresses the same non-owning fixed extent more compositionally.

### `auto&` preserves arrays because reference deduction suppresses decay

~~~cpp
int values[4];

auto& r = values;

static_assert(
    std::is_same_v<decltype(r), int (&)[4]>
);
~~~

By contrast:

~~~cpp
auto p = values;
~~~

deduces a pointer because the initializer expression undergoes array-to-pointer conversion.

This is a recurring rule in modern generic C++:

> Value deduction tends to erase array shape; reference deduction can preserve it.

### `decltype` can observe the declared array type directly

For an unparenthesized id-expression:

~~~cpp
int values[4];

static_assert(
    std::is_same_v<decltype(values), int[4]>
);
~~~

Parenthesization changes the `decltype` rule:

~~~cpp
static_assert(
    std::is_same_v<decltype((values)), int (&)[4]>
);
~~~

The first asks for the declared type of the named entity.

The second asks for the type/value-category result of the expression. Since `values` is an lvalue expression, the result is an lvalue reference to the array.

### `std::rank` and `std::extent` expose nested array shape

For:

~~~cpp
using matrix = int[2][5];
~~~

C++11 type traits can observe:

~~~cpp
static_assert(std::rank_v<matrix> == 2);

static_assert(std::extent_v<matrix, 0> == 2);
static_assert(std::extent_v<matrix, 1> == 5);
~~~

These are compile-time properties of the type, not runtime metadata stored beside the elements.

The element type after removing one extent is still an array:

~~~cpp
using row = std::remove_extent_t<matrix>;

static_assert(std::is_same_v<row, int[5]>);
~~~

To reach the scalar element:

~~~cpp
using scalar = std::remove_all_extents_t<matrix>;

static_assert(std::is_same_v<scalar, int>);
~~~

### C++20 distinguishes bounded and unbounded array types directly

C++20 added:

~~~cpp
std::is_bounded_array_v<T>
std::is_unbounded_array_v<T>
~~~

These make a distinction that previously required custom trait machinery.

~~~cpp
static_assert(std::is_bounded_array_v<int[4]>);
static_assert(std::is_unbounded_array_v<int[]>);
~~~

An unknown-bound type such as `int[]` can appear in declarations like:

~~~cpp
extern int table[];
~~~

The type says “array of int, bound not known here.”

That is different from:

~~~cpp
int*
~~~

An unknown-bound array is still an array type. It simply lacks a bound in that declaration context.

### Shape metadata is type-level information until decay destroys it

This explains a common surprise:

~~~cpp
template<class T>
void f(T value);
~~~

Calling `f(array)` does not deduce `T` as an array type, because the by-value parameter cannot be an array object and the argument is adjusted through decay.

By contrast:

~~~cpp
template<class T>
void g(T& value);
~~~

can deduce `T` as an array type.

When generic code unexpectedly “forgets” an array size, check whether a by-value boundary triggered decay.

---

## 3. Array initialization is recursive, but uninitialized scalar elements remain dangerous

Raw arrays are aggregates. Their initialization follows aggregate and element-initialization rules recursively.

### Empty braces value-initialize every element

~~~cpp
int values[5] = {};
~~~

Every element becomes zero.

For class element types, value-initialization follows the corresponding class rules.

This is fundamentally different from:

~~~cpp
int values[5];
~~~

at automatic storage duration, where the scalar elements are not initialized.

Reading those indeterminate values is invalid.

### Partial aggregate initialization zero-initializes the tail

~~~cpp
int values[5] = {1, 2};
~~~

produces:

~~~text
1, 2, 0, 0, 0
~~~

The rule applies recursively to nested arrays:

~~~cpp
int matrix[2][3] = {
    {1, 2, 3},
    {4}
};
~~~

The second row becomes:

~~~text
4, 0, 0
~~~

This makes aggregate initialization convenient, but it can also hide accidental omissions. In configuration-like data, consider whether a named type would make missing fields more visible.

### An omitted bound can be deduced from the initializer

~~~cpp
int values[] = {1, 2, 3};
~~~

has type:

~~~text
int[3]
~~~

The bound is part of the completed array type after initialization.

This works because the initializer supplies enough information to determine the number of elements.

### Brace initialization rejects narrowing conversions

Since C++11:

~~~cpp
// int values[1] = {3.14}; // ill-formed
~~~

The rule is not array-specific; it comes from list-initialization.

For arrays, however, it is particularly useful because long initializer lists otherwise make accidental narrowing easy to miss.

### String literals are lvalue arrays that include the terminator

The literal:

~~~cpp
"ABC"
~~~

has underlying array type:

~~~text
const char[4]
~~~

and is an lvalue expression.

Therefore:

~~~cpp
static_assert(
    std::is_same_v<
        decltype("ABC"),
        const char (&)[4]
    >
);
~~~

The bound is four, not three, because the null terminator is part of the array.

### Initializing `char[]` from a literal creates a separate writable array

~~~cpp
char text[] = "Hi";
~~~

creates:

~~~text
'H', 'i', '\0'
~~~

inside a new local array.

That local copy is writable:

~~~cpp
text[0] = 'B';
~~~

This does not modify the string literal.

By contrast:

~~~cpp
const char* p = "Hi";
~~~

stores only a pointer to the literal.

The difference is visible immediately in `sizeof`:

~~~text
sizeof(text) -> 3
sizeof(p)    -> pointer size
~~~

### C++20 changed the element type of UTF-8 string literals

Since C++20:

~~~cpp
u8"ABC"
~~~

has element type `const char8_t` rather than `const char`.

Conceptually:

~~~cpp
static_assert(
    std::is_same_v<
        decltype(u8"ABC"),
        const char8_t (&)[4]
    >
);
~~~

This can break older APIs that assumed a UTF-8 literal could be passed directly as `const char*`.

The change is intentional: UTF-8 code units now have a distinct type.

### Standard C++ has no zero-length built-in array type

This is not standard C++:

~~~cpp
int values[0];
~~~

Some compilers accept it as an extension.

If zero length is part of the domain, use a type that represents it portably:

~~~cpp
std::array<int, 0>
std::span<int>
std::vector<int>
~~~

depending on ownership and extent requirements.

---

## 4. Multidimensional arrays are arrays of arrays

The declaration:

~~~cpp
int matrix[2][3];
~~~

does not create a magical two-dimensional runtime container.

Its type is:

~~~text
array 2 of array 3 of int
~~~

or:

~~~text
int[2][3]
~~~

The outer array's element type is `int[3]`.

### The first decay of a matrix produces a pointer to a row

Given:

~~~cpp
int matrix[2][3];
~~~

the expression `matrix` normally decays to:

~~~text
int (*)[3]
~~~

not:

~~~text
int*
~~~

because the first element of the outer array is an entire row of type `int[3]`.

This is why:

~~~cpp
matrix + 1
~~~

advances by one row.

### `&matrix` adds one more array level to the pointer type

~~~cpp
auto p = &matrix;
~~~

has type:

~~~text
int (*)[2][3]
~~~

and:

~~~cpp
p + 1
~~~

moves one complete `2 x 3` matrix object.

The progressively nested pointer types reflect progressively larger array objects.

### Multidimensional built-in arrays have contiguous nested storage

Array elements are contiguously allocated.

Since the outer elements are row arrays, the rows themselves are adjacent:

~~~cpp
static_assert(
    sizeof(int[2][3]) == 6 * sizeof(int)
);
~~~

This is why multidimensional arrays have predictable row-major representation.

But a subtle distinction matters:

> Contiguous representation does not turn the nested object model into one flat inner array object.

### A pointer to the first scalar is not a license to walk across sibling rows

Consider:

~~~cpp
int matrix[2][3];
int* p = &matrix[0][0];
~~~

`p` points into the inner array `matrix[0]`, whose type is `int[3]`.

Pointer arithmetic on `p` is defined relative to that inner array (plus its one-past position).

Treating:

~~~cpp
p[3]
~~~

as a portable way to access `matrix[1][0]` relies on a flattened model the language's pointer-arithmetic rules do not provide.

The storage is adjacent, but the array object used for scalar pointer arithmetic is the row.

This is one of the strongest reasons to preserve row types or use a multidimensional view rather than manually flattening a nested raw array.

### `T**` is not a multidimensional array type

A `T**` points to a `T*` object.

A `T[R][C]` decays to a pointer to `T[C]`.

Those are different layouts and different types:

~~~text
T**       -> pointer -> pointer -> T
T (*)[C]  -> pointer -> contiguous row of C T objects
~~~

A typical dynamically allocated jagged matrix may legitimately use `T**`, because each row pointer can refer to a different allocation.

A built-in matrix is not such a pointer graph.

### Function parameters must preserve the inner row extent

This works:

~~~cpp
void f(int matrix[][3]);
~~~

but the parameter is adjusted to:

~~~cpp
void f(int (*matrix)[3]);
~~~

The first outer bound may be omitted because the parameter is already a pointer after adjustment.

The inner bound `3` is required because it determines the pointed-to row type and therefore the stride of:

~~~cpp
matrix + 1
~~~

A reference can preserve the whole shape:

~~~cpp
template<std::size_t R, std::size_t C>
void f(int (&matrix)[R][C]);
~~~

### Nested range-for loops preserve row structure naturally

~~~cpp
int matrix[2][3] = {{1,2,3}, {4,5,6}};

for (auto& row : matrix) {
    for (int value : row) {
        // ...
    }
}
~~~

Here `row` is a reference to `int[3]`.

No decay is needed to express the iteration structure.

### C++23 `std::mdspan` separates storage from multidimensional indexing

C++23 adds `std::mdspan`, a non-owning multidimensional view.

Given genuinely flat storage:

~~~cpp
std::array<int, 6> storage = {
    1, 2, 3,
    4, 5, 6
};

std::mdspan view{
    storage.data(),
    2, 3
};
~~~

the view supplies 2D indexing over the same six elements:

~~~cpp
assert(view.extent(0) == 2);
assert(view.extent(1) == 3);
assert(view[1, 0] == 4);
~~~

The key architectural difference is explicit:

~~~text
storage ownership/layout
        +
multidimensional mapping
        =
mdspan view
~~~

`mdspan` does not own the elements.

It records extents and mapping/access policy so multidimensional indexing does not require pretending the backing storage is a built-in nested array.

### `mdspan` can represent layouts that a raw `T[R][C]` type cannot

A built-in multidimensional array has one fixed nested row-major layout.

`mdspan` separates:

* extents;
* mapping;
* accessor;
* underlying storage.

This makes it suitable for HPC and systems APIs where the same memory may need:

* row-major interpretation;
* column-major interpretation;
* custom strides;
* device-specific access policies.

The view abstraction is richer than “pointer plus two integers” because the mapping policy is part of the type.

---

## 5. Dynamic arrays are primarily an ownership problem

A built-in array with automatic or static storage has lifetime controlled by its surrounding storage duration.

A dynamically allocated array introduces a separate question:

> Who owns the allocation and which deallocation operation matches it?

### `new T[N]` must be matched with `delete[]`

~~~cpp
int* p = new int[5]{};

delete[] p;
~~~

Using scalar delete is undefined behavior:

~~~cpp
// delete p; // wrong
~~~

The distinction is part of the allocation/deallocation contract.

Do not reason from the apparent type `int*` alone. The pointer value does not encode “scalar allocation” versus “array allocation” in its C++ static type.

### Array allocation may need implementation metadata

Implementations often need to remember enough information to destroy an array correctly, especially for non-trivial element types.

That implementation metadata is commonly described as an **array cookie**.

The standard does not require a particular cookie layout, size, or even that one exist in every case.

This is another reason to treat `new[]`/`delete[]` as a semantic pair rather than manually reverse-engineering allocator layout.

### A type alias can hide array-ness without changing the allocation rules

~~~cpp
using week = int[7];

int* p = new week;
delete[] p;
~~~

The alias can make the code visually misleading: the new-expression names one `week` type, but that type is itself an array.

The lesson is not that array aliases are forbidden.

The lesson is:

> If ownership depends on remembering that an alias secretly denotes an array, the API is too easy to misuse.

Prefer a value/owner type that makes the ownership model visible.

### Arrays are not covariant, even when individual object pointers are

Given:

~~~cpp
struct Base {
    virtual ~Base() = default;
};

struct Derived : Base {
    int extra;
};
~~~

this conversion is valid for a single object pointer:

~~~cpp
Derived* d = ...;
Base* b = d;
~~~

But this pattern is invalid:

~~~cpp
// Base* b = new Derived[10];
// delete[] b; // undefined behavior
~~~

Why?

Pointer arithmetic on `Base*` would use `sizeof(Base)` while the actual array elements have stride `sizeof(Derived)`.

A virtual destructor cannot repair the wrong array element addressing and deallocation model.

This is a deep rule:

> Object-pointer covariance does not imply array covariance.

### `std::unique_ptr<T[]>` encodes array ownership directly

Modern C++ should normally represent exclusive dynamic-array ownership as:

~~~cpp
auto values =
    std::make_unique<int[]>(count);
~~~

The specialization:

~~~cpp
std::unique_ptr<T[]>
~~~

uses `delete[]` and exposes indexed element access.

This eliminates one entire class of mismatched-delete bugs.

It still does not carry a size.

If the owner needs both allocation and extent as one abstraction, `std::vector<T>` is often the better type.

### C++20 added array support to `std::make_shared`

C++20 made forms such as:

~~~cpp
auto values =
    std::make_shared<int[]>(count);
~~~

standard.

This is occasionally useful when an array truly has shared lifetime.

Do not choose shared ownership merely to avoid deciding who owns the memory. Shared ownership is a semantic commitment with control-block and lifetime consequences.

### C++20 `make_unique_for_overwrite` can skip unnecessary initialization

For scratch buffers whose elements will immediately be overwritten:

~~~cpp
auto scratch =
    std::make_unique_for_overwrite<int[]>(count);
~~~

can avoid value-initializing scalar elements.

That makes the precondition explicit:

> Every element must be written before it is read.

This is an optimization-oriented facility. It should be used only when initialization cost matters and the subsequent code has a clear full-write invariant.

### A pointer still does not carry the dynamic array length

Whether obtained from:

~~~cpp
new T[n]
unique_ptr<T[]>
shared_ptr<T[]>
malloc
a C API
~~~

a raw `T*` data pointer does not intrinsically encode `n`.

If length belongs to the interface, pass it explicitly or use a view/container that carries it.

---

## 6. `std::array` makes a fixed-size array behave like a value

`std::array<T, N>` is the standard fixed-size container abstraction.

Conceptually, it provides array-like storage with ordinary value semantics.

### `std::array` keeps the bound in the type without decay

~~~cpp
std::array<int, 3> values{1, 2, 3};
~~~

The type carries `3`:

~~~text
std::array<int, 3>
~~~

Passing it by value copies the elements rather than decaying to a pointer.

Assignment is defined.

Comparison is defined.

Iterators and container operations are available.

### `std::array` is an aggregate

Initialization remains lightweight:

~~~cpp
std::array<int, 3> values{1, 2, 3};
~~~

No dynamic allocation is implied.

Its contiguous storage makes C interop straightforward:

~~~cpp
legacy_api(
    values.data(),
    values.size()
);
~~~

### `std::array<T, 0>` is a real standard type

A built-in:

~~~cpp
int values[0];
~~~

is not standard C++.

But:

~~~cpp
std::array<int, 0> values;
~~~

is valid.

The useful guarantees are:

~~~cpp
values.empty() == true
values.begin() == values.end()
values.size() == 0
~~~

Do not build logic around `data()` being null. Its exact value for the zero-length specialization is not the semantic contract you need.

Also remember that:

~~~cpp
front()
back()
operator[](0)
~~~

have no valid element to access.

### CTAD and string literals expose a surprising difference

Since C++17, class template argument deduction allows:

~~~cpp
std::array a{1, 2, 3};
~~~

to deduce:

~~~text
std::array<int, 3>
~~~

But:

~~~cpp
std::array text{"abc"};
~~~

deduces an array whose single element is a pointer:

~~~text
std::array<const char*, 1>
~~~

because the literal is treated as an argument from which the element type is deduced.

This is not the result many programmers expect.

### C++20 `std::to_array` converts a built-in array into a `std::array`

C++20 provides:

~~~cpp
constexpr auto text =
    std::to_array("abc");
~~~

which produces:

~~~text
std::array<char, 4>
~~~

including the null terminator.

This is an elegant example of preserving array shape at the boundary before ordinary deduction can erase it.

`std::to_array` is limited to one-dimensional built-in arrays; it does not recursively convert multidimensional arrays.

---

## 7. C++20 `std::span` carries a contiguous extent without owning storage

Many APIs do not need to own an array. They need to observe a contiguous sequence.

Before C++20, such APIs commonly used:

~~~cpp
void process(
    const T* data,
    std::size_t size
);
~~~

C++20 `std::span` gives that pair a vocabulary type.

### Dynamic extent is the modern pointer-plus-length parameter

~~~cpp
void process(
    std::span<const int> values
);
~~~

Callers can supply contiguous sources such as:

~~~cpp
int raw[4];
std::array<int, 4> fixed;
std::vector<int> dynamic;
~~~

without manually spelling pointer and size at each call site.

The span does not own any of them.

### Static extent puts the bound back into the type

~~~cpp
int raw[4];

std::span<int, 4> s{raw};
~~~

Now the extent is part of the span type:

~~~cpp
static_assert(
    decltype(s)::extent == 4
);
~~~

A function can state a fixed-size precondition in its signature:

~~~cpp
void transform_block(
    std::span<float, 16> block
);
~~~

This is often clearer than a raw reference-to-array when the API conceptually wants a view rather than specifically a built-in array object.

### A span is a view, not a lifetime manager

This is the most important rule about `std::span`.

A span can dangle:

~~~cpp
std::span<int> bad()
{
    std::vector<int> values{1,2,3};
    return values;
}
~~~

The span's size metadata does not extend the vector's lifetime.

It is bounds-aware relative to its stored range, not ownership-aware.

### A const span object is not the same as a span of const elements

These are different:

~~~cpp
const std::span<int> a = ...;
std::span<const int> b = ...;
~~~

For `a`, the span descriptor is const, but its elements are still mutable through `operator[]`.

For `b`, the element type is const.

This mirrors the difference between:

~~~cpp
int* const
const int*
~~~

The view object and the viewed element type have separate constness.

### Subviews preserve range semantics without pointer arithmetic at the call site

~~~cpp
std::span<int> values = ...;

auto first = values.first(4);
auto tail = values.subspan(4);
~~~

When compile-time offsets/counts are used with static extents, the resulting span can itself acquire a static extent.

That lets the type system retain more shape information through slicing.

### `std::as_bytes` exposes representation safely

Given:

~~~cpp
std::array<std::uint32_t, 2> words;
~~~

C++20 can create a byte view:

~~~cpp
auto bytes =
    std::as_bytes(std::span{words});
~~~

The result is a non-owning span of `const std::byte` over the object representation.

This is preferable to inventing an unrelated pointer type with `reinterpret_cast` just to inspect bytes.

### C++20 ranges treat arrays and spans as ranges without forcing decay

Raw arrays work with range customization points such as:

~~~cpp
std::ranges::begin(a)
std::ranges::end(a)
std::ranges::size(a)
std::ranges::data(a)
~~~

This is significant because generic range algorithms can work with the array as a range abstraction rather than immediately converting it into “pointer plus folklore.”

---

## 8. Choosing among raw arrays, `std::array`, `std::span`, `std::vector`, and `std::mdspan`

The modern array question is rarely “Which syntax is newest?”

It is:

> Which type communicates ownership, extent, dimensionality, and mutability correctly?

### Use a raw `T[N]` when the language-level array type itself matters

Raw arrays remain appropriate for:

* ABI and C interoperability;
* embedded layouts;
* aggregate members with exact built-in representation requirements;
* examples that need pointer-to-array types;
* low-level code where the array type itself is part of the contract.

Do not replace every raw array mechanically.

### Use `std::array<T, N>` for fixed-size value semantics

Prefer `std::array` when:

* the size is compile-time fixed;
* the sequence should be copyable/assignable as one value;
* container APIs are useful;
* zero length should be representable;
* raw-array decay would be undesirable.

### Use `std::span<T>` for non-owning contiguous parameters

Prefer span when:

* ownership belongs elsewhere;
* the callee needs elements and extent;
* the source may be a raw array, `std::array`, vector, or another contiguous range;
* pointer-plus-length should be one parameter.

### Use `std::vector<T>` when dynamic extent and ownership belong together

A vector carries:

* dynamic size;
* capacity;
* ownership;
* contiguous storage.

If the function/object is responsible for the lifetime and resizable extent, a view is too weak and a raw pointer is too vague.

### Use C++23 `std::mdspan` for multidimensional non-owning indexing

`mdspan` fits when:

* storage ownership is separate;
* there are multiple extents;
* mapping/stride policy matters;
* the same backing storage may have different multidimensional interpretations.

It is a view, not a multidimensional vector owner.

---

## 9. Cross-cutting pitfalls worth remembering

### Pointer arithmetic belongs to an array object, not merely contiguous addresses

C++ pointer arithmetic is defined relative to array objects and their one-past positions.

This distinction matters for:

* pointer-to-array arithmetic;
* nested arrays;
* subobjects;
* attempts to flatten matrices manually.

Physical adjacency is necessary for many low-level operations, but it is not by itself the language rule that makes arbitrary typed pointer arithmetic valid.

### Bounds information can disappear in one token

This declaration:

~~~cpp
void f(int a[]);
~~~

looks array-oriented but has already lost the bound.

This assignment:

~~~cpp
auto p = array;
~~~

does the same thing.

Once only `T*` remains, diagnostics and generic code have less information to work with.

### `sizeof` is correct even when the programmer's model is wrong

This:

~~~cpp
sizeof(array)
~~~

and this:

~~~cpp
sizeof(pointer_parameter)
~~~

produce different values because they operate on different types.

`sizeof` is not “context-sensitive” in a mysterious way. The preceding language conversions and parameter adjustments changed the operand's type.

### Contiguous does not mean interchangeable

These can all describe contiguous integers:

~~~text
int[6]
int[2][3]
std::array<int, 6>
std::span<int>
std::mdspan<...>
std::vector<int>
~~~

They are not interchangeable abstractions.

Their differences include:

* ownership;
* type-level shape;
* runtime extent;
* dimensional mapping;
* assignability;
* lifetime behavior.

### Views improve interfaces but can make dangling easier to hide

A span or mdspan is easier to pass correctly than a naked pointer-plus-size pair.

It is also easy to store past the lifetime of its source.

View types solve shape/interface problems, not ownership.

### Built-in arrays do not become polymorphic containers

Array covariance is not part of C++ object-pointer covariance.

If heterogeneous polymorphic objects are required, use indirection:

~~~cpp
std::vector<std::unique_ptr<Base>>
~~~

or another ownership model.

Do not store derived objects in an array and then reinterpret the array through a base-element pointer.

---

## Rules worth keeping in working memory

The mental model that explains most array surprises is that **shape lives in the type until an interface or expression converts the array to a pointer**.

> A C++ array is a typed object with an extent; decay is a conversion that discards that extent.

### Core rules

1. **Treat `T[N]` as a distinct object type, not as spelling for `T*`.** The bound is part of the type.
2. **Assume ordinary by-value boundaries erase raw-array shape.** Use references, `std::array`, or `std::span` when the extent matters.
3. **Remember that `T[R][C]` is an array of `T[C]` rows.** Its first decay produces `T (*)[C]`, not `T*`.
4. **Keep pointer arithmetic inside the array object that defines it.** Adjacency of sibling subarrays does not create one larger scalar array.
5. **Match `new[]` with array-aware ownership.** Prefer `unique_ptr<T[]>` or a container over manual delete forms.
6. **Use `std::array` for fixed-size ownership and value semantics.** It preserves extent without raw-array decay.
7. **Use `std::span` for non-owning contiguous one-dimensional access.** It carries extent but not lifetime.
8. **Use `std::mdspan` when multidimensional indexing and mapping are the problem.** It is a view over storage, not an owner.
9. **Do not infer a zero-length built-in array from compiler acceptance.** Standard C++ requires another representation.
10. **Treat string literals as arrays first and pointers only after decay.** The terminator and C++20 `char8_t` are part of the type story.

### Pitfalls at a glance

| Pitfall | What happens | Do instead |
|---|---|---|
| `void f(int a[10])` | Parameter is still `int*`; bound is lost | Use `T(&)[N]` or `std::span` |
| `auto p = array` | Array decays to pointer | Use `auto&` when shape must remain |
| `sizeof(param)` after array adjustment | Reports pointer size | Carry/deduce the extent before decay |
| Treating `T[R][C]` as `T**` | Wrong type and layout model | Use pointer-to-row, references, or `mdspan` |
| Flattening nested array via `&m[0][0]` pointer arithmetic | Crosses an inner-array boundary | Preserve rows or use flat backing storage + `mdspan` |
| `delete p` after `new T[n]` | Undefined behavior | `delete[]` or array-aware RAII |
| `Base* p = new Derived[n]` | Array stride/type mismatch | Use indirection per element |
| Assuming `std::array<T,0>::data() == nullptr` | Non-portable assumption | Test `empty()` / iterator equality |
| Storing `std::span` past source lifetime | Dangling view | Make ownership/lifetime explicit |
| `std::array{"abc"}` expecting characters | Deduces one pointer element | Use `std::to_array("abc")` |

### Mental model summary

| Feature | Raw `T[N]` | `std::array<T,N>` | `std::span<T>` | `std::vector<T>` | `std::mdspan` |
|---|---|---|---|---|---|
| Owns elements | Yes, by storage duration | Yes | No | Yes | No |
| Extent in type | Yes | Yes | Optional static extent | No | Static/dynamic mix |
| Dynamic size | No | No | View may have dynamic extent | Yes | Extents may be dynamic |
| Decays to pointer | Yes in many expressions | No | No | No | No |
| Copy/assignment as one value | Array assignment: no | Yes | Copies view | Yes | Copies view |
| Contiguous storage | Yes | Yes | Requires contiguous range | Yes | Depends on mapping/backing storage |
| Multidimensional semantics | Nested arrays | Nest `std::array` types | One-dimensional | One-dimensional owner | Yes |
| Carries lifetime/ownership | Object itself | Object itself | No | Yes | No |
| Common pitfall | Silent decay | Zero-size assumptions | Dangling | Iterator invalidation | Backing lifetime/mapping assumptions |

### Review checklist

When reviewing array code, ask:

- [ ] Is the extent still present where correctness depends on it?
- [ ] Has any function parameter written with `[]` already adjusted to a pointer?
- [ ] Is pointer arithmetic confined to the correct array object and one-past position?
- [ ] Is a multidimensional built-in array being incorrectly modeled as `T**` or one flat `T*`?
- [ ] Does a non-owning `span`/`mdspan` clearly outlive neither its backing storage nor its owner?
- [ ] Is every dynamic array owned by an RAII type or container with the correct deletion form?
- [ ] Are zero-length cases represented with a standard type?
- [ ] Are string/UTF-8 literal element types and terminators accounted for?
- [ ] Would `std::array`, `std::span`, `std::vector`, or `std::mdspan` make the contract clearer than raw syntax?

---

## Diagnostics, useful compiler settings and extensions

Array mistakes range from statically visible shape loss to runtime out-of-bounds access. Use compiler warnings to catch suspicious declarations, sanitizers to catch executed memory errors, and standard vocabulary types so more invariants exist in the type system before tooling is needed.

### Warnings

| Compiler | Flags | Catches |
|---|---|---|
| GCC | `-Wall -Wextra -Wpedantic -Warray-bounds -Wsizeof-array-argument -Wvla -Wstringop-overflow` | Constant/provable bounds errors, array-parameter `sizeof` mistakes, VLAs, some overflow into arrays |
| Clang | `-Wall -Wextra -Wpedantic -Warray-bounds -Wsizeof-array-argument -Wvla` | Bounds diagnostics, adjusted-array-parameter mistakes, non-standard VLAs |
| MSVC | `/W4 /permissive- /analyze` | General array misuse plus static-analysis diagnostics such as potential buffer overruns |

Warnings become much stronger when the compiler still sees an array type or statically sized span. A naked pointer erases information the diagnostic engine could otherwise exploit.

### Sanitizers and runtime checks

| Tool | Flag | Detects |
|---|---|---|
| AddressSanitizer | `-fsanitize=address` (GCC/Clang), `/fsanitize=address` (MSVC) | Stack/heap/global out-of-bounds, use-after-free, many lifetime errors |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | Several invalid indexing/pointer operations and related UB |
| Bounds sanitizer | `-fsanitize=bounds` (GCC/Clang support varies) | Selected array bounds violations |
| libstdc++ debug mode | `-D_GLIBCXX_DEBUG` | Bounds/iterator checks for many standard containers (not raw arrays) |

ASan is particularly effective for teaching array failures because an off-by-one that silently corrupts adjacent memory in a release build often becomes an immediate diagnostic.

### Semantics-changing options

| Option | Effect on this topic | Recommendation |
|---|---|---|
| `-std=gnu++20` / `-std=gnu++23` | Enables GNU extensions such as variable-length arrays in C++ | Prefer `-std=c++20` / `-std=c++23` for portable course code |
| `-fno-bounds-check`-style vendor options | May disable implementation safety checks | Avoid unless measurement proves the checks are a bottleneck |
| Hardened standard-library modes | May turn some invalid standard-container/span operations into diagnosed contract failures | Useful in debug/hardened builds; do not rely on them to validate raw arrays |

### Compiler extensions

| Extension | Compilers | What it gives | Standard alternative / note |
|---|---|---|---|
| Variable-length arrays, e.g. `int a[n]` | GCC/Clang in GNU modes | Runtime-sized stack array syntax | `std::vector`, `std::array` for fixed extent, or allocator/arena storage |
| Zero-length arrays `T a[0]` | GCC and others as extension | Tail-storage/layout trick | Flexible design with explicit byte storage; no standard built-in zero-length array |
| Flexible array members | GCC/Clang as C-family extension | Struct tail payload | Explicit allocation/layout wrapper; not standard C++ |
| `_countof(array)` | MSVC | Compile-time element count macro | `std::size(array)` since C++17 |
| `__builtin_object_size` / `__builtin_dynamic_object_size` | GCC/Clang | Compiler estimate of reachable object size | No exact general standard equivalent; prefer type/extent-carrying APIs |
| `__attribute__((counted_by))` and related bounds metadata | Compiler/platform-specific | Improves analysis of C-style pointer/length layouts | Prefer `std::span` inside C++ APIs |

* Prefer the standard facility when it exists; keep extensions behind a small wrapper header or macro.
* Guard them with feature-test macros (`__cpp_*`, `__has_cpp_attribute`, `__has_builtin`) rather than compiler-version checks.
* `-pedantic` / `-pedantic-errors` (GCC/Clang) and `/permissive-` (MSVC) flag non-standard code; `-std=c++NN` instead of `-std=gnu++NN` disables GNU extensions.

### Libraries and tooling beyond the standard

| Tool / library | Purpose | Typical use for this topic |
|---|---|---|
| clang-tidy / cppcheck | Static analysis | Bounds/lifetime checks, suspicious pointer arithmetic, modernization suggestions |
| Microsoft GSL `span` / `multi_span`-style facilities | Guideline-oriented views and annotations | Legacy projects that cannot yet use all standard facilities |
| Boost.MultiArray | Owning/multidimensional array abstractions | Multidimensional data before or beyond `std::mdspan` needs |
| Compiler Explorer | Inspect generated code per compiler | Verify whether `span`/`array` abstractions optimize to the expected pointer arithmetic |
| ASan-enabled tests/fuzzing | Runtime verification | Exercise edge indices, empty ranges, lifetime transitions |

The layered strategy is: preserve extent in types and APIs first, enable warnings, run sanitizers, isolate non-standard extensions, then use static analysis and boundary-heavy tests.

---

## Standards timeline

### C++98/03: built-in arrays, decay, and the original C compatibility model

* bounded and unknown-bound raw array types;
* array-to-pointer conversion and adjusted array function parameters;
* multidimensional arrays as nested arrays;
* `new[]` / `delete[]`;
* string literals as array objects, with historical compatibility rules inherited from C.

### C++11: fixed-size container vocabulary and array type traits

* `std::array<T,N>` becomes the standard fixed-size container;
* `std::begin` / `std::end` gain raw-array overloads;
* range-for works naturally with arrays;
* `std::rank`, `std::extent`, `std::remove_extent`, and related type traits;
* list-initialization rejects narrowing conversions.

### C++14: safer dynamic-array ownership factories

* `std::make_unique<T[]>` standardizes convenient exclusive ownership for unknown-bound arrays;
* no material change to raw array decay rules.

### C++17: nonmember container access and deduction improvements

* `std::size`, `std::data`, and `std::empty` work uniformly with raw arrays and containers;
* class template argument deduction makes `std::array{...}` concise;
* deduction does not make string-literal-to-`std::array` behavior identical to `std::to_array`.

### C++20: spans, array conversion helpers, traits, and ranges

* `std::span` provides a standard non-owning contiguous view with static or dynamic extent;
* `std::to_array` converts one-dimensional built-in arrays into owning `std::array` values;
* `std::is_bounded_array` and `std::is_unbounded_array` distinguish known and unknown bounds;
* ranges customization points work naturally with arrays and spans;
* `std::make_shared<T[]>` and array-aware overwrite factories become available;
* UTF-8 literals use `char8_t` element type.

### C++23: multidimensional views

* `std::mdspan` standardizes non-owning multidimensional indexing over user-provided storage;
* `std::span` gains const iterator accessors and a standard guarantee that its specializations are trivially copyable;
* the fundamental raw-array decay and function-parameter adjustment rules remain unchanged.

### C++26: safer access and richer multidimensional slicing

* `std::span::at` adds checked element access;
* `std::submdspan` provides standardized multidimensional subviews;
* additional mdspan layout/accessor facilities continue to evolve (expected/shipping status depends on the standard library).

The version dependency belongs in code and build constraints, not only in assumptions based on the compiler currently used on one machine.

---

## Further reading

The standard-library facilities are easiest to understand when read together with the core-language array rules they are designed to preserve or replace.

### Standard and language reference

* C++ array declaration and array-to-pointer conversion: <https://en.cppreference.com/w/cpp/language/array>
* `std::array`: <https://en.cppreference.com/w/cpp/container/array>
* `std::to_array`: <https://en.cppreference.com/w/cpp/container/array/to_array>
* `std::span`: <https://en.cppreference.com/w/cpp/container/span>
* `std::mdspan`: <https://en.cppreference.com/w/cpp/container/mdspan>
* Array type traits: <https://en.cppreference.com/w/cpp/types/is_array>

### Proposals and Core Guidelines

* P0122R7 — `span: bounds-safe views for sequences of objects`: <https://wg21.link/p0122r7>
* P0009R18 — `mdspan: A Non-Owning Multidimensional Array Reference`: <https://wg21.link/p0009r18>
* C++ Core Guidelines, rule `R.14` — Avoid `[]` parameters, prefer `span`: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r14-avoid--parameters-prefer-span>
* C++ Core Guidelines, rule `C.152` — Never assign a pointer to an array of derived class objects to a pointer to its base: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c152-never-assign-a-pointer-to-an-array-of-derived-class-objects-to-a-pointer-to-its-base>

### Articles and talks

* ISO C++ FAQ, arrays and pointers — useful historical context for why decay and parameter adjustment exist: <https://isocpp.org/wiki/faq/arrays>
* Compiler Explorer — compare raw-array, `std::array`, and `std::span` code generation: <https://godbolt.org/>

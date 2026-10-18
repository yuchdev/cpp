# 06. Arrays from C++98 to C++26: Shape, Decay, Views, and Ownership

C++ arrays look deceptively primitive. Their syntax predates C++, they interoperate directly with C, and a raw array often seems to turn into a pointer as soon as it is used. That encourages a simplified mental model in which `T[N]` is “basically a `T*` with some storage behind it.”

That model is wrong in exactly the places where experienced C++ programmers get into trouble. A raw array is a distinct object type whose bound is part of the type. Array-to-pointer conversion is a context-dependent conversion that discards shape information. Multidimensional arrays are nested array objects, not pointer graphs. Dynamic arrays add a separate ownership problem. Modern C++ does not remove raw arrays; instead, it gives us vocabulary types—`std::array`, `std::span`, and C++23 `std::mdspan`—that preserve the useful parts of the array model without forcing every API to speak in pointer arithmetic.

For experienced C++ developers, the more important questions are:

* what exactly survives when an array crosses a function or template boundary;
* when a pointer still represents an array element and when shape information has already been lost;
* why contiguous multidimensional storage is not equivalent to `T**` or even to one flat `T*` range;
* which ownership and lifetime guarantees are encoded by `T[N]`, `std::array`, `std::span`, and `std::mdspan`;
* and which old array idioms become unnecessary or actively misleading in C++20/23.

This chapter works from the C++98 core language model to C++26 library evolution. Its baseline builds with C++20, while C++23/26 snippets are conditional demonstrations and may require newer standard-library implementations.

This chapter follows the examples in this directory:

1. [`01_arrays_decay_and_overloads/arrays_decay_and_overloads.cpp`](01_arrays_decay_and_overloads/arrays_decay_and_overloads.cpp) — array-to-pointer conversion, `sizeof`, pointer-to-array types, and adjusted function parameters.
2. [`02_arrays_templates_and_traits/arrays_templates_and_traits.cpp`](02_arrays_templates_and_traits/arrays_templates_and_traits.cpp) — preserving array bounds in templates and inspecting array shape with type traits.
3. [`03_arrays_initialization_and_literals/arrays_initialization_and_literals.cpp`](03_arrays_initialization_and_literals/arrays_initialization_and_literals.cpp) — aggregate initialization, zero-filling, narrowing, string literal array types, and C++20 `char8_t`.
4. [`04_arrays_multidim_and_pointers/arrays_multidim_and_pointers.cpp`](04_arrays_multidim_and_pointers/arrays_multidim_and_pointers.cpp) — arrays of arrays, row types, pointer-to-array arithmetic, and C++23 `std::mdspan`.
5. [`05_arrays_memory_and_pitfalls/arrays_memory_and_pitfalls.cpp`](05_arrays_memory_and_pitfalls/arrays_memory_and_pitfalls.cpp) — `new[]`/`delete[]`, hidden array aliases, smart-pointer ownership, and polymorphic-array hazards.
6. [`06_modern_arrays/modern_arrays.cpp`](06_modern_arrays/modern_arrays.cpp) — `std::array`, `std::to_array`, `std::span`, ranges access, and byte views.

The repository currently builds as C++20. The six existing examples remain compatible with that baseline; newer facilities in the narrative are presented as guarded, version-specific demonstrations.

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

## 8. Array declarations and unknown bounds have a richer type system than their syntax suggests

An array declaration is not merely an element type followed by a length. The declarator determines a nested type, and apparently small changes in where a qualifier, pointer, or reference appears can change the program's ABI. This is particularly relevant when maintaining C++98 interfaces alongside C++20 range-oriented code.

### Read complicated declarations from the identifier outward

A single declaration can distinguish an array of pointers from a pointer to an array. Neither arrangement can be deduced reliably from how the resulting values might print as addresses.

```cpp
int some_array[4]{};
int* elements[4]{};         // four independent int* objects
int (*rows)[4] = nullptr;  // one pointer to an array of four int
int (&alias)[4] = some_array; // binds to an existing array
```

In this snippet, `some_array` must denote a live `int[4]` object. The `elements` object is itself an array; `rows` is a single pointer object; and `alias` binds to an array without copying it. This distinction survives all the way to pointer arithmetic and function signatures.

### Element types must satisfy array completeness restrictions

A built-in array must have a valid object element type. An array cannot directly contain references, functions, `void`, or elements of an incomplete class type at its definition. Arrays *of pointers* to such things are a different matter.

```cpp
struct Node;
Node* links[8]{};      // complete pointer objects; Node can be incomplete
// Node objects[8];    // ill-formed here: Node is incomplete
// int& refs[3];       // ill-formed: array of references
```

A declaration such as `extern Node objects[];` can defer both the bound and object definition, but actually defining an array requires the element type to be complete at the appropriate point. An unknown bound is not a promise that incomplete elements are somehow supported.

### Unknown-bound arrays remain arrays rather than pointers

The incomplete type `int[]` has no known first extent, but it is not `int*`. This is why one may declare an array externally without knowing its eventual bound, while `sizeof` cannot compute its size in that context.

```cpp
extern int external_table[];

// sizeof(external_table); // ill-formed: bound is unknown
void consume(int data[]); // adjusted to int* as a function parameter
```

An unknown-bound array may occur as the first dimension of a multidimensional type, as in `extern int pixels[][4];`, because the inner element type `int[4]` is complete. `int pixels[4][]` is not a way to describe four variable-length rows in standard C++.

### C++20 relaxed conversions to unknown-bound array references and pointers

C++20 permits certain conversions from a pointer or reference to a known-bound array to the corresponding unknown-bound array type. That change is subtle because it operates at the *pointer-to-array* level; it does not make the array decay into an ordinary `int*` carrying an unknown size.

```cpp
int values[3]{1, 2, 3};
int (&unknown)[] = values;  // C++20
int (*p_unknown)[] = &values; // C++20
```

The name `unknown` still denotes an array. Nevertheless, `sizeof(unknown)` is not available because its *reference target type* has unknown bound. A pointer to an array of unknown bound does not gain the right to perform pointer arithmetic as though the bound were known. Preserve the actual extent separately when algorithms need it.

### Array cv-qualification applies to the element type

Qualifying an array type through an alias qualifies its elements. This matters in template metaprogramming because the type `const int[4]` is not an array of mutable integers with a const descriptor.

```cpp
using Four = int[4];
using ReadOnly = const Four;
static_assert(std::is_same_v<ReadOnly, const int[4]>);
```

A raw array has no separately assignable descriptor whose constness can differ from its elements. Contrast this with `const std::span<int>`, where the view object is const but access to mutable elements is still allowed.

### Raw arrays cannot be returned by value, but references can be returned

A built-in array cannot be the by-value return type of a function. Yet a function can return a reference or pointer to an array when the object lifetime is assured, and a function can return `std::array<T,N>` by value.

```cpp
template<std::size_t N>
int (&identity(int (&a)[N]))[N] { return a; }

std::array<int, 3> make_values() { return {1, 2, 3}; }
```

The reference-returning function retains shape but transfers no ownership. Returning an array reference to a local automatic object would dangle just as surely as returning `int*` to its first element.

---

## 9. Arrays have construction and lifetime rules, not just memory layouts

Arrays of class objects make it impossible to maintain the fiction that an array is merely an untyped allocation. Every element has an independent lifetime, initialization step, and destruction obligation. These rules become especially consequential under exceptions and when uninitialized storage is involved.

### Array construction follows element order, and partial construction unwinds safely

When an array of class objects is initialized, elements are initialized in increasing subscript order. On normal scope exit, successfully constructed elements are destroyed in reverse order. If construction of a later element throws, the already-constructed elements are destroyed before the allocation or containing object unwinds.

```cpp
struct Resource {
    Resource();   // may throw
    ~Resource();
};

void example()
{
    Resource resources[4]; // constructs 0,1,2,3; destroys 3,2,1,0
}
```

This is why a properly constructed array can provide exception safety without explicit cleanup code. An array of resource-owning objects is often preferable to several independent raw allocations, but remember that stack requirements grow with the full object size.

### Value initialization and default initialization are not interchangeable

An automatic `int values[4];` does not initialize its scalar elements. By contrast, `int values[4]{};` value-initializes them to zero, and static-storage arrays are zero-initialized as part of static initialization.

```cpp
int no_initialization[4]; // do not read before assigning elements
int zeroed[4]{};          // every element is zero
```

The distinction also applies to allocation: `new int[n]` does not value-initialize each scalar element, but `new int[n]{}` does. C++26 refines the standard's wording on *indeterminate* and *erroneous* values, discussed later; it does not make reading an unwritten ordinary `int` safe.

### Array lifetime cannot be fabricated with a cast

Storage, an address, and a live object are separate requirements. For trivially copyable or implicit-lifetime types, modern C++ provides specific operations and implicit-object-creation rules; for general class types, explicit construction and destruction are required.

```cpp
alignas(int) std::byte storage[sizeof(int)];
// *reinterpret_cast<int*>(storage) = 42; // do not use a cast
// as an unexplained lifetime-creation protocol
```

The portable choice for an ordinary integer is almost always to declare an `int` object in the first place. For real low-level storage reuse, study the C++20 implicit-lifetime model and C++23 `std::start_lifetime_as` (where available), rather than assuming alignment by itself constructs an array of objects.

### Overlapping byte copies and object copies are different operations

`std::copy` copies a sequence of objects through iterators and, like many algorithms, has restrictions on overlap depending on the operation. `std::memcpy` copies bytes and requires non-overlapping storage regions; `std::memmove` is specified to support overlapping byte ranges.

```cpp
std::array<int, 5> a{0, 1, 2, 3, 4};
std::copy_backward(a.begin(), a.begin() + 4, a.end());
// a now contains 0,0,1,2,3
```

Choose an algorithm whose overlap rules match the actual operation. Byte-wise copying of a trivially copyable representation is not a general replacement for the assignment or copy construction of arbitrary class elements.

### Dynamic array allocation has failure and exception contracts

The expression `new T[n]` can fail during allocation or during individual element construction. Both cases need correctly paired cleanup; a raw pointer that escapes before initialization is complete makes reasoning unnecessarily difficult.

```cpp
auto values = std::make_unique<std::string[]>(count);
// Every element is a constructed std::string;
// ownership is released automatically.
```

Even `std::unique_ptr<T[]>` stores no length by default; when capacity, current size, and storage lifetime all belong together, an owning container is usually clearer. For over-aligned types, also consider the alignment guarantees of the allocator and the placement of each element.

---

## 10. Arrays integrate with algorithms and ranges, but not every range is contiguous

Generic algorithms operate on iterator pairs or ranges, not necessarily on owning containers. A raw array is a legitimate range, and its iteration is a case where the compiler can preserve the full extent instead of forcing the user to calculate a size from pointers.

### An array can be sorted without converting it into a vector

C++98 already permitted sorting a raw array with pointers obtained from its begin and end positions. C++11 added `std::begin` and `std::end` overloads, while C++20 makes the range itself an argument to ranges algorithms.

```cpp
int values[5]{9, 1, 7, 3, 5};
std::sort(std::begin(values), std::end(values)); // C++11
std::ranges::sort(values);                        // C++20
```

The ranges spelling matters because it can preserve more static information at the call boundary. Neither version checks arbitrary indexing elsewhere in the program, and sorting requires element types that satisfy the algorithm's comparison and movement contracts.

### Random access, contiguous iterators, and borrowed ranges answer different questions

A random-access iterator supports efficient positional movement; a contiguous iterator additionally models adjacency of elements in memory. A borrowed range controls whether iterators obtained from a *temporary range object* may remain usable; it is not a declaration of ownership.

```cpp
static_assert(std::ranges::contiguous_range<std::array<int, 4>>);
static_assert(std::ranges::contiguous_range<std::span<int>>);
static_assert(!std::ranges::contiguous_range<std::deque<int>>);
```

A span is typically a borrowed range because destruction of its descriptor does not invalidate iterators on its own. That says nothing about destruction of the **backing array**. The owner must still outlive every iterator and view.

### C++20 range results can encode dangling explicitly

An algorithm invoked on a temporary owning range must not casually return an iterator that appears usable after the owner has been destroyed. The ranges library uses `std::ranges::dangling` in relevant return types when it cannot promise safe borrowing.

```cpp
auto result = std::ranges::find(std::vector{1, 2, 3}, 2);
static_assert(std::is_same_v<decltype(result), std::ranges::dangling>);
```

Here the return type exposes a real lifetime boundary. A raw pointer obtained by calling `.data()` on a temporary container can hide the same hazard because pointers do not encode lifetime.

### Proxy element references break assumptions about arrays of bool

`std::vector<bool>` is a specialized space-efficient container; indexed access generally returns a proxy instead of an ordinary `bool&`. It is not a contiguous range of `bool` objects, despite its familiar container name.

```cpp
std::array<bool, 4> flags{true, false, true, false};
std::span<bool> ordinary{flags}; // actual bool elements
```

Choose `std::array<bool,N>` or another suitable storage format when a `bool*`/span interface is required. For packed bits, expose a bit-oriented interface rather than pretending the bits form an array of independent addressable bool objects.

### Pointer and iterator invalidation should be modeled per owner

Raw array elements remain at stable locations throughout that array object's lifetime. A vector's reallocation may invalidate pointers, references, and spans into its storage, while erase operations have their own invalidation rules even when no reallocation occurs. Moving a `std::array` or a C++26 `std::inplace_vector` moves/copies contained objects rather than transferring a stable heap buffer.

This matters in caches, callbacks, and asynchronous tasks: copying a view into a queue is cheap precisely because it does not copy or retain the underlying data.

---

## 11. Layout, alignment, ABI, and cache behavior constrain real array designs

The type system can establish the shape of an array without saying that the chosen layout is good for a particular processor. In low-level code the physical layout determines cache locality, vectorization opportunities, and binary interoperability. Treat those as separate engineering questions from core-language well-formedness.

### Contiguity is not a universal serialization format

For a built-in `T[N]` array, consecutive elements of type `T` are contiguous, and `sizeof(T[N]) == N * sizeof(T)`. That does not mean an arbitrary `T` has a portable wire representation: endianness, padding, object representations, and ABI layout can differ.

```cpp
struct Record { std::uint32_t id; char tag; };
Record records[4]{};
// Sending sizeof(records) raw bytes is not a portable file format.
```

If the data crosses process, machine, or compiler-version boundaries, serialize well-defined field values into a specified byte order. Do not expose padding bytes or pointers as enduring identifiers.

### Aligning the first element also affects the stride constraints

Arrays align their elements according to the element type. If a type is over-aligned, each element's stride must be compatible with that alignment because each element must occupy an appropriately aligned address.

```cpp
struct alignas(64) CacheLine {
    std::uint64_t counter;
};
CacheLine slots[8]{};
static_assert(alignof(CacheLine) >= 64);
static_assert(sizeof(CacheLine) % alignof(CacheLine) == 0);
```

Such deliberate spacing can reduce false sharing between heavily written counters, but it can also waste cache capacity when most elements are read together. Measure the access pattern before choosing a layout.

### Array-of-structures and structure-of-arrays trade different locality patterns

An array of records is useful when algorithms consume whole records. Structure-of-arrays storage is often better when kernels process one field of many records and can exploit contiguous scalar streams.

```cpp
struct Particle { float x, y, z; };
std::vector<Particle> aos;

struct ParticlesSoA {
    std::vector<float> x, y, z;
};
```

The second design introduces a cross-array invariant: all three vectors must represent the same logical element count. No array syntax enforces that relationship automatically. Its performance advantage depends on access patterns, cache hierarchy, SIMD code generation, and the cost of managing multiple allocations.

### The lack of standard restrict means alias information matters

C++20 has no portable `restrict` keyword. GCC/Clang `__restrict__` and MSVC `__restrict` are vendor-specific contracts and must not be attached to aliases that actually overlap. For numeric kernels, aliasing assumptions may strongly affect auto-vectorization.

A `std::span<float>` parameter communicates contiguity and extent, but **not** that two such spans refer to disjoint storage. If non-overlap is part of the algorithm, document and validate it, then isolate platform-specific optimizer hints behind well-defined interfaces.

### C++26 data-parallel facilities build on well-defined contiguous access

C++26 also brings standard data-parallel types in `<simd>`, with load/store operations and masks designed for portable SIMD-oriented computations. They offer an explicit way to express vector lanes, but do not erase the usual requirements that the selected memory range contain live, accessible, suitably aligned objects.

The older optimization lesson remains valid: a kernel built around contiguous streams, clear aliasing preconditions, and carefully handled remainder elements is easier to reason about than one built around unchecked reinterpretation of arbitrary storage. Feature-test the SIMD library independently of the language mode, and benchmark against compiler auto-vectorization.

### Logical shape, physical pitch, and data ownership must remain separate

Image-processing and scientific libraries often store rows with padding. The logical matrix might be `height × width`, while the actual memory stride between row starts is larger than `width`.

```cpp
std::size_t offset(std::size_t row, std::size_t col,
                   std::size_t pitch) {
    return row * pitch + col; // callers must validate all bounds/overflow
}
```

A flat owner plus explicit pitch can represent this correctly. A built-in `T[R][C]` insists that each row contains exactly `C` elements, while an `mdspan` mapping can describe broader strided layouts without inventing a pointer-to-pointer representation.

---

## 12. `std::mdspan` makes extents, mapping, and access policies independent

C++23 `mdspan` is more than syntactic sugar for a two-dimensional subscript. Its design separates the index space from physical layout and from the means of accessing elements. This is valuable for scientific arrays, tensor engines, image planes, and interoperability with libraries that do not own their incoming data.

### An extent object can mix static and dynamic dimensions

The index space can know some dimensions at compile time while receiving others at runtime. Those two kinds of size information are intentionally represented differently.

```cpp
#if defined(__cpp_lib_mdspan) && __cpp_lib_mdspan >= 202207L
using Shape = std::extents<std::size_t, 3, std::dynamic_extent>;
Shape shape{5};
static_assert(Shape::static_extent(0) == 3);
assert(shape.extent(1) == 5);
#endif
```

A statically fixed row count does not imply the backing storage is automatically allocated. `mdspan` remains a non-owning adapter, even when every extent is compile-time constant.

### `layout_right` and `layout_left` express different stride conventions

For a two-dimensional index `[row, column]`, a right-layout mapping makes the rightmost index contiguous, conventionally giving `row * columns + column`. A left-layout mapping makes the leftmost index contiguous, conventionally giving `column * rows + row`.

```text
logical indices (2 × 3):
[0,0] [0,1] [0,2]
[1,0] [1,1] [1,2]

layout_right physical order:  [0,0] [0,1] [0,2] [1,0] [1,1] [1,2]
layout_left  physical order:  [0,0] [1,0] [0,1] [1,1] [0,2] [1,2]
```

Neither policy is inherently faster. Match the traversal order and library interop requirements to the actual memory layout. Choosing a policy that disagrees with the existing allocation silently reads the wrong logical elements.

### `layout_stride` handles padded rows and general regular strides

A stride mapping associates every index dimension with a physical stride, making it possible to represent an image with row padding or an interleaved view. The mapping must remain valid for the actual backing allocation.

```cpp
#if defined(__cpp_lib_mdspan) && __cpp_lib_mdspan >= 202207L
using Ext = std::dextents<std::size_t, 2>;
using Map = std::layout_stride::mapping<Ext>;
std::array<int, 16> storage{};
Map map{Ext{2, 3}, std::array<std::size_t, 2>{8, 1}};
std::mdspan<int, Ext, std::layout_stride> image{storage.data(), map};
image[1, 2] = 42; // storage[8 + 2]
assert(storage[10] == 42);
#endif
```

This does not itself allocate padding. The backing owner must provide the required storage, alignment, and lifetime. Shape and stride validation are still application obligations.

### Accessors allow policies beyond plain pointer dereference

`mdspan`'s accessor policy controls how a mapped element is accessed. Its handle need not be an ordinary pointer. This is particularly important for custom memory spaces or specialized views.

For ordinary memory, the default accessor is generally preferable: it keeps the contract simple and the generated access close to normal pointer arithmetic. A custom accessor should exist for a concrete memory-model requirement, not simply to add indirection for its own sake.

### A subview may no longer have the original contiguous layout

Slicing a two-dimensional matrix can select a full row, one column, or a strided subset. The resulting logical view may have a different rank and nontrivial strides even when the original data was stored contiguously.

C++26 `std::submdspan` gives standardized slicing operations, while `std::layout_left_padded`, `std::layout_right_padded`, and `std::aligned_accessor` add additional vocabulary for real layouts. Check corresponding library feature macros before using them; an implementation may ship C++23 `mdspan` without its C++26 facilities.

---

## 13. C++26 improves array-adjacent APIs without changing the raw-array model

The C++26 additions matter because they solve problems that forced developers into custom bounded containers or home-grown strided views. They do not reinterpret `T[N]` or remove array-to-pointer decay. A codebase targeting C++20 should use these facilities only in feature-tested demonstrations or in configurations that explicitly require a newer library.

### `std::span::at` offers a checked operation when supported

For C++20/23 `span`, `operator[]` has a valid-index precondition; the library is not required to perform runtime bounds checks for that operation. C++26 adds `at`, which throws `std::out_of_range` for an invalid index.

```cpp
#if defined(__cpp_lib_span) && __cpp_lib_span >= 202311L
std::array<int, 3> values{2, 4, 8};
std::span<int> view{values};
assert(view.at(2) == 8);
// view.at(3) would throw std::out_of_range
#endif
```

`at` is useful at externally controlled indices and validation boundaries. It does not repair a view already dangling because its owner has been destroyed.

### `std::span<const T>` can accept an initializer list for immediate calls

C++26 adds an initializer-list construction path for suitable const-element spans. The motivating use is a callee that consumes a small temporary list during the same full-expression.

```cpp
void consume(std::span<const int> numbers);
#if defined(__cpp_lib_span_initializer_list) && __cpp_lib_span_initializer_list >= 202311L
consume({1, 2, 3}); // backing initializer-list array lives through this call
#endif
```

A stored span created from an initializer list can dangle once the initializer-list backing array expires. This convenience changes construction syntax, **not** borrowing lifetime. Library implementations may use a separate feature-test macro from that used for `span::at`; check both independently.

### `std::inplace_vector` is a fixed-capacity owner with a dynamic size

C++26 adds `std::inplace_vector<T, Capacity>`. Unlike `std::array<T, Capacity>`, it constructs only its logically present elements and permits push/pop operations up to a fixed capacity. Unlike `std::vector`, its element storage is part of the container object rather than an allocation managed through a dynamically growing heap buffer.

```cpp
#if defined(__cpp_lib_inplace_vector) && __cpp_lib_inplace_vector >= 202406L
std::inplace_vector<int, 8> samples;
samples.push_back(10);
samples.push_back(20);
assert(samples.size() == 2);
assert(samples.capacity() == 8);
#endif
```

The fixed capacity does not guarantee that *mutating* elements will preserve every iterator: insertion, erasure, movement, and destruction still have invalidation rules. Operations exceeding capacity have specified failure behavior; check the chosen insertion API rather than assuming an unchecked overflow is harmless. This container is valuable for bounded real-time or embedded workloads where variable-size logical arrays should not allocate during normal operation.

### C++26 `std::submdspan` makes multidimensional slicing composable

With the C++26 subview facility, a row of a right-layout matrix can become a rank-one view without manually manufacturing a pointer and a length.

```cpp
#if defined(__cpp_lib_submdspan) && __cpp_lib_submdspan >= 202306L
std::array<int, 6> data{0, 1, 2, 3, 4, 5};
std::mdspan matrix{data.data(), 2, 3};
auto second_row = std::submdspan(matrix, 1, std::full_extent);
assert(second_row.extent(0) == 3);
assert(second_row[0] == 3);
#endif
```

A column or strided slice may use different mapping properties, so callers must not automatically reinterpret every subview as a `std::span` of physically adjacent elements.

### Uninitialized scalar arrays remain invalid data sources

C++26 distinguishes some *erroneous values* of uninitialized automatic objects from *indeterminate values*, and provides rules governing their use. That vocabulary is important to tools and compilers, but it is not permission to use an unwritten element in arithmetic or indexing.

```cpp
int scratch[16];  // elements are not initialized
for (int& value : scratch)
    value = 0;
```

The example is safe because it writes all elements before reading them. The lesson remains unchanged from C++98: a programmer must establish initialization and object lifetime before using array elements as meaningful values.

---

## 14. Choosing among raw arrays, `std::array`, `std::span`, `std::vector`, and `std::mdspan`

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


## 15. Cross-cutting pitfalls worth remembering

The recurring problems below are the intersection of type-level shape, ownership, and real memory access. They apply as readily to new C++26 views as to raw C++98 arrays.

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
* `-pedantic` / `-pedantic-errors` (GCC/Clang) and `/permissive-` (MSVC) diagnose non-standard code; `-std=c++NN` selects the strict ISO dialect instead of `-std=gnu++NN`, while `-pedantic-errors` is needed when supported extensions must be rejected.

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

The history of C++ arrays is best read as three overlapping stories: the C-inherited array object model; safer owning and borrowing vocabulary; and multidimensional layout facilities. New abstractions do not silently change the semantics of existing raw-array expressions.

### C++98/03: Built-in arrays and the original C-compatible model

Ordinary bounded array declarations have compile-time bounds; unknown-bound array types specify no extent. Since C++98, `new int[n]` has also permitted a runtime first bound. Arrays frequently undergo array-to-pointer conversion in expressions and function parameter adjustments, can be nested, and cannot be assigned as whole arrays. C++98 algorithms already accept iterator pairs formed from array element pointers; pointer arithmetic remains limited to the appropriate array object.

### C++11: Value-like fixed arrays and modern deduction tools

`std::array<T,N>` provides fixed-size owning value semantics; `std::begin`/`std::end`, range-for, and array-related type traits expose shape in generic code. Brace initialization prevents narrowing, and rvalue references improve interfaces involving ownership and element movement. These conveniences coexist with the legacy parameter adjustment rules.

### C++14: Convenient exclusive array ownership

`std::make_unique<T[]>(count)` standardizes a safe factory for exclusive dynamically allocated arrays. It avoids raw manual deletion but does not incorporate the count into `unique_ptr<T[]>` itself. There is no material change to array decay.

### C++17: Non-member accessors and deduction

`std::size`, `std::data`, and `std::empty` unify useful access for raw arrays and containers. Class template argument deduction enables concise `std::array{...}` expressions, while their element-type deduction still differs from converting a built-in character array with `std::to_array`.

### C++20: Ranges, spans, and explicit views

`std::span` gives a non-owning contiguous one-dimensional range with static or dynamic extent. `std::to_array`, bounded/unbounded-array traits, ranges algorithms, and expanded smart-pointer array factories reduce dependence on manual pointer-plus-count interfaces. `char8_t` changes the type of UTF-8 literals; array-of-unknown-bound conversions also gain flexibility.

### C++23: Non-owning multidimensional mapping

`std::mdspan` introduces extents, mappings, and accessors to represent multidimensional data independently from its owner. The availability of `<mdspan>` still varies across standard libraries, so feature testing is necessary even when the compiler accepts C++23 language mode. Raw nested arrays remain arrays of rows, never `T**`.

### C++26: Checked access, slicing, and bounded dynamic sequences

`std::span::at` introduces checked element access, `std::submdspan` adds multidimensional slicing, and `std::inplace_vector` represents an owning array with dynamic logical size but fixed in-object capacity. New padded layout and aligned-accessor options address real multidimensional storage. Initializer-list construction for suitable const spans improves call ergonomics while retaining lifetime hazards. C++26 also introduces data-parallel types in `<simd>` for explicit vector-style operations, and refines uninitialized-value terminology; neither change makes uninitialized ordinary element reads safe. Implementations may ship these facilities on different schedules.

Code examples should declare their **minimum language and standard-library version**, not merely depend on a local compiler version number.

---

## Migration note for Java / Python / C# developers

Managed-language arrays commonly carry runtime bounds and rely on automatic lifetime management. C++ raw arrays instead have compile-time bounds in their types, while pointers and views can lose or borrow that shape without extending the storage lifetime.

### Where intuition transfers and where it breaks

Similar-looking indexing and access do not guarantee the same checks or lifetime behavior.

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| Array extent | Runtime array length | Runtime sequence length | Runtime array length | In `T[N]`'s type, but lost when it decays to `T*` |
| Invalid access | Bounds exception | Exception | Bounds exception | May be undefined behavior; raw pointer access is not automatically checked |
| Lifetime | Managed reachability | Runtime-managed object | Managed reachability | Depends on storage duration and ownership; `std::span` does not own its elements |

### Common wrong assumptions

A successful run does not establish that out-of-bounds pointer arithmetic or access is valid; C++ may give undefined behavior where a managed language throws. Likewise, a pointer, reference, or `std::span` does not keep its referred-to array alive. The array-to-pointer conversion and view-lifetime rules described above are the key differences at API boundaries.

### Idiomatic C++ replacement

Choose a type that states whether the sequence is owned, fixed-size, dynamic-size, or borrowed.

| Habit from managed languages | Idiomatic C++ |
|---|---|
| Fixed-size owned sequence | `std::array<T, N>` |
| Dynamic-size owned sequence | `std::vector<T>` |
| Borrowed contiguous sequence | `std::span<T>` |

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

### C++26 facilities and further technical references

* C++26 `std::inplace_vector`: <https://en.cppreference.com/w/cpp/container/inplace_vector>
* C++26 `std::span::at`: <https://en.cppreference.com/w/cpp/container/span/at>
* C++26 `std::submdspan`: <https://en.cppreference.com/w/cpp/container/mdspan>
* C++26 uninitialized-value rules: <https://en.cppreference.com/w/cpp/language/default_initialization>
* C++26 data-parallel types: <https://en.cppreference.com/w/cpp/numeric/simd>
* Strided mappings and extents: <https://en.cppreference.com/w/cpp/container/mdspan>
* Array declarations, unknown bounds and reference conversions: <https://en.cppreference.com/w/cpp/language/array>

### Articles and talks

* ISO C++ FAQ, arrays and pointers — useful historical context for why decay and parameter adjustment exist: <https://isocpp.org/wiki/faq/arrays>
* Compiler Explorer — compare raw-array, `std::array`, and `std::span` code generation: <https://godbolt.org/>

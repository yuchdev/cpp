# Chapter 06. Arrays: Shape, Storage, Views, and Ownership

The array is one of C++'s oldest abstractions. Its notation was inherited from C, its storage model is simple, and its elements usually occupy precisely the sequence of addresses a programmer expects. Those properties have made it indispensable in numerical software, binary protocols, embedded systems, and the standard library itself. They have also encouraged a misleading shorthand: that an array is essentially a pointer accompanied by some storage.

It is not. An array is a distinct object type with an element type and, when bounded, an extent. A pointer is what certain expressions produce *after converting an array*. That conversion discards information, and the loss is observable in overload resolution, templates, pointer arithmetic, diagnostics, and API design. The problem becomes more interesting with two-dimensional arrays, where physical adjacency does not make every possible flat pointer expression legal, and with dynamic storage, where the address tells us almost nothing about ownership.

Modern C++ did not replace the array object model. Instead, it built several complementary abstractions around it. `std::array` gives fixed-size sequences value semantics; `std::vector` couples dynamically sized storage to ownership; `std::span` conveys a borrowed contiguous extent; and `std::mdspan` generalizes non-owning indexing to multiple dimensions and configurable layouts. The most important skill is not memorizing which is newer, but knowing which guarantee the interface actually needs.

The chapter follows six companion experiments: [array decay and overloads](01_arrays_decay_and_overloads/arrays_decay_and_overloads.cpp), [templates and array traits](02_arrays_templates_and_traits/arrays_templates_and_traits.cpp), [initialization and literals](03_arrays_initialization_and_literals/arrays_initialization_and_literals.cpp), [multidimensional arrays and pointers](04_arrays_multidim_and_pointers/arrays_multidim_and_pointers.cpp), [dynamic arrays and pitfalls](05_arrays_memory_and_pitfalls/arrays_memory_and_pitfalls.cpp), and [modern array abstractions](06_modern_arrays/modern_arrays.cpp). The repository's working baseline is C++20, while the C++23 and C++26 features discussed below depend on newer language and library support.

## 1. An array is a type with a shape

The declaration

~~~cpp
int numbers[5]{1, 2, 3, 4, 5};
~~~

creates one array object whose type is `int[5]`. An `int[6]` is a different type. The distinction is visible to the type system:

~~~cpp
static_assert(!std::is_same_v<int[5], int[6]>);
~~~

No pointer object is constructed as part of that declaration. The array has five element subobjects, and the bound is compile-time type information. A pointer arises when an applicable *array-to-pointer conversion* takes place:

~~~cpp
int* first = numbers;
assert(first == &numbers[0]);
~~~

The result points to the first element, but its type, `int*`, does not record that there are five elements. It could have come from an array of five or five thousand, or from a single object treated as an array of one for limited pointer arithmetic. The language does not reconstruct the lost bound from the pointer value.

This is *decay*, and its most significant property is that it is contextual rather than inevitable.

### 1.1 When an array does not decay

Several expressions deliberately preserve the array's identity. `sizeof` measures the whole array, not a pointer:

~~~cpp
static_assert(sizeof(numbers) == 5 * sizeof(int));
~~~

Taking its address yields a pointer to the entire array:

~~~cpp
auto p = &numbers;
static_assert(std::is_same_v<decltype(p), int (*)[5]>);
~~~

A reference can bind to that array without changing its type:

~~~cpp
int (&alias)[5] = numbers;
~~~

Likewise, an unparenthesized id-expression in `decltype` reports the entity's declared array type:

~~~cpp
static_assert(std::is_same_v<decltype(numbers), int[5]>);
static_assert(std::is_same_v<decltype((numbers)), int (&)[5]>);
~~~

The extra parentheses trigger the ordinary expression rule: `numbers` is an lvalue expression, so `decltype((numbers))` produces an lvalue reference to the array. These are not arbitrary language exceptions. Each construct needs information about the array object itself rather than merely a pointer to element zero.

The distinction between `numbers` after decay and `&numbers` is particularly instructive. On a conventional machine the two pointer values may print with the same numeric address. Nevertheless:

~~~cpp
int* element = numbers;
int (*whole)[5] = &numbers;

++element; // next int
++whole;   // one-past the entire array
~~~

The increment sizes differ because the pointed-to types differ. The first operates in units of `int`, the second in units of `int[5]`. The printed address cannot capture that semantic distinction.

### 1.2 The function parameter that looks like an array

One of C++'s most persistent sources of confusion is the parameter adjustment rule:

~~~cpp
void examine(int items[]);
void examine(int items[10]);
void examine(int* items);
~~~

These spellings declare the same parameter type. Within the function, `items` is a pointer, and `sizeof(items)` reports pointer size. The bound `10` does not become a checked or stored extent. An array parameter declaration can advertise an intention to a reader, but the compiler does not enforce that intention merely because the bound appears in brackets.

This is why the familiar idiom

~~~cpp
sizeof(items) / sizeof(items[0])
~~~

is valid for an actual bounded array expression and wrong inside a function whose parameter has already been adjusted to a pointer. `sizeof` is not malfunctioning; the type changed earlier.

A reference parameter avoids the conversion:

~~~cpp
template<class T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept
{
    return N;
}
~~~

Both the element type and the bound are deduced at compile time. No runtime size field is necessary. A function that specifically requires a built-in array may benefit from this signature; a function that merely needs a contiguous sequence often benefits more from `std::span`.

Finally, raw arrays do not have whole-array assignment semantics:

~~~cpp
int a[3]{1, 2, 3};
int b[3]{4, 5, 6};
// a = b; // ill-formed
~~~

An array can be initialized and its elements can be assigned individually, but it is not an assignable value in the same sense as `std::array`. That seemingly old-fashioned restriction explains much of the subsequent development of C++ container vocabulary.

## 2. Templates can preserve shape without runtime metadata

An array-aware template can retain information that a by-value parameter silently discards:

~~~cpp
template<class T, std::size_t N>
void inspect(T (&array)[N]);

const int values[4]{1, 2, 3, 4};
inspect(values); // T is const int, N is 4
~~~

The technique is useful for legacy array interfaces, compile-time dispatch, exact-size APIs, and educational examples concerned with the underlying object type. It is more specific than a generic contiguous view, because the parameter really does require a built-in array.

The difference between `auto` and `auto&` follows the same rule:

~~~cpp
int values[4]{};

auto p = values;   // int*
auto& r = values;  // int (&)[4]
~~~

An ordinary by-value template parameter behaves like the first case; reference deduction behaves like the second. When a generic function unexpectedly loses an extent, first inspect whether its argument was taken by value or passed through an adjusted array parameter.

The standard type traits expose array shape directly. For a matrix type:

~~~cpp
using Matrix = int[2][5];

static_assert(std::rank_v<Matrix> == 2);
static_assert(std::extent_v<Matrix, 0> == 2);
static_assert(std::extent_v<Matrix, 1> == 5);

using Row = std::remove_extent_t<Matrix>;
using Element = std::remove_all_extents_t<Matrix>;

static_assert(std::is_same_v<Row, int[5]>);
static_assert(std::is_same_v<Element, int>);
~~~

These are properties of the type, not hidden runtime metadata. C++20 further distinguishes `std::is_bounded_array_v<T>` from `std::is_unbounded_array_v<T>`, making it easier to handle known and unknown first extents in generic code.

An unknown-bound declaration deserves particular care:

~~~cpp
extern int external_table[];
~~~

This denotes an array of `int` whose bound is not supplied in the declaration. It is not a pointer declaration. Operations needing the completed extent, such as `sizeof(external_table)`, cannot be performed merely from that incomplete array type. A by-value function parameter written `int items[]`, by contrast, is adjusted to `int*` and has already abandoned array identity.

The useful organizing idea is that **shape is available in the type until a conversion or interface explicitly discards it**. Modern generic code should choose whether to preserve or erase it deliberately.

## 3. Initialization, literals, and the objects inside arrays

Arrays are aggregates, and their initialization proceeds through their elements. Consider the difference between

~~~cpp
int uninitialized[5];
int zeros[5]{};
int partly[5]{1, 2};
~~~

for automatic storage. The first declaration performs no initialization of its scalar elements, which must be assigned before their values are read. Empty braces value-initialize the elements, producing zeros. Partial aggregate initialization initializes the specified elements and value-initializes the remainder, so `partly` contains `1, 2, 0, 0, 0`.

The rule applies recursively:

~~~cpp
int matrix[2][3]{
    {1, 2, 3},
    {4}
};
// second row is {4, 0, 0}
~~~

That convenience has an engineering consequence. Aggregate initialization silently fills omitted scalar elements, which is useful for zero-filled tables but can conceal an accidentally incomplete configuration. A named structure and explicit invariants may be better when every supplied value has a different semantic meaning.

An omitted bound can be deduced from the initializer:

~~~cpp
int numbers[]{1, 2, 3}; // int[3]
~~~

Since C++11, list initialization also rejects many narrowing conversions:

~~~cpp
// int values[1]{3.14}; // ill-formed
~~~

This is particularly valuable in large literal tables. However, list initialization is a compile-time syntactic and semantic restriction, not a runtime range validator for arbitrary input arriving later.

### 3.1 String literals are arrays—including the terminator

The expression `"ABC"` is an lvalue array with type `const char[4]`; the fourth element is the terminating zero. Accordingly:

~~~cpp
static_assert(std::is_same_v<
    decltype("ABC"), const char (&)[4]>);
~~~

Here `decltype` reports a reference type because a literal is an array lvalue expression rather than an unparenthesized identifier naming a declared object.

Initializing a local character array from a literal creates a separate, mutable array:

~~~cpp
char text[] = "Hi"; // {'H', 'i', '\0'}
text[0] = 'B';      // valid
~~~

By contrast,

~~~cpp
const char* view = "Hi";
~~~

stores a pointer to the literal's static storage. `sizeof(text)` is three C++ bytes, while `sizeof(view)` is the size of a pointer object. The two declarations may look similar in code using `text[0]`, but ownership, storage duration, and type are different.

C++20 changed UTF-8 string literals to arrays of `const char8_t`:

~~~cpp
static_assert(std::is_same_v<
    decltype(u8"ABC"), const char8_t (&)[4]>);
~~~

Before C++20, UTF-8 string literals used `char` code units. The new type intentionally breaks some implicit conversions to `const char*`, making the encoding distinction visible instead of silently blending UTF-8 with narrow character APIs.

A zero-length *built-in* array is not standard C++:

~~~cpp
// int none[0]; // non-standard compiler extension
~~~

For an empty fixed-size value, `std::array<int, 0>` is legal. For borrowed or dynamically sized sequences, `std::span` and `std::vector` can represent emptiness without relying on GNU-style zero-length arrays or flexible-array-member extensions.

## 4. A multidimensional array is an array of arrays

The declaration

~~~cpp
int matrix[2][3]{};
~~~

defines an outer array of two elements, each of which is an array of three integers. Its type is `int[2][3]`. This nesting is not merely decorative notation: it controls conversion, pointer arithmetic, function parameters, and iteration.

The first array-to-pointer conversion of `matrix` produces a pointer to its first *row*, of type `int (*)[3]`, not `int*`:

~~~cpp
int (*row)[3] = matrix;
++row; // moves by one complete int[3] row
~~~

Taking the address of the entire matrix preserves another level:

~~~cpp
auto whole = &matrix; // int (*)[2][3]
~~~

The three types—`int*`, `int (*)[3]`, and `int (*)[2][3]`—can refer to related addresses but have different arithmetic domains and step sizes.

The storage of the nested rows is contiguous. Indeed, an ordinary `int[2][3]` contains six `int` element subobjects in nested row-major order, and its `sizeof` is `6 * sizeof(int)`. Yet contiguity alone is not permission to flatten every nested pointer operation:

~~~cpp
int* first = &matrix[0][0];
// first[3] is not a portable way to reach matrix[1][0].
~~~

The pointer `first` points into the inner `int[3]` array `matrix[0]`. Pointer arithmetic beyond that row's one-past position crosses an array-object boundary. The fact that the next row immediately follows in physical memory does not change the array whose elements the pointer addresses.

This is a valuable distinction between **physical layout** and **the object model that validates typed pointer arithmetic**. If a flat range is required, create flat backing storage and a multidimensional mapping rather than silently treating sibling row arrays as one larger scalar array.

### 4.1 Why `T**` is not a matrix

A `T**` points to a `T*` object. It describes at least one additional pointer indirection; it does not describe an array of fixed-width rows. A matrix declared `T[R][C]` decays to `T (*)[C]`, which increments by one row of `C` elements.

A dynamically allocated jagged structure may legitimately use an array of row pointers, perhaps with each row separately allocated and of different length. Its representation and ownership topology differ from a built-in rectangular array. Passing a built-in matrix to a function expecting `T**` is therefore a type and layout error, not a small conversion problem.

The conventional parameter syntax

~~~cpp
void process(int matrix[][3]);
~~~

is adjusted to

~~~cpp
void process(int (*matrix)[3]);
~~~

Only the outer bound is omitted: the inner extent remains part of the element type and determines the stride. A reference can preserve both:

~~~cpp
template<std::size_t R, std::size_t C>
void process(int (&matrix)[R][C]);
~~~

Nested range-for loops are often the clearest traversal:

~~~cpp
for (auto& row : matrix) {
    for (int& value : row) {
        // ...
    }
}
~~~

The outer loop yields a reference to `int[3]`; it does not need to flatten the object first.

### 4.2 From nested types to explicit multidimensional mappings

C++23 `std::mdspan` treats the storage and indexing interpretation as separate concerns. Given truly flat backing storage:

~~~cpp
std::array<int, 6> storage{1, 2, 3, 4, 5, 6};

std::mdspan view{storage.data(), 2, 3};
assert(view.extent(0) == 2);
assert(view.extent(1) == 3);
assert(view[1, 0] == 4);
~~~

Here `mdspan` does not construct a multidimensional allocation or take ownership. It describes how two indices map into the existing one-dimensional storage.

This enables interfaces for numerical computing, image processing, and accelerator data where “two dimensions” is a logical property and the physical memory arrangement may differ from a built-in nested array. To use such a view safely, however, the actual allocation must be large enough for every mapped offset, correctly aligned, and alive for the duration of access.

## 5. The extra responsibility introduced by dynamic arrays

A local array's storage duration follows its scope; a static array's lifetime follows static storage rules. Dynamic arrays separate the allocation's lifetime from the lexical scope that creates it, raising a separate ownership problem:

~~~cpp
int* data = new int[5]{};
delete[] data;
~~~

The array form of a new-expression must be paired with `delete[]`, not scalar `delete`. The static pointer type `int*` does not reveal whether the allocation originated from `new int` or `new int[n]`. That history is part of the allocation contract.

Implementations may store hidden information, sometimes called an *array cookie*, to manage element destruction. The standard does not require a particular cookie representation, size, or universal presence. Code should not infer deletion rules by inspecting memory preceding the pointer.

Aliases can make the problem less obvious:

~~~cpp
using Week = int[7];
int* p = new Week;
delete[] p;
~~~

The name in the new-expression denotes an array type even though the syntax no longer shows a bracketed bound. Array ownership is too important to depend on remembering such visual accidents.

### 5.1 Arrays of derived objects are not arrays of base objects

An ordinary derived-object pointer converts to a base-object pointer:

~~~cpp
struct Base {
    virtual ~Base() = default;
};
struct Derived : Base {
    int extra{};
};

Derived* d = nullptr;
Base* b = d;
~~~

This does not make an array covariant. Attempting to traverse `new Derived[n]` as an array of `Base` elements is invalid: incrementing a `Base*` advances by `sizeof(Base)`, while the actual elements are `Derived` objects with different stride and potentially different layout. Virtual dispatch and a virtual destructor do not repair the array-arithmetic and deallocation mismatch.

A heterogeneous polymorphic collection normally stores indirection, such as `std::vector<std::unique_ptr<Base>>`, not a contiguous derived array masquerading as base elements.

### 5.2 Array-aware ownership types

For exclusive dynamic-array ownership, use `std::unique_ptr<T[]>` or its standard factory:

~~~cpp
auto values = std::make_unique<int[]>(count);
~~~

The array specialization deletes with `delete[]` and provides indexed element access. It is an owning object, but it does not automatically carry the element count as a public size property. If allocation, size, and resizing semantics should travel together, `std::vector<T>` is often the clearer choice.

C++20 also added array support to `std::make_shared`:

~~~cpp
auto shared = std::make_shared<int[]>(count);
~~~

This is appropriate when the array truly has shared lifetime. It should not be chosen merely to avoid stating who is responsible for cleanup; control-block lifetime and shared ownership are real design commitments.

For scratch storage that will be completely written before any read, C++20's `std::make_unique_for_overwrite` can avoid value-initializing scalar elements:

~~~cpp
auto scratch = std::make_unique_for_overwrite<int[]>(count);
~~~

The optimization changes the programmer's obligation: every element must be initialized before it is used as a value. That should be a deliberately proved full-write invariant, not an assumption derived from an apparently successful benchmark.

Regardless of whether storage came from `new[]`, an owning smart pointer, `malloc`, or a C API, an ordinary `T*` data pointer still does not encode the dynamic length.

## 6. `std::array` restores fixed-size value semantics

The fixed-size container `std::array<T,N>` preserves the element count in its type while behaving like an ordinary value:

~~~cpp
std::array<int, 3> a{1, 2, 3};
std::array<int, 3> b{4, 5, 6};
a = b; // copies the elements
~~~

It supports assignment and comparison, provides iterators, and exposes `data()` and `size()` for interoperability. It is an aggregate and does not imply dynamic allocation.

Where `T[N]` has shape but lacks whole-array assignment, `std::array<T,N>` combines type-level extent with ordinary object semantics. Passing it by value copies or moves the contained elements according to their requirements; it does not silently decay to a pointer.

C++17 class template argument deduction makes common declarations concise:

~~~cpp
std::array numbers{1, 2, 3}; // std::array<int,3>
~~~

But string literals expose a surprising deduction consequence:

~~~cpp
std::array pointers{"abc"};
// deduces std::array<const char*,1>, not a character array
~~~

The literal participates in element-type deduction as an argument that can decay. C++20's `std::to_array` retains the original built-in array:

~~~cpp
constexpr auto text = std::to_array("abc");
// std::array<char,4>, including '\0'
~~~

This facility handles one-dimensional built-in arrays rather than automatically transforming every nested multidimensional array into nested `std::array` objects.

### 6.1 Empty arrays are real container values

`std::array<int,0>` is a valid type, unlike a built-in `int[0]`. It has size zero and `begin() == end()\). The value of `data()` in this case must not be used as a portable “null means empty” invariant. The proper questions are `empty()` and `size()`.

The absence of elements also means `front()`, `back()`, and `operator[](0)` have no valid element to access. An empty owner is useful because it obeys the container interface where possible, not because it fabricates a usable element zero.

## 7. `std::span`: a sequence without ownership

An API that merely reads or updates an existing contiguous range should not be forced to accept a particular owner:

~~~cpp
void process(std::span<const int> values);
~~~

A caller may supply a built-in array, `std::array`, `std::vector`, or another compatible contiguous source. A dynamic-extent span conceptually carries a pointer and a length; a static-extent span adds the bound to its type:

~~~cpp
int raw[4]{};
std::span<int,4> four{raw};
static_assert(decltype(four)::extent == 4);
~~~

A function can therefore express a fixed block requirement without requiring a built-in array specifically:

~~~cpp
void transform(std::span<float,16> samples);
~~~

This is often more compositional than `float (&)[16]`: the semantic requirement is sixteen contiguous floating elements, not one particular owner or declaration syntax.

Span copies remain views of the same storage. They are inexpensive to pass but never keep a vector allocation or local array alive. Returning a span into a local vector yields a dangling view. Resizing or reallocating the vector can invalidate a previously obtained span even while the vector itself remains alive.

The constness of the *descriptor* and the constness of the *elements* are also distinct:

~~~cpp
const std::span<int> mutable_elements = four;
std::span<const int> read_only_elements = four;
~~~

The first span object cannot itself be reassigned, but its `int` elements remain writable through the view. The second provides const access to elements, analogous to the difference between `int* const` and `const int*`.

Subview operations can preserve range semantics without making the caller rederive pointer arithmetic:

~~~cpp
std::span<int> all = four;
auto head = all.first(2);
auto tail = all.subspan(2);
~~~

When compile-time counts and offsets are combined with static extents, the result can retain a static bound.

C++20's `std::as_bytes` and `std::as_writable_bytes` create views of object representations as byte sequences, where constness permits. They are clearer than pretending that an arbitrary unrelated element type is a valid alias:

~~~cpp
std::array<std::uint32_t,2> words{};
auto bytes = std::as_bytes(std::span{words});
~~~

This gives non-owning byte access; it does not define network byte order or a portable serialization format for arbitrary objects.

Finally, C++20 ranges can inspect raw arrays directly through `std::ranges::begin`, `end`, `size`, and `data`, without making every generic algorithm begin by discarding extent information.

## 8. Unknown bounds, declarators, and array type rules

C++ declarators should be read from the identifier outward. An array of pointers, a pointer to an array, and a reference to an array are distinct:

~~~cpp
int values[4]{};
int* pointers[4]{};         // four pointer elements
int (*row_pointer)[4] = &values;
int (&reference)[4] = values;
~~~

An array's element type must be an allowed object type, complete where an array definition requires it. Arrays of references, functions, or `void` are ill-formed; an array of pointers to an incomplete type can be valid:

~~~cpp
struct Node;
Node* links[8]{}; // pointer objects are complete
// Node objects[8]; // Node incomplete here
// int& refs[8];    // no arrays of references
~~~

The unknown-bound array type remains an array, not a pointer. An external declaration such as `extern int table[];` can defer the actual bound. The first dimension of a multidimensional declaration may be unknown, as in `extern int pixels[][4];`, because each row element has a known `int[4]` type. An unknown inner bound, `int pixels[4][]`, cannot describe four variable-length rows in standard C++.

C++20 relaxed conversions from a pointer or reference to a known-bound array to the corresponding unknown-bound array form:

~~~cpp
int values[3]{1, 2, 3};
int (&unknown)[] = values;
int (*pointer_to_unknown)[] = &values;
~~~

Neither expression turns the array into an `int*`. The unknown-bound reference still cannot offer `sizeof(unknown)` based on its incomplete target type, and pointer arithmetic requiring the unknown row size cannot be justified merely because a particular original object had a known bound.

Cv-qualification of a raw array effectively applies to its elements:

~~~cpp
using Four = int[4];
using ReadOnly = const Four;
static_assert(std::is_same_v<ReadOnly, const int[4]>);
~~~

There is no separately const descriptor object analogous to `const std::span<int>` that would retain mutable access to raw array elements.

Built-in arrays cannot be function return types by value, but a function can return a reference or pointer to a live array:

~~~cpp
template<std::size_t N>
int (&identity(int (&a)[N]))[N] { return a; }

std::array<int,3> produce() { return {1, 2, 3}; }
~~~

The reference-returning version preserves shape and borrows the original object. It does not extend lifetime; returning a reference to a local array is as dangerous as returning a pointer to its first element. Returning `std::array` by value instead expresses an owning value result.

## 9. Elements have lifetimes, not just locations

Arrays of class objects make the object model unavoidable. Elements are initialized in increasing index order and destroyed in reverse construction order. If constructing one element throws, previously constructed elements are destroyed as part of unwinding:

~~~cpp
struct Resource {
    Resource();  // can throw
    ~Resource();
};

void example()
{
    Resource resources[4];
}
~~~

This means a properly constructed array of RAII objects can acquire and release resources safely without a separate manual cleanup loop. A large local array still consumes automatic storage, and its full object size matters for stack constraints.

Default initialization and value initialization remain distinct for scalar elements. An automatic `int values[4];` leaves values unwritten, while `int values[4]{};` initializes them to zero. Dynamic allocation has the analogous distinction between `new int[n]` and `new int[n]{}`. Static-storage arrays participate in static zero-initialization rules even without explicit braces.

Uninitialized storage is not a license to read objects merely because bytes occupy addresses:

~~~cpp
alignas(int) std::byte storage[sizeof(int)];
~~~

Modern C++ has specific implicit-lifetime rules and facilities for creating certain objects in suitable storage, but a cast alone is not a general proof that the required array or element lifetime has begun. For ordinary code, declare the object. For allocator, arena, or mapped-storage code, use the relevant construction and implicit-lifetime machinery deliberately, checking size, alignment, lifetime, and type-accessibility independently.

The same discipline applies when copying. `std::copy` performs element assignment through iterators and has overlap restrictions. `std::memcpy` copies bytes and requires nonoverlapping regions, while `std::memmove` is defined for overlapping byte ranges. `std::copy_backward` is useful for certain shifts to the right:

~~~cpp
std::array<int,5> a{0, 1, 2, 3, 4};
std::copy_backward(a.begin(), a.begin() + 4, a.end());
// {0, 0, 1, 2, 3}
~~~

Byte copying of trivially copyable representations is not the same as copy-constructing or assigning arbitrary class objects.

A dynamic `new T[n]` expression can fail during allocation or during construction of an element. A correct new-expression cleans up partially constructed arrays on failure. An owning container or smart pointer further ensures cleanup after the allocation successfully completes:

~~~cpp
auto strings = std::make_unique<std::string[]>(count);
~~~

The owner still must respect the allocation's extent and the element lifetimes; ownership does not manufacture bounds checking on an arbitrary index.

## 10. Algorithms, iterator categories, and borrowed ranges

Raw arrays participate directly in generic algorithms:

~~~cpp
int values[5]{9, 1, 7, 3, 5};
std::sort(std::begin(values), std::end(values)); // C++11
std::ranges::sort(values);                       // C++20
~~~

A vector conversion is unnecessary. Ranges APIs can accept the array itself as a range, preserving information that would disappear if the caller immediately converted it to a naked pointer.

However, being a range, being random-access, being contiguous, and being borrowed are separate properties. A random-access iterator supports efficient movement and indexing; a contiguous iterator additionally guarantees memory adjacency under its iterator model. A `std::deque` supplies random access but not contiguous storage, while `std::array` and `std::span` model contiguous ranges:

~~~cpp
static_assert(std::ranges::contiguous_range<std::array<int,4>>);
static_assert(std::ranges::contiguous_range<std::span<int>>);
static_assert(!std::ranges::contiguous_range<std::deque<int>>);
~~~

A borrowed range is about whether iterators can survive the *range wrapper's* destruction. The wrapper might be a span whose backing array lives elsewhere. Borrowing gives no guarantee that the backing allocation stays alive independently.

C++20's ranges library makes some dangling-iterator mistakes visible in the return type:

~~~cpp
using R = decltype(
    std::ranges::find(std::vector{1, 2, 3}, 2));
static_assert(std::is_same_v<R, std::ranges::dangling>);
~~~

Instead of returning an iterator into an already destroyed temporary vector, the algorithm can return a marker type. This is useful precisely because a classic raw-pointer interface cannot encode that distinction on its own.

Generic algorithms must also allow proxy references. `std::vector<bool>` stores packed bits and normally returns a proxy from its subscript operator, not a `bool&`. Similar issues arise with compressed sequences and specialized views. The iterator concept, element value type, and actual dereference expression are not interchangeable.

Finally, iterator, pointer, and reference invalidation follow the *owner's* contract. A vector's reallocation can invalidate every observer. Lists, deques, maps, unordered containers, and custom buffers have different rules. Neither a range adaptor nor a span can promise stability that its underlying owner does not have.

## 11. Contiguity is a storage property, not a wire format

Contiguous arrays are efficient partly because their elements are adjacent and their stride is predictable. For a built-in `T[N]`, adjacent elements are separated according to the complete object size `sizeof(T)`, which includes any padding needed by the element type.

That does not make a raw memory dump a portable serialization format. Element padding, type representation, endian conventions, object invariants, pointer values, floating-point representations, and ABI details can all differ between builds or platforms. A binary protocol should define its field widths and ordering deliberately instead of treating the layout of `struct Record records[N]` as globally meaningful.

Alignment applies to each element of an array, not just to the first. The implementation must arrange an array's stride so every element satisfies its type's alignment. Adding `alignas` to an element type may increase `sizeof(T)` and therefore expand a large array's memory footprint. Using over-alignment solely in the hope of better SIMD performance can easily worsen cache pressure.

### 11.1 Array of structures versus structure of arrays

A typical array-of-structures layout is

~~~cpp
struct Particle {
    float x, y, z;
    float mass;
};

std::vector<Particle> particles;
~~~

All fields of one particle are adjacent. This is attractive when each iteration uses most fields of one particle. But an algorithm that reads only `x` and `mass` may fetch unused `y` and `z` data, wasting bandwidth.

A structure-of-arrays layout stores each field in its own sequence:

~~~cpp
struct Particles {
    std::vector<float> x, y, z, mass;
};
~~~

It can improve locality and vectorization for field-wise operations but complicates ownership of synchronized lengths, record-oriented access, and insertion/removal. Neither layout is universally superior: cache-line size, working set, NUMA locality, vector width, hardware prefetching, and the actual access pattern govern the result.

C++ has no general standard `restrict` keyword equivalent for arbitrary function-pointer alias contracts. Vendor extensions and compiler analysis may exploit proven non-aliasing, but aliasing assumptions should be explicit and justified. C++26 data-parallel facilities in `<simd>` can improve expression of vector-oriented computations; they do not waive bounds, aliasing, or ownership constraints.

Multidimensional software must separately define *logical shape*, *physical pitch/strides*, and *storage ownership*. Conflating them makes padded images, accelerator buffers, and transposed views unnecessarily difficult.

## 12. The anatomy of `std::mdspan`

A multidimensional view can be described through three conceptual components: a collection of extents, a mapping from indices to storage offsets, and an accessor that performs element access. In `std::mdspan`, these are separate template-level policies, and the backing memory remains externally owned.

The extents type can mix static and dynamic dimensions:

~~~cpp
using Shape = std::extents<std::size_t, 3, std::dynamic_extent>;
Shape shape{5}; // three rows, five columns
~~~

A static dimension is carried by the type; a dynamic dimension is stored as view metadata. This resembles the distinction between static and dynamic `std::span` extents but generalizes it to several axes.

The mapping policy decides how indices become offsets. For a two-dimensional view, `std::layout_right` supplies the familiar rightmost-index-fastest arrangement:

~~~text
offset(i, j) = i * columns + j
~~~

while `std::layout_left` makes the leftmost index vary fastest:

~~~text
offset(i, j) = i + rows * j
~~~

These conventions are often called row-major and column-major for a two-axis view, respectively. The view does not rearrange the data; it merely interprets an existing allocation through the chosen mapping.

### 12.1 Padded rows and custom strides

Image buffers, GPU surfaces, and aligned numerical matrices frequently have rows with a physical pitch larger than the logical width. A tightly packed matrix might use stride `columns`; a padded image can require a larger row stride. `std::layout_stride` expresses these regular layouts explicitly:

~~~cpp
// Conceptual mapping for an image with physical pitch:
offset(row, col) = row * pitch + col;
~~~

The backing storage must cover the maximum offset produced by the mapping, including padding gaps where applicable. A view's logical element count is not always the size of the addressable storage span.

Accessors allow further policy customization, which can be useful for specialized memory or proxy access behavior. They are not magic portability adapters: a host C++ pointer still must designate memory accessible under the relevant execution and platform contract.

A subview can alter its layout properties. Extracting a rectangular window from a contiguous matrix may yield a strided view whose rows have gaps in the original allocation. Consequently, “the source is contiguous” does not guarantee that every slice has the same contiguous mapping.

C++26's `std::submdspan` makes this kind of slicing more composable, producing views with appropriately derived extents and mapping semantics. It remains non-owning. A subview is as vulnerable to backing storage invalidation as the original `mdspan`.

## 13. C++26 refinements and their real scope

New library versions make common contracts easier to state, but they do not retroactively make unbounded pointer access or uninitialized scalar reads safe.

C++26 adds `std::span::at`, enabling checked element access where the implementation supplies the facility:

~~~cpp
std::span<int> values = /* ... */;
int x = values.at(index); // checks the span bound
~~~

This differs from assuming an unchecked `operator[]` is automatically safe in every build. A checked view is valuable for boundary-facing code, although invalidation of the backing storage remains outside the index check's power.

Another C++26 convenience allows certain `std::span<const T>` constructions from initializer lists for immediate use:

~~~cpp
void inspect(std::span<const int>);

inspect({1, 2, 3}); // supported C++26 usage
~~~

The temporary initializer-list storage lasts through the full expression containing the call. Storing the span beyond that point would create a dangling view. The feature improves call ergonomics, not ownership.

C++26's `std::inplace_vector` occupies an interesting point between `std::array` and `std::vector`. It is an owning container with a dynamic logical size and a fixed, in-object capacity:

~~~cpp
std::inplace_vector<int, 16> values;
values.push_back(42);
~~~

Unlike `std::array<int,16>`, it need not contain sixteen live elements. Unlike ordinary `std::vector`, it does not grow via a dynamic allocation beyond its fixed capacity. This is valuable for embedded work, deterministic allocation constraints, and short fixed-capacity buffers, but the capacity limit is part of its API contract.

The same standard brings multidimensional slicing through `std::submdspan`, additional padded layout and aligned-accessor tools, and data-parallel library vocabulary in `<simd>`. These are different facilities addressing distinct layout and vectorization problems. Actual availability varies by standard-library implementation, so use relevant feature-test macros or direct compilation tests rather than assuming any C++26 compiler ships them all.

C++26 also refines terminology around indeterminate and erroneous values. None of those changes authorizes reading an unwritten ordinary scalar array element as though it had a meaningful application value. Initialization remains a precondition, regardless of whether storage is on the stack, in an overwrite-oriented allocation, or inside a bounded-capacity container.

## 14. Choosing an array abstraction

The appropriate abstraction follows from four questions: who owns the elements, whether the extent is fixed or dynamic, whether it must be part of the type, and whether access is logically multidimensional.

A built-in `T[N]` remains useful when the *language-level array type* matters: C compatibility, literal data, stack storage with simple layout, template shape preservation, and low-level object-model work. It is not a convenient general-purpose value object because assignment, return-by-value, and interface propagation behave differently from ordinary classes.

`std::array<T,N>` is preferable for an owning fixed-size value with ordinary copy, move, assignment, comparison, and iterator behavior. It also handles `N == 0` portably.

`std::span<T>` is the natural borrowed interface for contiguous one-dimensional access. It carries an extent and offers slicing, but it does not control destruction or reallocation of the source.

`std::vector<T>` combines dynamic size, capacity, ownership, and contiguous storage. Its resizing and reallocation semantics must be considered whenever observers or iterators escape.

`std::mdspan` is appropriate when non-owning indexing has multiple extents and physical layout or stride policy matters. It does not replace an owning multidimensional container; the owner is independent.

And `std::inplace_vector` adds a C++26 option for dynamic logical length with fixed in-object capacity, especially when allocation behavior matters.

The distinctions can be summarized without pretending the types are interchangeable:

| Abstraction | Owns elements | Shape carried by type | Main concern |
|---|---|---|---|
| `T[N]` | Yes, within its storage duration | Bounded array type | Decay and no whole-array assignment |
| `std::array<T,N>` | Yes | Fixed `N` | Value semantics, including zero extent |
| `std::span<T,E>` | No | Static `E` or dynamic extent | Backing lifetime and invalidation |
| `std::vector<T>` | Yes | Dynamic size not part of type | Reallocation and observer invalidation |
| `std::mdspan<...>` | No | Static/dynamic extents and mapping policies | Strides and backing allocation |
| `std::inplace_vector<T,N>` | Yes | Capacity `N`, dynamic size | Capacity bound |

A view is not a weak substitute for an owner, and an owner is not always a good parameter type for a function that only needs borrowed access. The design becomes clearer when those roles are kept separate.

## 15. The review discipline: shape, validity, and lifetime

Many array failures begin with an apparently harmless conversion. A parameter written `int a[10]` is actually a pointer. `auto p = array` loses the bound. An `int[R][C]` is treated as `int**` or traversed through a flat `int*` across rows. A `span` retains a size after its backing vector reallocates. A dynamic array's raw pointer is destroyed with the wrong delete form. These are not unrelated pitfalls; they result from confusing shape, contiguous representation, and ownership.

A useful code review first identifies the array object that validates each pointer operation. It then asks whether the extent survives the function/template boundary, whether the selected type can represent emptiness, whether initialization has made every accessed element a valid value, and whether an owner remains alive throughout every view's use.

For multidimensional data the review must additionally compare logical shape with physical mapping: what are the extents, strides, pitch, padding, and backing-storage span? A well-typed `mdspan` over insufficient storage is still invalid. Likewise, a contiguous native record array is not automatically a portable wire format.

This gives the subject one durable rule:

> An array has shape in its type; decay discards that shape. A view can restore an extent, but only an owner can establish the lifetime contract for the storage it views.

That rule explains why modern C++ has several array-like abstractions instead of one purportedly universal container.

## 16. Diagnostics, tooling, and extensions

Array errors range from statically detectable extent mistakes to runtime memory corruption. GCC commonly offers `-Wall -Wextra -Wpedantic -Warray-bounds -Wsizeof-array-argument -Wvla -Wstringop-overflow`; Clang offers similar array-bounds, `sizeof`-parameter, and variable-length-array diagnostics. MSVC's `/W4 /permissive- /analyze` provides both language diagnostics and static-analysis warnings. Coverage varies by compiler version and optimization level.

These warnings improve when the compiler still knows the bounded array type or static span extent. A raw pointer carries less information for the analysis to use.

AddressSanitizer (`-fsanitize=address` on GCC/Clang and `/fsanitize=address` on MSVC) detects many executed stack, heap, and global out-of-bounds accesses and use-after-free failures. UndefinedBehaviorSanitizer (`-fsanitize=undefined`) covers selected bad indexing and other invalid operations, while `-fsanitize=bounds` can add targeted checks where supported. libstdc++'s `-D_GLIBCXX_DEBUG` can diagnose invalid operations on many standard containers and iterators, but it does not turn a raw pointer into a bounds-checked object. Some hardened standard-library modes add contract checks to eligible container or span operations; their availability and exact failures are implementation-specific.

GNU C++ dialects may accept runtime-sized local arrays such as `int a[n]`, zero-length arrays, and flexible-array-member extensions that are not standard C++ array facilities. MSVC's `_countof` macro provides compile-time array size extraction; `std::size(array)` (C++17) is the standard alternative for compatible uses. GCC/Clang builtins such as `__builtin_object_size` and `__builtin_dynamic_object_size` can estimate available storage, while compiler-specific bounds attributes such as `counted_by` may aid static analysis of C-style layouts. None creates portable array bounds from an arbitrary pointer.

Choose `-std=c++20` or another ISO mode, with `-pedantic-errors` where useful, when testing portability rather than silently relying on `-std=gnu++20`. Vendor flags that suppress bounds checking or alter hardened container behavior should be treated as semantic policy, not casual speed toggles. Feature-test macros and targeted compilation tests are more reliable than assuming compiler brand implies complete C++23/26 library support.

Beyond the standard library, clang-tidy and cppcheck provide static checks, Microsoft GSL offers bounds-oriented vocabulary for older projects, and Boost.MultiArray can supply owning multidimensional structures where standard `mdspan` supplies only a view. Compiler Explorer helps inspect whether `std::array` and `std::span` optimize to the expected code. Fuzzing and sanitizer-enabled boundary tests should cover empty ranges, last-element and one-past indices, reallocation, stride padding, and view lifetime transitions.

The engineering priority is to preserve extent and ownership in the interface *before* relying on diagnostics to reconstruct them.

## 17. How the C++ array model evolved

**C++98/03** established the foundational C-compatible array model: fixed bounds in the type, array-to-pointer conversion, function parameter adjustment, unknown-bound declarations, nested arrays, and the inability to assign whole built-in arrays. The first bound of a dynamically allocated `new T[n]` array could already be computed at runtime; raw automatic variable-length arrays were not standard C++.

**C++11** introduced `std::array<T,N>`, `std::begin`/`std::end` for arrays, range-for, array-related traits, and list initialization with narrowing restrictions. It also improved movement and generic type reasoning, while retaining all existing decay rules.

**C++14** made exclusive array ownership easier with `std::make_unique<T[]>(n)`. That removed much explicit `new[]` spelling but did not make `unique_ptr<T[]>` a sized container.

**C++17** added `std::size`, `std::data`, and `std::empty` accessors, as well as class template argument deduction for `std::array`. Shape and string-literal element deduction still needed careful attention.

**C++20** added `std::span` and static/dynamic extent vocabulary, `std::to_array`, bounded/unbounded array traits, ranges and borrowed/dangling result machinery, byte views, `char8_t` UTF-8 literal element types, array-aware shared factories, and selected unknown-bound array conversions.

**C++23** introduced `std::mdspan`, separating multidimensional extents, layout mappings, accessors, and non-owning storage handles. Raw nested arrays still retained their original array-of-arrays semantics.

**C++26** provides additional array-adjacent APIs: `std::span::at`, suitable initializer-list construction for const spans, `std::inplace_vector`, `std::submdspan`, padded layout and aligned-accessor extensions, and data-parallel facilities. It also refines the vocabulary for uninitialized values without making unwritten ordinary scalar reads valid. Implementations may deliver these pieces on different schedules.

The progression is one of *more precise abstractions*, not of a revised meaning for `T[N]`.

## 18. Translating expectations from Java, Python, and C#

Java arrays and C# arrays generally carry runtime lengths and perform bounds checking. Python sequence objects also retain size and raise exceptions for invalid indexing. C++ raw arrays are different: a bound is available in the original type but can be discarded by a context-dependent conversion to a pointer. Unchecked indexing through a raw array or pointer can invoke undefined behavior rather than throwing an exception.

Managed references often participate in reachability-based lifetime management. A `std::span`, `std::string_view`, or `std::mdspan` does not own or pin the backing storage. A view can be perfectly formed and then become invalid because its owner dies or reallocates.

The most productive translation is not to emulate managed array syntax everywhere, but to choose the C++ abstraction that expresses the required lifetime and extent. Use `std::array<T,N>` for an owned fixed-size value, `std::vector<T>` for an owned dynamically sized sequence, and `std::span<T>` when a function needs borrowed contiguous access. For multidimensional borrowing, use `std::mdspan` with an explicitly established layout and backing lifetime.

This is a stronger model than treating every array-like value as a raw pointer and hoping the caller remembers its size.

## 19. Final perspective

The array is a good example of why C++ low-level abstractions must be understood at three levels. The **type model** says what array object exists and which extent is known. The **storage model** says where its elements reside, how they are aligned, and how multidimensional indices map to addresses. The **ownership model** says who keeps that storage alive and what operations can invalidate observers.

Most errors occur when a fact from one level is treated as proof of another. Contiguity does not justify arbitrary scalar pointer arithmetic across nested subarrays. A pointer does not carry an extent. A span does not establish lifetime. A compiler extension does not redefine ISO array semantics. A successful memory access on one execution does not prove the operation was within the required array object.

The practical questions are therefore straightforward: what owns the elements, where is the extent represented, which array object bounds pointer arithmetic, when does each element's lifetime begin, and does a view's physical mapping match the underlying allocation? When those answers are explicit, the choice among raw arrays, `std::array`, `std::span`, `std::vector`, and `std::mdspan` becomes much easier.

## Further reading

For exact rules, see the C++ standard and the references on [array declarations](https://en.cppreference.com/w/cpp/language/array), [`std::array`](https://en.cppreference.com/w/cpp/container/array), [`std::to_array`](https://en.cppreference.com/w/cpp/container/array/to_array), [`std::span`](https://en.cppreference.com/w/cpp/container/span), [`std::mdspan`](https://en.cppreference.com/w/cpp/container/mdspan), and [array type traits](https://en.cppreference.com/w/cpp/types/is_array). The design history of [P0122R7](https://wg21.link/p0122r7) and [P0009R18](https://wg21.link/p0009r18) helps explain why bounded one-dimensional and multidimensional views were standardized separately.

The [C++ Core Guidelines R.14](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#r14-avoid--parameters-prefer-span) recommends explicit bounds-aware interfaces, and [C.152](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c152-never-assign-a-pointer-to-an-array-of-derived-class-objects-to-a-pointer-to-its-base) documents the array-covariance hazard. For newer library facilities, consult [`std::inplace_vector`](https://en.cppreference.com/w/cpp/container/inplace_vector), [`std::span::at`](https://en.cppreference.com/w/cpp/container/span/at), and [C++ data-parallel types](https://en.cppreference.com/w/cpp/numeric/simd). The [ISO C++ FAQ on arrays](https://isocpp.org/wiki/faq/arrays) offers historical context, while [Compiler Explorer](https://godbolt.org/) is valuable for examining the actual code generation of raw arrays and their modern wrappers.

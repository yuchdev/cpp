# Chapter 05. Pointers and References: Objects, Lifetimes, and the Meaning of an Address

On most contemporary computers, a pointer to an ordinary object looks like a machine address. It can be printed as a hexadecimal number, copied between registers, and manipulated by instructions that appear indistinguishable from integer arithmetic. Yet a correct C++ program must know considerably more than the address of an object before it may access that object.

An address can still be present after the lifetime of the object at that address has ended. Suitably aligned storage may contain no object of the desired type. Two pointers may contain the same machine bits while representing different generations of a resource. Conversely, two smart pointers may hold different addresses while participating in exactly the same ownership relationship. These distinctions matter in everyday container use, in allocators, at C ABI boundaries, and especially in concurrent systems.

The governing principle is that **a pointer is a typed value participating in the C++ object, array, lifetime, and memory models; its machine representation is only one aspect of its meaning**.

The companion programs are [pointer facts](01_pointer_facts/pointer_facts.cpp), [pointer comparisons](02_compare_pointers/compare_pointers.cpp), [alignment](03_alignment/alignment.cpp), and [reference facts](04_references/reference_facts.cpp). The repository uses C++20 as its baseline and guards demonstrations that require C++23 facilities. Rather than treating every pointer rule as an isolated exception, we shall develop them from a few underlying contracts.

## 1. What a pointer actually denotes

Consider an ordinary declaration:

~~~cpp
int value = 42;
int* p = &value;
std::cout << *p;
~~~

The important facts are not that the machine has placed a number into `p`. The type `int*` determines the intended kind of object, the expression `&value` denotes a particular live `int`, and the dereference is permitted because a suitable object exists there with appropriate type and alignment. Its bounds and lifetime also constrain subsequent operations.

This model accommodates platforms that do not use simple, flat numeric addresses: segmented systems, multiple address spaces, capability pointers carrying bounds or permissions, pointer-authentication systems, and architectures with tagged pointers. It also matters on conventional x86-64, because the optimizer reasons about object lifetime, aliasing, and which array an expression belongs to—not merely the printed pointer value.

### 1.1 Different pointer categories have different representations

C++ distinguishes pointers to objects, pointers to functions, pointers to data members, and pointers to member functions. Their declaration syntax makes the categories visible:

~~~cpp
struct Widget {
    int field;
    void action();
};

int* object_pointer = nullptr;
void (*function_pointer)() = nullptr;
int Widget::* data_member = &Widget::field;
void (Widget::* member_function)() = &Widget::action;
~~~

Neither a function pointer nor a pointer-to-member is necessarily interchangeable with a `void*`. A member pointer can require an offset, an adjustment to `this`, and sometimes additional dispatch information. Multiple inheritance is an especially clear illustration: a member of a base subobject may need an adjusted object pointer when applied to a derived object.

No portable equality is required among

~~~cpp
sizeof(void*)
sizeof(void (*)())
sizeof(int Widget::*)
sizeof(void (Widget::*)())
~~~

and treating a member pointer as a raw machine address loses the very semantics it was designed to carry.

A pointer to an object type can be converted to `void*` and back to the appropriate original type. This supports many C interfaces:

~~~cpp
int n = 7;
void* erased = &n;
int* restored = static_cast<int*>(erased);
~~~

But `void` has no object size and no objects of type `void`; dereferencing or incrementing a `void*` is not valid standard C++. GNU modes sometimes accept pointer arithmetic on `void*` as an extension. A function pointer is a separate matter: converting it to `void*` is not a general portable implicit conversion, even though some platform ABIs support such interchanges.

### 1.2 Null is a value, not a portable bit pattern

C++11 supplied the null pointer literal `nullptr`, whose type is `std::nullptr_t`. It is not an integral type:

~~~cpp
static_assert(!std::is_integral_v<std::nullptr_t>);

void f(int);
void f(int*);

f(0);       // f(int)
f(nullptr); // f(int*)
~~~

A null pointer value can be formed for suitable object pointers, function pointers, and pointers-to-members. The literal `nullptr` is a prvalue, so `&nullptr` is invalid; a named `std::nullptr_t` object, however, can have its address taken.

The language specifies what a null pointer *means*, not that all its storage bits must be zero. On mainstream systems null object pointers commonly have an all-zero representation, but a general serializer cannot derive that representation from the C++ rules. In particular, a block of zero-filled bytes is not automatically a collection of valid null pointers of every possible pointer category.

Null and dangling are also fundamentally different. A null pointer represents the absence of a pointee. A dangling pointer may still contain a value associated with a former object, but that object's lifetime has ended:

~~~cpp
int* dangling = nullptr;
{
    int local = 42;
    dangling = &local;
} // local no longer exists
~~~

The compiler does not automatically reset observers when their pointee dies. Although assigning `nullptr` after invalidation can improve local clarity, it does nothing for other aliases and is not a lifetime-management strategy.

## 2. Pointer arithmetic belongs to arrays, not address space

Pointer addition is defined with reference to an array object. Given

~~~cpp
int values[10]{};
int* p = &values[3];
int* q = p + 4; // points to values[7]
~~~

the increment is measured in elements of `int`, not in arbitrarily addressable bytes. A conventional implementation may implement that by adding `4 * sizeof(int)` to an address register, but that is how the machine realizes an already-valid operation, not the rule that makes it valid.

For limited pointer arithmetic, a non-array complete object is treated as a one-element array. Thus a pointer to `int x` may be advanced to `&x + 1`, even though there is no second element. That is a **one-past pointer**: it may be formed and used as an endpoint, but must not be dereferenced.

~~~cpp
int a[4]{};
int* last = a + 4; // valid endpoint
// int x = *last;  // undefined behavior
~~~

The half-open convention `[first,last)` owes much of its usefulness to this distinction: the endpoint is easy to compute without being a valid element to read.

The bounds restriction applies to *forming* pointers, not just to dereferencing them. An expression that computes `a - 1` is already outside the permitted array arithmetic domain; the plan to increment it again before dereferencing does not make the intermediate operation valid.

Subtraction follows the same object constraint:

~~~cpp
int a[100]{};
auto distance = &a[80] - &a[10]; // std::ptrdiff_t, value 70
~~~

Subtracting pointers into unrelated objects is not a portable way to measure their numeric address separation; the language generally makes that subtraction undefined. The signed result type `std::ptrdiff_t` allows negative differences. `std::size_t`, by contrast, is unsigned and expresses object sizes. They need not cover identical positive ranges; for an extremely large array, even a mathematically meaningful difference might not fit in `ptrdiff_t`, which is itself an additional limitation.

These rules explain why a pointer is not an integer equipped with automatic element scaling. The permitted relationship to a particular array is part of the operation's contract.

## 3. Arrays, spans, and the extent of a sequence

An array is an object with a type and an extent, not a disguised pointer:

~~~cpp
int a[10]{};
auto p = a;  // int*, array-to-pointer conversion
auto q = &a; // int (*)[10], pointer to whole array
~~~

Adding one to `p` moves by one `int`; adding one to `q` moves by one entire array object (to its one-past position). Similarly, `sizeof(a)` measures the whole array, while `sizeof(p)` measures a pointer.

The array-to-pointer conversion does not occur in every context. `sizeof` and address-of are common counterexamples, and the declaration of a function parameter is another special case:

~~~cpp
void process(int values[100]); // adjusted to int*
void process(int* values);      // same parameter type
~~~

The written `100` does not impose an argument extent. If the contract needs a contiguous sequence of known length, C++20's `std::span` is much more expressive:

~~~cpp
void increment(std::span<int> values)
{
    for (int& value : values)
        ++value;
}
~~~

A caller can supply an array, `std::array`, or `std::vector`. A dynamic-extent span conceptually holds an address and a count, encoding the range that a raw `T*` alone cannot recover.

Still, a span is **non-owning**. It does not make invalid pointer arithmetic valid, guard against an external owner being destroyed, or keep a vector allocation alive after reallocation:

~~~cpp
std::span<int> broken()
{
    std::vector<int> data{1, 2, 3};
    return std::span<int>{data}; // dangling on return
}
~~~

For character sequences, `std::string_view` has the same lifetime class of hazard. A view is cheap precisely because it does not own its storage. In a C ABI, a pointer-plus-length pair may be the correct public representation:

~~~cpp
void consume(const std::byte* data, std::size_t size);
~~~

A C++ implementation can wrap it in a span after validating the API's pointer/length preconditions; constructing a span does not retroactively prove that the backing region is valid.

### 3.1 Contiguous iterators need not be raw pointers

Modern library code also distinguishes a contiguous iterator from `T*`. C++20's `std::contiguous_iterator` concept promises contiguity but does not require pointer identity for the iterator's type.

The utility `std::to_address` obtains the underlying address from a suitable pointer or contiguous iterator without spelling `&*it`. That old idiom conceptually dereferences the iterator, cannot be used on a one-past iterator, and is awkward for proxy and fancy pointer abstractions. `pointer_traits` participates in address extraction for pointer-like types.

The distinction becomes important for allocators and specialized storage—shared memory, persistent memory, GPU buffers, segmented storage, or customized heaps—where a pointer-like value may carry more information than `T*`. Generic code should not assume that an allocator's pointer type is always a raw pointer or that `span::iterator` is necessarily spelled `T*`.

## 4. Comparing pointers: order, address, and ownership

Within one array, relative ordering has its expected meaning:

~~~cpp
int a[10]{};
assert(&a[2] < &a[7]);
assert(&a[9] < a + 10);
~~~

For unrelated objects, built-in pointer relational comparisons do not generally supply a portable numeric address order. Where no specified object relationship determines the result, the ordering can be unspecified; that is different from claiming every such comparison necessarily invokes undefined behavior.

The library supplies `std::less` (and C++20 ranges comparison facilities) for a strict total pointer ordering suitable for ordered containers and pointer-indexed structures:

~~~cpp
std::less<>{}(px, py);
~~~

The total order is useful as an ordering contract; it does not promise to expose ascending numerical virtual addresses. C++20's pointer `<=>` likewise does not transform arbitrary pointers into ordinary integers: three-way syntax preserves the underlying pointer comparison model.

Ownership introduces another dimension. Consider two aliasing shared pointers:

~~~cpp
struct Record { int first, second; };

auto owner = std::make_shared<Record>();
std::shared_ptr<int> p(owner, &owner->first);
std::shared_ptr<int> q(owner, &owner->second);
~~~

Their `get()` results differ, yet they share the same ownership control block and keep the same `Record` alive. `std::owner_less` exists to compare ownership relationships. Address ordering, pointer equality, and ownership identity answer different questions.

## 5. Pointer-to-integer conversions and the temptation of pointer tagging

`<cstdint>` can provide `std::intptr_t` and `std::uintptr_t` when the implementation supports the requisite integer representation of pointers. Converting a pointer to a sufficiently capable integer type and back under the specified conditions can preserve the original pointer value. This is useful for certain debugging utilities, diagnostics, runtime implementations, and platform-defined address APIs:

~~~cpp
auto bits = reinterpret_cast<std::uintptr_t>(p);
~~~

It does not follow that `bits` is a portable physical or virtual address, that arbitrary arithmetic on it preserves pointer validity, or that an arbitrary integer can be fabricated into a usable pointer:

~~~cpp
// An integer does not manufacture a live object.
auto imaginary = reinterpret_cast<int*>(0x12345678);
~~~

No integer conversion begins object lifetime, establishes memory mapping, grants access permission, or fixes alignment. Moreover, pointer provenance—the relationship of a pointer to an allocation or object—is relevant to optimizer reasoning even where the exact formal model continues to evolve. Keeping pointers as pointers is usually safer than repeatedly converting them to integers for ordinary traversal.

An address is especially unsuitable as persistent identity. Process restarts, address-space layout randomization, new allocations, allocator reuse, different processes, or different machines can all invalidate its meaning. Stable serialization should use IDs, handles, or offsets within a deliberately defined format.

Pointer tagging illustrates the performance appeal and the risks. Alignment may leave unused low address bits on a particular ABI, inviting the programmer to store a small tag there. This is a valid specialized technique only with a platform contract addressing pointer representation, authentication, capability systems, memory tagging, sanitizers, garbage collectors, and compiler assumptions. A low-bit pattern that “works on my CPU” is not a general C++ pointer abstraction.

## 6. Alignment, layout, and allocation

Every complete object type has an alignment requirement expressed by `alignof(T)`. To access a live object as `T`, the storage must satisfy that requirement as well as its size and type-access conditions.

Structures commonly contain padding:

~~~cpp
struct S {
    char c;
    int i;
    double d;
};
~~~

There may be padding between members and after the last member. The trailing space ensures that successive elements of an array of `S` can each satisfy `S`'s alignment. Thus the sum of member sizes is not a reliable prediction of `sizeof(S)`. Class alignment must be sufficient for its members and can also be strengthened; it is not universally useful to define it as “exactly the maximum member alignment.”

The `alignas` specifier permits an explicit stronger requirement:

~~~cpp
struct alignas(64) CacheLine {
    std::byte bytes[64];
};
~~~

This can support SIMD, DMA, specialized allocators, or efforts to reduce false sharing. But a C++ alignment request of 64 does not establish that 64 is the machine's actual cache-line size or that each object will occupy an isolated cache line under every memory topology.

A subtle layout mistake is to align *each member* rather than the aggregate:

~~~cpp
struct alignas(16) Vec4 {
    float x, y, z, w;
};

struct Wasteful {
    alignas(16) float x, y, z, w;
};
~~~

The first requests a 16-byte-aligned group; the second can force substantial padding between individual scalars. Alignment is a layout constraint, not a universal “make this code vectorize faster” switch.

C++17 strengthened ordinary `new` support for over-aligned types via aligned allocation machinery:

~~~cpp
struct alignas(64) X { int value; };
auto owner = std::make_unique<X>();
~~~

A conforming implementation is expected to arrange suitable alignment when required. But `malloc` and legacy custom allocators should not be assumed to satisfy every extended-alignment request. When stronger alignment is necessary, use aligned `operator new`, a matching allocator, `std::aligned_alloc` where its restrictions are satisfied, or the appropriate platform API. The deallocation mechanism must match the allocation mechanism.

C++20's `std::assume_aligned<N>(p)` serves a completely different purpose. It promises to the implementation that `p` already has the specified alignment. It does not allocate, check, or adjust the pointer. If the promise is false, the caller has violated the precondition and may enable invalid optimizer assumptions. This makes `assume_aligned` powerful only after the allocation or layout invariant has been established independently.

## 7. Storage is not an object

One of the most important distinctions in low-level C++ is between the existence of storage and the lifetime of an object.

~~~cpp
alignas(T) std::byte storage[sizeof(T)];
~~~

This declaration gives a region with enough space and alignment for `T`. It does not universally guarantee that a live `T` object exists in the region. For arbitrary non-implicit-lifetime class types, construction is required.

C++20's `std::construct_at` makes explicit object construction in existing storage convenient:

~~~cpp
T* p = std::construct_at(
    reinterpret_cast<T*>(storage),
    constructor_arguments...);

// ... use live T ...
std::destroy_at(p);
~~~

This code assumes an appropriately aligned region, correct construction arguments, and a lifetime/representation model for that storage. It is not an invitation to cast any available bytes to any class type.

After destruction, the numeric address may be unchanged and the bytes may remain physically present, but the object is no longer alive:

~~~cpp
std::destroy_at(p);
// p->member; // no longer a valid use of the destroyed T
~~~

Placement `new` is the traditional mechanism for creating a new object in existing storage:

~~~cpp
T* p = ::new (storage) T(args...);
~~~

Containers, allocators, object pools, optional-like types, variants, and arenas rely on carefully controlled lifetime changes. Reusing storage may begin a new object lifetime even though an old pointer still contains the same address. Whether an old pointer/reference can transparently refer to the new object depends on the relevant transparent-replacement rules.

C++17's `std::launder` exists for specific cases involving object replacement and obtaining a suitable pointer when transparent replacement does not apply. It does *not* begin lifetime, legalize arbitrary aliasing, repair misalignment, or make a bad cast safe. Its presence should provoke a careful explanation of exactly which lifetime rule requires it.

C++23's `std::start_lifetime_as` and related array facility make a different operation explicit for suitable implicit-lifetime types: begin an object lifetime in suitable existing storage under the facility's rules. This helps specialized binary I/O, memory-mapped layouts, shared memory, and serialization infrastructure. It still does not guarantee that arbitrary bytes are a valid object representation or that the resulting object's semantic invariants, pointers, or endian-sensitive fields are meaningful.

The recurring test is therefore: **is storage present, large enough, aligned, occupied by a suitable live object, and accessible through the selected type for the intended operation?** Each condition is independent.

## 8. Aliasing, bit interpretation, and representation

A `reinterpret_cast` changes the type of a pointer expression. It does not suspend the C++ type-accessibility rules:

~~~cpp
float f = 1.0f;
int* p = reinterpret_cast<int*>(&f);
// std::cout << *p; // not a permitted general type-punning read
~~~

Compilers rely on alias analysis when optimizing. An incompatible dereference can appear to work at `-O0` and fail in optimized builds—not because optimization is inherently unsafe, but because the source operation was outside the required contract.

C++ permits viewing an object's representation through appropriate character or byte types. C++20 also provides a direct high-level view:

~~~cpp
std::span<const T> object{&value, 1};
auto bytes = std::as_bytes(object);
~~~

For equal-size trivially copyable representations, `std::bit_cast` describes a *copy of object representation*, not an aliasing read:

~~~cpp
std::uint32_t bits = std::bit_cast<std::uint32_t>(f);
~~~

This is fundamentally different from `static_cast<std::uint32_t>(f)`, which performs numeric conversion of the floating value. Before C++20, a suitably sized `std::memcpy` was and remains a valid way to copy object representation; optimizing compilers usually recognize this pattern and need not emit a literal memory-copy routine.

C and C++ have differing details around unions and type punning. Vendor extensions, kernel idioms, or C-language folklore should not be imported silently into standard C++ code. If the intent is to copy bits, use a representation-copy facility; if the intent is a numerical value, use numeric conversion; if the intent is accessing a live object, use an access path allowed for its type.

### 8.1 Endianness is not pointer direction

If the integer `0x01020304` is stored in four eight-bit bytes, the lowest-address byte commonly contains `0x04` on a little-endian machine and `0x01` on a big-endian one. This concerns the *representation* of a multibyte value. It does not make pointers increment backward or change the language's array-ordering rules.

`std::endian::native` (C++20) makes byte order observable, and `std::byteswap` (C++23) can help transform integer encodings. A portable wire format must nevertheless specify widths, byte order, signed and floating representation where relevant, padding policy, and versioning. Dumping the native bytes of an arbitrary C++ object is not a portable serializer.

A related error is to compare objects by their bytes:

~~~cpp
bool equal(const S& a, const S& b)
{
    return std::memcmp(&a, &b, sizeof(S)) == 0;
}
~~~

Padding bytes, multiple representations of equivalent values, signed floating zero, NaNs, pointer values, and domain invariants can all invalidate the intended equivalence. `std::has_unique_object_representations_v<T>` can help establish certain representation properties, but even then domain equality should not be replaced mechanically by bytewise equality.

Bytewise initialization with `std::memset` is similarly unsafe as a general method of initializing nontrivial or representation-sensitive C++ objects. For C++ storage, distinguish object representation equality, language-level value equality, and application-level semantic equality.

A C++ byte is also not necessarily an eight-bit octet. Pointer arithmetic on `char*`, `unsigned char*`, or `std::byte*` advances by one *C++ byte*, whose bit count is `CHAR_BIT`. Formats defined in octets should state that environmental requirement explicitly. `std::byte` deliberately disallows ordinary arithmetic and provides `std::to_integer` for cases where numeric interpretation is intended.

## 9. Ownership is a different question from access

Two declarations of `Widget*` can look identical even when one points to an owned heap object and the other is a temporary observer:

~~~cpp
Widget* a;
Widget* b;
~~~

Nothing in raw-pointer syntax tells us which party must destroy the object. This ambiguity is why resource-management design deserves types of its own.

`std::unique_ptr<T>` represents exclusive ownership with move-only transfer and destruction through a configurable deleter. The deleter can carry state, and the pointer representation used by the owner can be customizable through deleter-related pointer types. This is more than a bare pointer with automatic `delete`.

`std::shared_ptr<T>` represents shared lifetime ownership via a control block. It distinguishes the stored pointer returned by `get()` from the owner control block, as the earlier aliasing example showed. A `std::weak_ptr<T>` refers into a shared ownership graph without extending strong ownership. It is especially important for cycles: if A strongly owns B and B strongly owns A, reference counts may never reach zero.

Raw pointers remain useful for non-owning optional access, C interfaces, intrusive data structures, hardware and runtime implementations, iteration, and explicit pointer arithmetic. “Modern C++” does not require banishing `T*`; it requires making ownership and borrowing clear.

### 9.1 The size of ownership wrappers is an implementation property

A stateless-deleter `unique_ptr<T>` is commonly the size of one machine pointer because implementations can optimize away storage for an empty deleter. A stateful deleter may increase its size, and its exact layout is not standardized. Similarly, `shared_ptr` commonly occupies two pointer-sized words but that is not a normative ABI guarantee.

C++20's `[[no_unique_address]]` lets an empty policy member potentially share storage with another member:

~~~cpp
template<class T, class Policy>
struct pointer_wrapper {
    T* p;
    [[no_unique_address]] Policy policy;
};
~~~

An empty policy can therefore add little or no size, but a specific layout requires measurement and an ABI contract. This matters in memory-dense iterators, allocators, pointer wrappers, and custom deleters.

### 9.2 Allocation and construction are distinct stages

An ordinary `new T` expression obtains storage, constructs/initializes an object, and yields a pointer; a matching `delete` ends the lifetime and deallocates storage. Arrays use `new T[n]` and `delete[]`; mixing the scalar and array forms is undefined behavior, even for trivially destructible element types.

A zero-length array allocation is allowed:

~~~cpp
T* p = new T[0];
delete[] p;
~~~

There are no constructed elements. The result must be handled according to the array-new contract; do not infer from it a usable element address or a fixed numeric representation. Deleting a null pointer does nothing, so an explicit `if (p)` guard before `delete p` is unnecessary.

Allocation failure and constructor failure are different. A throwing allocation function normally reports failure with `std::bad_alloc`; if construction throws after allocation succeeds, the new-expression invokes the matching deallocation function for that storage. Manually spelling the corresponding operations requires equally careful exception safety.

This is one reason RAII is foundational:

~~~cpp
auto owner = std::make_unique<T>();
may_throw(); // owner is cleaned up if the call throws
~~~

A raw `new` followed by a later `delete` after a throwing call can leak if the intervening operation throws. The same RAII principle governs files, sockets, locks, mapped memory, database transactions, and device resources.

### 9.3 C APIs that write a pointer

C interfaces commonly use a `T**` output parameter:

~~~cpp
resource* raw = nullptr;
int create_resource(resource** out);
~~~

A smart pointer cannot simply expose its internal pointer for arbitrary external overwriting, because that would bypass its ownership invariants. Handwritten adapters must carefully handle error paths, old contents, and resource cleanup.

C++23 provides `std::out_ptr` and `std::inout_ptr` adapters for appropriate smart-pointer/C-API interoperability. They represent a specific ownership transition rather than granting the C function unrestricted access to a smart pointer's implementation. Correctness still depends on understanding whether the C API allocates, replaces, releases, or retains the object on each success and failure path.

## 10. References: aliases, not reseatable pointers

A reference is often represented using an address at the ABI level, but its language semantics differ from those of a pointer:

~~~cpp
int x = 1;
int y = 2;
int& r = x;
r = y; // assigns x = 2; does not rebind r
~~~

A reference does not have an ordinary null value. Attempting to manufacture one by dereferencing a null pointer does not provide a legitimate optional-reference abstraction. If absence matters, use a pointer or a handle such as `std::optional<std::reference_wrapper<T>>`.

References are not ordinary objects whose addresses or sizes can be manipulated independently. Arrays of references and pointers to references are ill-formed; `sizeof(r)` gives the size associated with the referred-to type, not a possible hidden pointer in the implementation.

C++ permits overloading unary `operator&` for class types, so `&object` might call user code rather than return the built-in object address. Generic library code that needs the actual address uses `std::addressof(object)`. This matters in allocators, placement construction, intrusive containers, and generic serialization. Modern constant evaluation made the utility easier to use in some contexts, but its purpose is the older fundamental need to bypass an overloaded address-of operator.

### 10.1 Binding a temporary does not always extend its lifetime

A temporary directly bound to a suitable local const lvalue reference or rvalue reference generally has its lifetime extended to that reference's lifetime:

~~~cpp
const std::string& text = std::string("hello");
std::string&& moved_view = std::string("world");
~~~

The rule has important contextual exceptions. Returning a reference to a temporary from a function does not extend the temporary into the caller:

~~~cpp
const std::string& broken()
{
    return std::string("temporary"); // dangles
}
~~~

A temporary bound to a reference parameter in a function call survives until the end of that full expression, not indefinitely. If the callee stores a pointer or reference to the parameter for later, that stored observer can dangle.

References stored as class members deserve equally careful treatment:

~~~cpp
struct View {
    const std::string& text;
};
~~~

This class owns no string. Rules for lifetime extension in member initialization depend on the precise initialization context, and the familiar slogan about const references cannot be applied without qualification. An aggregate initialized with braces can have different lifetime consequences from one initialized through parentheses in modern C++; dynamically escaping wrappers must be reviewed regardless.

### 10.2 Named rvalue references are lvalue expressions

An rvalue reference is a type, but the expression formed by using its name is an lvalue:

~~~cpp
std::string&& ref = make_string();
std::string copy = ref;            // ref is an lvalue expression
std::string moved = std::move(ref); // permits move construction
~~~

`std::move` does not itself move resources; it is an appropriate cast that makes the expression eligible for rvalue treatment. The selected constructor or assignment operator performs any actual transfer. A moved-from object remains alive and, for standard-library types under their documented rules, generally valid but in an unspecified state unless stronger guarantees are given. “Moved-from” does not mean “destroyed.”

Reference collapsing gives generic programming its forwarding machinery:

~~~text
T&  &  -> T&
T&  && -> T&
T&& &  -> T&
T&& && -> T&&
~~~

Whenever an lvalue-reference layer appears, the collapsed type is an lvalue reference. Thus a deduced `T&&` in the correct context can be a forwarding reference:

~~~cpp
template<class T>
void wrapper(T&& value)
{
    target(std::forward<T>(value));
}
~~~

For an lvalue argument, `T` deduces as an lvalue reference and collapsing preserves that category. For an rvalue argument, `T` is ordinarily a non-reference type and the parameter becomes an rvalue reference. Since `value` has a name, `std::forward<T>` is necessary to restore the caller's category; `std::move(value)` would treat lvalue arguments as disposable rvalues too.

A deduced `auto&&` behaves similarly in appropriate contexts and is common in generic loops and range machinery.

### 10.3 `decltype`, wrappers, and proxy references

An unparenthesized id-expression in `decltype` has a special rule reporting the entity's declared type. Parenthesizing it produces an ordinary lvalue expression:

~~~cpp
int x = 0;
static_assert(std::same_as<decltype(x), int>);
static_assert(std::same_as<decltype((x)), int&>);
~~~

The same distinction appears in return-type deduction:

~~~cpp
int global{};

auto copied() { return (global); }       // returns int
decltype(auto) borrowed() { return (global); } // returns int&
~~~

Preserving references is powerful but can make dangling returns equally easy to write.

`std::reference_wrapper<T>` is a different mechanism: an object type holding reference-like semantics that can be stored in a container and reseated by assigning another wrapper/target. A genuine language reference cannot be reseated or held directly as a vector element. The wrapper itself does not extend the referenced object's lifetime.

Not every iterator dereference returns `T&`. `std::vector<bool>` normally returns a proxy object representing a packed bit, and similar proxies occur in compressed containers, SIMD abstractions, database accessors, GPU memory interfaces, and transform/zip views. Copying a proxy can retain access to underlying storage rather than copying the apparent value. Modern iterator concepts account for this; generic code must not assume that `decltype(*it)` is always `T&`.

## 11. Ranges and the detection of dangling access

Returning an iterator into a destroyed temporary container is an old and dangerous pattern. C++20 ranges make some of these errors visible in types:

~~~cpp
using R = decltype(std::ranges::find(
    std::vector<int>{1, 2, 3}, 2));

static_assert(std::is_same_v<R, std::ranges::dangling>);
~~~

For an appropriate non-borrowed temporary range, the ranges algorithm can return a `std::ranges::dangling` marker instead of a syntactically usable iterator.

The `std::ranges::borrowed_range` concept means roughly that iterators can remain usable independently of the lifetime of the range *wrapper*. A `std::span` is a typical borrowed range: destroying the span object need not destroy the array its iterators refer to. But if the underlying array dies, those iterators still dangle. Borrowed does not mean owning.

C++23's `std::forward_like<Like>(value)` supplies a way to propagate constness and value category modeled on another type. It is particularly useful in generic accessors that return a subobject with the appropriate reference category:

~~~cpp
int x = 42;
static_assert(std::same_as<
    decltype(std::forward_like<const int&>(x)), const int&>);
static_assert(std::same_as<
    decltype(std::forward_like<int&&>(x)), int&&>);
~~~

Like `std::move`, it is a cast-like facility, not a resource transfer.

C++23 also introduced `std::reference_constructs_from_temporary` and `std::reference_converts_from_temporary`, allowing generic wrappers to reject dangerous reference bindings to temporaries where the traits detect them. This is valuable because ordinary source syntax can hide temporary materialization inside generic conversions.

Explicit object parameters—often called *deducing this*—supply a complementary C++23 tool. Instead of four nearly identical overloads for mutable/const lvalues and rvalues,

~~~cpp
T& value() &;
const T& value() const &;
T&& value() &&;
const T&& value() const &&;
~~~

a member function can deduce the cv/ref category of its object and use `forward_like` where appropriate. This improves implementation reuse, but does not absolve an accessor from preventing a returned reference from outliving its containing object.

## 12. Invalidation, identity, and concurrency

Container invalidation is a major source of dangling pointers in otherwise ordinary application code:

~~~cpp
std::vector<int> v{1, 2, 3};
int* observed = &v[0];
v.push_back(4); // may reallocate and invalidate observed
~~~

`reserve` can avoid some reallocations while size remains within capacity, but is not a permanent guarantee against erasure, destruction, assignment, later growth, and other invalidating operations. Different containers have different iterator/reference/pointer invalidation contracts; rules for `vector` must not be copied blindly to `deque`, `list`, `map`, `unordered_map`, flat containers, or custom small-vector types.

The small-string optimization and custom small-vector storage complicate address stability further. A string's `data()` pointer may become invalid when the object grows from inline to allocated storage even though the string object itself has not moved.

Allocator reuse creates a deeper issue. After

~~~cpp
T* old = new T;
delete old;
T* fresh = new T;
~~~

the second allocation may reuse the same numeric address, but it creates a different object with a distinct lifetime. A cache or registry using addresses as persistent object IDs cannot infer identity from the printed number alone.

### 12.1 The ABA problem

In a lock-free structure, one thread can read pointer value A while another thread removes A, changes the structure, and later inserts an object that happens to occupy the same address. A compare-exchange can then observe A again and falsely conclude that the data structure has not changed. This is the ABA problem.

Versioned or tagged pointers, hazard pointers, epoch-based reclamation, read-copy-update techniques, reference counting, or other specialized protocols may address aspects of the problem. None follows automatically from having atomic pointer values.

`std::atomic<T*>` allows atomic publication, loads, comparisons, and supported pointer operations with appropriate memory ordering. That does not keep the pointed-to object alive:

~~~text
Thread A loads pointer p
Thread B unlinks and destroys the object
Thread A dereferences p
~~~

The load may be perfectly atomic while the dereference is a use-after-free. Concurrency correctness therefore requires at least two distinct arguments: the pointer value is synchronized, and the pointee cannot be reclaimed during a permitted read. C++20 atomic shared-pointer facilities and C++26 standardized hazard-pointer/RCU-related reclamation tools address particular ownership and reclamation scenarios, but they do not turn arbitrary raw-pointer algorithms into safe ones.

A `volatile T*` is not an alternative to this reasoning. `volatile` can express observable accesses as required by particular implementation/platform contracts, including some memory-mapped I/O, but does not provide C++ atomicity, happens-before relationships, mutual exclusion, or a general device-memory model. Use atomics and synchronization for threads; use documented hardware access mechanisms for device registers.

Pointer provenance ties these concerns back to the optimizer. Even where the exact formal provenance model is the subject of continuing standards work, portable code should not assume that identical integer address bits make all pointers interchangeable. Allocation identity, object lifetime, and access permissions are meaningful constraints.

## 13. String and function addresses

A string literal such as `"hello"` is an array with static storage duration; it is not a heap-allocated `std::string`. Modern C++ does not permit modifying a string literal:

~~~cpp
const char* p = "hello";
// char* q = "hello"; // ill-formed in modern C++
~~~

Adjacent literals concatenate during translation:

~~~cpp
const char* sentence = "one " "two " "three";
~~~

Raw-string syntax changes escaping, not storage lifetime:

~~~cpp
auto path = R"(C:\path\file)";
~~~

C++20 changed the element type of UTF-8 string literals to `const char8_t`, so older code that passed `u8"text"` to a `const char*` parameter may need adjustment. `std::u8string_view` is a non-owning view of such code units; it still obeys ordinary view lifetime rules.

Function pointers occupy their own ABI category:

~~~cpp
using Callback = int (*)(int);

int square(int x) { return x * x; }
Callback cb = &square;
int result = cb(4);
~~~

The explicit address-of operator is optional in many such conversions. Calling conventions can be ABI-significant, so C interoperability and plugin interfaces should state the required linkage and calling-convention contract.

A noncapturing lambda may convert to a compatible function pointer:

~~~cpp
int (*fp)(int) = [](int x) { return x + 1; };
~~~

A capturing lambda contains state and cannot generally be represented by a plain function pointer. A C callback API commonly needs a function pointer together with an opaque context pointer, while C++ code can use a suitable callable wrapper.

Pointers-to-members use `.*` and `->*` because they designate members *relative to an object*, not stand-alone locations:

~~~cpp
struct S {
    int value{};
    void action();
};

int S::* pm = &S::value;
void (S::* pf)() = &S::action;

S s;
S* ptr = &s;

s.*pm = 10;
ptr->*pm = 20;
(s.*pf)();
(ptr->*pf)();
~~~

The possible adjustment of `this` under multiple inheritance makes member-pointer representation and size even less suitable for serialization as `void*`.

## 14. Pointer qualification, incomplete types, and structure design

Const qualification has two independent dimensions:

~~~cpp
int* p = nullptr;                 // reseatable, mutable pointee
const int* cp = nullptr;          // reseatable, const access
int* const fixed = p;             // fixed pointer, mutable pointee
const int* const fixed_const = p; // fixed pointer, const access
~~~

Converting `int*` to `const int*` is allowed; removing pointee constness is not implicit. A `const_cast` can remove qualification syntactically, but modifying an actually const object through it is undefined behavior. Deep constness does not follow automatically from the outer pointer: `T* const*` and `const T**` have different meanings and qualification conversion rules must prevent unsafe mutation through multiple levels of indirection. The same problem prevents naïve covariance of `Derived**` to `Base**`.

Similarly, `const std::unique_ptr<T>&` prevents modifying that owner object through the reference but does not necessarily prohibit changing its non-const pointee. C++ constness is not a transitive guarantee that the reachable object graph is immutable, nor does it determine who owns the objects reached.

A pointer can be declared to an incomplete type:

~~~cpp
class Impl;
Impl* ptr = nullptr;
~~~

At that point, constructing an `Impl` object by value or asking for `sizeof(Impl)` requires the complete definition. This separation is the basis of PImpl designs. A class can hold `std::unique_ptr<Impl>` while `Impl` is incomplete in its header, but a default deleter needs a complete type at the destruction point. A common solution is to define the owning class's destructor out of line, after `Impl` becomes complete.

Indirection is also structurally necessary for recursive data:

~~~cpp
// struct Node { Node next; }; // impossible infinite-size object

struct Node {
    std::unique_ptr<Node> next;
};
~~~

The pointer's size is independent of the eventual node object and allows the recursive structure to have finite representation.

## 15. Sequencing is not an excuse for clever pointer mutations

Older literature sometimes says that “evaluation order is undefined” without distinguishing the actual standard concepts. C++ separates *sequenced before*, *indeterminately sequenced*, *unsequenced*, and *unspecified order*. Certain unsequenced conflicting side effects cause undefined behavior, but unspecified order is not synonymous with UB. C++17 strengthened sequencing rules for several expression forms.

In pointer-heavy code, a simple style remains safer than depending on subtle sequencing guarantees:

~~~cpp
auto old = p;
++p;
use(old, p);
~~~

This is easier to review than a compact expression mixing increments, dereferences, calls, and aliasing side effects. The objective is not to ban expressions with multiple operands; it is to make mutation order visible when correctness depends on it.

## 16. Choosing the appropriate interface

The language offers multiple access and ownership abstractions because they carry different contracts. A value `T` normally means a value whose lifetime is managed directly by the containing scope or object. `T&` expresses required mutable borrowed access, and `const T&` required read-only borrowed access. A `T*` commonly denotes optional or reseatable borrowed access, though low-level and C APIs may impose a required non-null contract.

A `std::span<T>` denotes a borrowed contiguous sequence, and `std::string_view` a borrowed character sequence. `std::unique_ptr<T>` denotes exclusive ownership, `std::shared_ptr<T>` shared lifetime ownership, and `std::weak_ptr<T>` a non-owning connection to that ownership. Iterators and subranges denote traversal positions or ranges with their own invalidation conditions.

These are conventions with explicit language and library consequences, not an infallible taxonomy. A C ABI may legitimately require a non-null `T*` output parameter. A performance-sensitive intrusive container may use raw pointers deliberately. What matters is whether nullability, extent, lifetime, invalidation, and cleanup are all defined somewhere the caller can rely on.

A useful modernization is to replace informal pointer-plus-length relationships with `span`, to use `contiguous_iterator` and `to_address` rather than assuming all iterators are raw pointers, to copy representations with `bit_cast` or byte views rather than unrelated-pointer dereferences, and to express construction with `construct_at`. C++20's `assume_aligned` is appropriate only with a proven precondition; C++23's `start_lifetime_as`, `out_ptr`, `forward_like`, reference-from-temporary traits, and explicit object parameters target distinct, specialized problems. None removes the need to prove the underlying contract.

## 17. Diagnostics and toolchain boundaries

Compiler warnings catch obvious returning-address-of-local defects and certain use-after-free patterns, but they cannot reconstruct all object-lifetime histories. GCC commonly supplies `-Wall -Wextra -Wpedantic -Wreturn-local-addr` and `-Wuse-after-free=2` where supported. Clang provides `-Wreturn-stack-address` alongside the usual warning families. MSVC `/W4 /permissive- /analyze` adds static-analysis coverage. Exact diagnostic availability varies by version.

AddressSanitizer can detect many heap and stack bounds violations and use-after-free errors on executed paths. Clang's stack-use-after-return support can extend coverage when enabled with the relevant instrumentation and runtime settings. UndefinedBehaviorSanitizer checks selected invalid operations and misalignments; ThreadSanitizer can identify data races. None of these tools is a proof of pointer provenance, correct reclamation, or semantic lifetime validity, and none is a general ABA detector.

The `-fno-strict-aliasing` option can restrict selected alias-based optimization assumptions but does not create missing objects, repair lifetimes, or legalize all type-punning access. GNU dialect modes such as `-std=gnu++20` can accept extensions that a strict `-std=c++20` build rejects. `-fno-delete-null-pointer-checks`, when supported, alters some optimizer reasoning but is not a dangling-pointer cure. MSVC `/permissive-` and GCC/Clang `-pedantic-errors` help check conformance.

Vendor builtins such as `__builtin_assume_aligned`, `__builtin_object_size`, `__builtin_addressof`, and declarations such as `__attribute__((aligned))` or `__declspec(align)` can serve specialized implementation work. Prefer `std::assume_aligned`, explicit spans/extents, `std::addressof`, and `alignas` where their contracts fit. `__assume`, `__builtin_assume`, and C++23 `[[assume]]` supply unchecked optimizer assumptions; they are not runtime validation.

The C++ Core Guidelines and Microsoft GSL's `not_null` and span-oriented vocabulary help formalize API preconditions. clang-tidy and cppcheck can flag suspicious casts and ownership patterns; Compiler Explorer can show generated behavior under a selected compiler, target, and optimization mode. These are complementary defenses after a sound ownership and lifetime model has been chosen.

For code review, the most revealing questions are often simple: can the pointer be null, who owns the pointee, can a container reallocate, can a pointer escape after its owner dies, is arithmetic confined to one array, is the current object alive and suitably aligned, is the glvalue permitted to access its type, and does concurrent reclamation preserve the object through every read? An answer based on the printed numeric address is usually incomplete.

## 18. The standards timeline

**C++98/03** already had object and function pointers, member pointers, lvalue references, array-bound pointer arithmetic, construction and destruction, `new`/`delete`, placement `new`, and the foundational rules of the object model. Standard containers offered RAII ownership, but modern smart-pointer conventions were not yet established.

**C++11** introduced `nullptr` and `std::nullptr_t`, rvalue references, move semantics, forwarding references, `std::move`, `std::forward`, `unique_ptr`, `shared_ptr`, `weak_ptr`, `alignas`, `alignof`, and the atomics model. These made ownership transfer and pointer synchronization more directly expressible.

**C++14** added `std::make_unique`, `decltype(auto)`, more flexible generic lambdas, and improved return-type deduction. It reduced the need for hand-written ownership construction and enabled more precise reference-preserving interfaces.

**C++17** added `std::byte`, `std::launder`, support for over-aligned `new`, `std::string_view`, `std::optional`, `std::variant`, and `std::has_unique_object_representations`. These offered explicit tools for representation and alternative value states without granting general cast-based access.

**C++20** introduced `std::span`, `std::bit_cast`, `std::endian`, `std::to_address`, `std::contiguous_iterator`, ranges borrowed/dangling facilities, `std::construct_at`, `std::assume_aligned`, `char8_t`, `[[no_unique_address]]`, and atomic shared/weak pointer specializations. The major improvement was vocabulary for distinguishing bounded views, representation transfer, construction, and iterator categories.

**C++23** introduced `std::byteswap`, `std::out_ptr`/`std::inout_ptr`, `std::start_lifetime_as` and its related array facility, `std::forward_like`, the reference-from-temporary traits, and explicit object parameters. Each makes a previously subtle low-level operation more directly expressible, while retaining its preconditions.

**C++26** includes standardized hazard-pointer and read-copy-update facilities, subject to standard-library implementation availability. Their role is targeted safe reclamation in concurrent systems, not a blanket rule that atomic pointers are lifetime-safe. Compiler or library support must be checked for the exact facility rather than guessed from an advertised standard version.

## 19. Coming from Java, Python, or C#

Managed-language references are often associated with runtime-managed reachability. Java and C# ordinarily use tracing garbage collection for managed objects, while Python implementations use reference counting and/or garbage collection depending on the runtime. A C++ `T*` or `T&` generally provides *access*, not a guarantee that the referenced object remains alive.

Arrays and collections in managed runtimes commonly perform bounds checks; raw-pointer indexing in C++ instead has preconditions that may be violated without an exception. C++ `span` makes the extent available but does not magically validate all indexing or pin the owner. A dangling pointer is not required to throw when dereferenced.

The useful transition is to separate ownership from borrowing. Store values directly when feasible, use `unique_ptr` for exclusive heap ownership or `shared_ptr` when shared lifetime really is needed, and use references, pointers, spans, or views for clearly bounded borrowing. Use stable application identifiers rather than memory addresses, and RAII rather than hoping a later manual cleanup call executes after every exception.

The resulting design can be as safe and expressive as a managed interface, but its guarantees are established differently: through types, object lifetime rules, explicit ownership, and documented borrowing.

## 20. Working rules

The difficulty of pointer-heavy C++ is rarely the placement of `*` and `&`. It is proving the intersection of several contracts at the time of access. Storage must be present, sufficiently large, and aligned. A suitable object must currently exist there. The expression must have permission to access it through the chosen type. Pointer arithmetic and traversal must remain within the appropriate array domain. Some party must own the resource and preserve its lifetime through the final use. Concurrent algorithms must additionally prevent reclamation while readers still depend on the object.

A pointer can designate a location without satisfying every one of these conditions. A successful dereference on one build demonstrates what happened during that execution; it does not establish that the program complied with the language model.

> Pointer safety is the intersection of lifetime, bounds, type accessibility, alignment, and ownership. The address alone proves none of them.

The practical review discipline follows from that observation. Preserve extents and invalidation contracts, express ownership in types where possible, distinguish copying bits from converting values, and do not treat a cast as construction. Demand explicit reasoning for tagged pointers, member-pointer representations, integerized addresses, optimizer alignment assumptions, and atomic reclamation. Prefer an ordinary value or a clear borrowed interface over a clever address manipulation when both can satisfy the same performance requirement.

## Further reading

The most useful language references are [pointers](https://en.cppreference.com/w/cpp/language/pointer), [references](https://en.cppreference.com/w/cpp/language/reference), and [object lifetime](https://en.cppreference.com/w/cpp/language/lifetime). The corresponding library contracts are documented for [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr), [`std::shared_ptr`](https://en.cppreference.com/w/cpp/memory/shared_ptr), [`std::span`](https://en.cppreference.com/w/cpp/container/span), [`std::bit_cast`](https://en.cppreference.com/w/cpp/numeric/bit_cast), and [`std::start_lifetime_as`](https://en.cppreference.com/w/cpp/memory/start_lifetime_as).

The [C++ Core Guidelines on resource management](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#s-resource) and [lifetime safety](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#ss-lifetime) provide design-level complements. [WG21 P0593](https://wg21.link/P0593) is valuable historical context for the implicit-object-creation and object-lifetime facilities used in specialized storage code.

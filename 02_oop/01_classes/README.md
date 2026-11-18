# 01. Classes: Construction, Constness, and Value Semantics

A class in C++ is not merely "a struct with functions". It is a contract about three independent questions:

* how and where is an object born, and in what order are its parts built and torn down?
* which operations does the compiler synthesize for you, and which must you write, delete, or default explicitly?
* does this type behave like a value (copyable, comparable, disposable) or like an identity (reference-like, non-copyable, managed through a handle)?

Getting any one of these wrong produces code that compiles, often even runs correctly in the common case, and then breaks under a copy, a container resize, or a destructor order you didn't anticipate. The examples in this section walk through all three questions in the order a class designer actually meets them: first "where does this object live and when is it alive", then "how many ways can I build one and which should the compiler write for me", then "what does `const` actually promise about this object", then "how do I make a type behave like a plain value", and finally "what did people do before move semantics existed, and why don't we do that anymore".

The examples in this directory are:

1. [`01_construction/`](01_construction/construct.h) ([`construct.cpp`](01_construction/construct.cpp), [`class_static.cpp`](01_construction/class_static.cpp)) — the nine ways an object gets constructed/destroyed, the `new T()`/`new T` distinction proven on a genuine aggregate, forward-declared-by-value proven at a real call site, and the thread-safety guarantee on a Meyers' singleton verified with `std::thread`.
2. [`02_constructors/main.cpp`](02_constructors/main.cpp) — memberwise init, explicit destruction, `initializer_list` disambiguation, delegating constructors, `=default`/`=delete`, slicing, and two `static_assert`/runtime-proven facts: a destructor-only class is deceptively "move constructible" (it secretly copies), and a non-`noexcept` move constructor makes `std::vector` copy instead of move on reallocation.
3. [`03_constness/constness.cpp`](03_constness/constness.cpp) — const/non-const member functions proven by overload resolution, `mutable`, bitwise vs. conceptual constness.
4. [`04_concrete_class/concrete.cpp`](04_concrete_class/concrete.cpp) — the "concrete class" idiom: a hand-rolled value type with `{}`-init, copy/move, `operator<=>`/`operator==` (and the pitfall of treating the resulting order as mathematically meaningful), and guaranteed copy elision proven on a type with copy *and* move both deleted.
5. [`05_reference_counting/ref_count.cpp`](05_reference_counting/ref_count.cpp) — Meyers' reference-counted/copy-on-write string, a historically important technique that modern C++ has mostly retired, with real sharing/detach assertions (and a real heap-buffer-overflow this exercise surfaced and fixed).
6. [`06_deducing_this/main.cpp`](06_deducing_this/main.cpp) — C++23 explicit object parameters collapsing §3's const/non-const accessor pair into one function template.

A compact map before the details:

| Question | Answered by | Covered in |
|---|---|---|
| Where/when is the object alive? | storage duration, construction order | §1 |
| How many constructors, and which are synthesized? | delegating ctors, `=default`/`=delete`, initializer lists | §2 |
| What may a method do to `*this`? | `const`, `mutable`, bitwise vs. conceptual constness | §3 |
| Does this type behave like a value? | Rule of 0/3/5, copy/move, `{}`-init | §4 |
| What did "efficient copying" mean before move semantics? | reference counting, copy-on-write | §5 |

The shortest reliable mental model for the whole section is:

> A class is first and foremost a lifetime and identity policy. Its interface (`const`-correctness, operators, accessors) is how that policy becomes visible to callers.

---

## 1. Object lifetime: where and when a class is born

### There are more construction sites than "stack" and "heap"

[`construct.h`](01_construction/construct.h) enumerates nine places an object can come into existence: a named automatic object, a free-store object via `new`, a non-static class member, an element of a C array, a local `static`, a global/namespace-scope object, a temporary, an object produced by a user-defined allocator, and (as a non-case) a union member — unions cannot have members with constructors or destructors, because the compiler has no way to know which one to call.

```cpp
SimpleClass c;              // 1. named automatic object
SimpleClass* p = new SimpleClass;  // 2. free-store object
SimpleStruct intrnal;       // 3. non-static member (built as part of the enclosing object)
SimpleClass arr_c[5];       // 4. array element — default-constructed, no arguments possible
static SimpleClass stat_c1; // 5. local static — constructed on first use
```

Each site has its own rules about when the constructor runs and, more importantly, when the matching destructor runs.

### `new T()` and `new T` are not the same for built-in-only state

```cpp
SimpleClass* pc1 = new SimpleClass();  // value-initializes: zeroes members with no initializer
SimpleClass* pc2 = new SimpleClass;    // default-initializes: scalar members are left indeterminate
```

For a class with a user-provided default constructor the two forms end up equivalent, but for an aggregate or a class whose default constructor is implicit, the empty parentheses matter. This is one of the few places where `()` still earns its keep over `{}`.

### C arrays of class members can only default-construct

```cpp
int arr[5];          // fine, PODs don't care
// Elements of a member array of class type can only be constructed by the default constructor
```

If the element type has no default constructor (or you need per-element arguments), a C array member is a dead end — use `std::array` or `std::vector` instead. This restriction is a large part of why raw array members are now mostly historical.

### Construction order follows declaration order, not initializer-list order

```cpp
SimpleClass::SimpleClass(int i) : intrnal(i), arr()  // constructed in declaration order: intrnal, then arr
```

Base classes and members are constructed in declaration order and destroyed in the exact reverse order, regardless of the order you write the mem-initializers in. Most compilers will warn (`-Wreorder`) if your initializer list disagrees with declaration order — treat that warning as a correctness signal, not style noise, because it means the code *reads* as doing one thing while the compiler *does* another.

### Local statics give you lazy initialization for free

```cpp
int& return_static()
{
    static int res = 0;   // constructed exactly once, on first call
    return res;
}
```

A local `static` is constructed the first time control passes through its declaration, and destroyed in the reverse order of construction at program exit. This single rule is the entire mechanism behind the Meyers' singleton pattern — no explicit "has it been initialized yet" flag needed, and it is thread-safe since C++11 (initialization is guaranteed not to race).

### Global object order is a trap across translation units

```cpp
SimpleClass sc_global;   // constructor runs before main()
```

Constructors of namespace-scope objects run before `main()` in declaration order *within one translation unit*, but the order **between** translation units is unspecified. A global object whose constructor depends on another global defined in a different `.cpp` file is gambling with the static-initialization-order fiasco. Prefer function-local statics (lazy, ordered by first use) over namespace-scope globals when one object's initialization depends on another's.

### Static data members belong to the class, not to any object

```cpp
static int SimpleClass::_counter = 0;   // one instance total, shared by every object
```

A static data member exists independently of how many (if any) objects of the class exist, is inherited by every derived class as the *same* variable, and non-inline static data members still need exactly one out-of-class definition (pre-C++17) to avoid a linker error. Since C++17, `static constexpr` data members are implicitly `inline`, which removes the need for that out-of-class definition for the common "named compile-time constant" case.

### Forward declarations can carry surprisingly far

```cpp
class A;                 // forward declaration only
class SimpleClass
{
public:
    A get_a(A a);         // legal: A used by value in a declaration, not defined here
};
```

A forward-declared, incomplete type can still appear as a function parameter or return type in a *declaration* (including by value) as long as nothing in that translation unit needs `A`'s layout before its definition is visible. This is routinely used to cut compile-time dependencies between headers.

---

## 2. Constructors: defaults, delegation, and deletion

### Memberwise aggregate initialization needs open members

```cpp
struct opened { int a; double b; };
opened op = { 1, 1.0 };     // fine: all members public, no user-declared constructors

class closed { friend void show_memberwise_init(); int a; double b; };
// cpp::closed cl { 1, 1.0 };  // error even for a friend — aggregate init bypasses access checks
//                                differently from a normal member access, and simply doesn't apply here
```

Being a `friend` grants access to private members through ordinary expressions; it does not make a type an aggregate. If a type has private data, it needs a constructor — memberwise `{}`-init is only for types that are aggregates in the first place.

### A destructor can be made unreachable from the outside

```cpp
class prevent_destruction_private
{
public:
    void destroy() { this->~prevent_destruction_private(); }   // explicit, controlled destruction
private:
    ~prevent_destruction_private() {}
};
```

Making the destructor `private` (or `= delete`) stops ordinary code from destroying the object automatically — no automatic/stack instance can exist, and `delete ptr` won't compile either. A `private` destructor is strictly more flexible than a deleted one: you can still call it yourself from an accessible member function (as `destroy()` does above), which a `= delete`d destructor forbids outright, even internally.

### Initializer-list constructors win ties you might not expect

```cpp
my_constructor m1 {};      // empty braces: default constructor wins, not initializer_list<int>{}
my_constructor m2 { 1 };   // one element: initializer_list<int> wins over my_constructor(int)
my_constructor m3 { 1, 2 }; // multiple elements: initializer_list<int>, unambiguously
```

When both an `initializer_list` constructor and an ordinary constructor are viable, overload resolution prefers the `initializer_list` form for any non-empty brace list — and prefers the default constructor for an empty one. If you design a type with both, make sure the `initializer_list` behavior for the empty-list case doesn't quietly contradict what the default constructor does; that mismatch is a design smell, not a clever feature.

### Delegating constructors replace "call a private init() method"

```cpp
class my_delegate
{
public:
    my_delegate(int i) : i_ { i } {}
    my_delegate() : my_delegate { 42 } {}   // delegates to my_delegate(int)
private:
    int i_ = 0;
};
```

Since C++11, one constructor can hand off to a sibling constructor of the same class by name, instead of factoring shared logic into a separate `init()` member that has to be called from every constructor by hand and offers no help from the compiler if you forget. The one hard rule: you cannot both delegate *and* initialize another member in the same mem-initializer list — `X() : X{42}, a{56} {}` does not compile. Pick one.

### A handful of static members can be initialized right in the class body

```cpp
struct my_statics
{
    static const int i1 = 1;
    static const int i2 = 1 + 2;                 // any constant expression is fine
    static constexpr std::complex<int> c1 { 1, 2 };
};
```

A `static const` member of integral/enumeration type, or a `static constexpr` member of a literal type, may be given its value directly in the class declaration as long as the initializer is a constant expression — this is the modern, type-safe replacement for the old "unnamed `enum` as a pseudo-constant" trick seen in pre-C++11 code.

### Move operations have a contract, not just a signature

```cpp
my_movable(my_movable&& rhs) noexcept : count_{ rhs.count_ }, i_{ rhs.i_ }
{
    rhs.count_ = 0;
    rhs.i_ = nullptr;   // leave the source in a valid, destructible, empty state
}
```

A move constructor doesn't take a `const` argument — its entire purpose is to strip the resource out of its argument, and a `const` source could never allow that. It should also be `noexcept` whenever you can honestly make that promise: a throwing move constructor disqualifies the type from several standard-library optimizations (for example, `std::vector` falls back to copying on reallocation if the move constructor isn't `noexcept`). And if you declare a move constructor, declare the move assignment operator too — implementing one without the other leaves a half-moveable type that is easy to misuse.

### Slicing is a copy-assignment problem, and you have exactly two fixes

```cpp
base_sliced b1;  derived_sliced d1;
b1 = d1;         // compiles: only the base part of d1 is copied into b1 — silently lossy
```

Assigning a derived object to a base object by value "slices" off the derived part with no diagnostic. The two deliberate defenses are: delete the base class's copy operations outright (so no slicing assignment can even be written), or keep the assignment operator `protected`/`private` in the base so only members and friends — not arbitrary external code — can trigger it. There is no third option that keeps public, unrestricted base-class assignment safe.

### `=default` beats hand-writing "the obvious thing"

```cpp
no_move() = default;
~no_move() = default;
no_move(const no_move& rhs) : i_{ rhs.i_ } { std::cout << "copied\n"; }  // written for debugging only
// move operations are not declared, so they are not implicitly generated either
```

If memberwise copy/move/destruction is correct for your type, say so with `=default` rather than retyping it — it documents intent, and the compiler can special-case defaulted operations (e.g. recognizing a trivial destructor) in ways it won't for a hand-written equivalent that happens to do the same thing. Note also the classic trap: declaring a copy constructor (even one that just logs) suppresses the implicitly-generated move constructor and move assignment, silently downgrading every move of that type to a copy.

### `=delete` is a general-purpose "no" for any function

```cpp
template <typename T> T* clone(T* p) { return new T{ *p }; }
int* clone(int*) = delete;              // remove one specialization from the overload set

struct restrict_conversion
{
    explicit restrict_conversion(int i) : i_{ i } {}
    restrict_conversion(double) = delete;   // kill an unwanted implicit conversion
};

struct stack_allocated { void* operator new(size_t) = delete; };  // forbid heap allocation
```

`=delete` isn't special-cased to special member functions — it can remove a template specialization from a call's candidate set, forbid an implicit conversion you don't want, or take away `operator new` to force stack-only usage. The key difference from simply *not declaring* a function: a deleted function still participates in overload resolution and produces a hard compile error when selected, whereas an undeclared alternative is silently skipped in favor of whatever else is viable. Don't multiply constructors "just in case someone finds them useful" — every constructor you add is one more thing to keep correct under copy, move, and exception safety; delete what you don't want supported.

---

## 3. Constness in classes: what a method promises about `*this`

### `const` on a member function changes what `this` can do, not what the function returns

```cpp
class SimpleClass
{
public:
    void set_static(int i) const { a_stat = i; }   // legal: a_stat is static, not part of *this
private:
    static int a_stat;
};
```

Trailing `const` qualifies the implicit object parameter — informally, `this` behaves like a pointer-to-const inside the function body. It says nothing about the return type. And because a `static` data member is not part of any particular object's state, a `const` member function is free to change one; the object itself genuinely hasn't been touched.

### Const and non-const overloads are two different functions, selected by the caller's constness

> When both versions are available: the `const` overload is called through a const object, the non-const overload through a mutable one.

This is the mechanism behind every const-correct accessor pair (`T& at(i)` / `const T& at(i) const`), and it composes with a fact that is easy to forget: **if you declare a non-const `operator=`, the compiler does not generate a const-qualified one for you** — const-correctness of the implicit members has to be earned explicitly once you've written any of them by hand.

### Bitwise constness is not the same thing as conceptual constness

```cpp
class Polygon
{
public:
    Point get_point(int index) const
    {
        points_[index].x = 0;   // compiles: points_ is `Point* const`, not `const Point*`
        return points_[index];
    }
private:
    Point* const points_;       // the pointer is const; what it points to is not
};
```

The compiler only enforces *bitwise* constness: a `const` member function may not reseat `points_` itself, but nothing stops it from mutating through a pointer or reference member — the pointee is not a subobject of the class. *Conceptual* constness — "the externally observable state is unchanged" — is a design discipline the type's author has to maintain by hand; the type above compiles cleanly while blatantly violating it.

### `mutable` is how you tell the compiler a violation is intentional

```cpp
class lazy_value
{
public:
    int get() const
    {
        if (!cached_) { cache_ = calculate(); cached_ = true; }
        return cache_;
    }
private:
    mutable int cache_{};
    mutable bool cached_{};
};
```

A `mutable` member is exempt from the "no mutation through a const function" rule, which is exactly what a cache, a memoization slot, or a debug counter needs: the object's *logical* value hasn't changed even though its *physical* representation has. Reach for `mutable` deliberately for this purpose — not as a workaround to silence a const-correctness complaint you haven't actually thought through, and never as a substitute for synchronization: `mutable` grants permission to write, it grants no atomicity, locking, or ordering guarantee against concurrent `const` calls.

### "Read backward" settles ambiguous const-pointer declarations

```cpp
char const* cp2;   // pointer (*) to 'const' 'char'   — pointee is const
const char* cp3;   // pointer (*) to 'char' 'const'   — same meaning as cp2, different spelling
char* const cp1;   // 'const' pointer (*) to 'char'   — the pointer itself is const
```

`const char*` and `char const*` are identical; the position of `const` relative to `*` is what matters, not relative to the type name. Reading the declaration from the variable name outward resolves any case that looks ambiguous at a glance.

### Unnamed `enum` as a const member is a historical idiom worth recognizing, not imitating

```cpp
enum { c1 = 5, c2 = 7, c3 = 12 };   // pre-C++11 stand-in for an in-class integer constant
```

Before in-class member initializers and `static constexpr` existed in their modern form, an anonymous enumerator was the standard way to define a compile-time integer constant scoped to a class, because it consumed no storage and required no out-of-class definition. You'll still meet this in older codebases; in new code, prefer `static constexpr`.

---

## 4. The concrete class idiom: making a type behave like a value

### A "concrete class" is just a type that behaves like a built-in one

```cpp
class complex
{
public:
    complex(double re = 0., double im = 0.) : re_{ re }, im_{ im } {}
    double real() const { return re_; }
    double imag() const { return im_; }
private:
    double re_;
    double im_;
};
```

No pointers to represent identity, no virtual functions, no base class — construction, copy, comparison, and destruction all behave the way they do for `int` or `double`. `{}`-init is preferred over `()` here: it is explicit about *initializing* rather than *calling a function that happens to look like a declaration* (the classic "most vexing parse" that `{}` sidesteps), and it refuses silently narrowing conversions.

### A hand-rolled container has to get copy, move, and assignment all pulling together

```cpp
vector(const vector& rhs) : data_{ new T[rhs.size()] }
{
    std::copy(rhs.data_, rhs.data_ + rhs.size(), data_);  // allocate-then-copy: exception safe
    sz_ = rhs.size();
}

vector(vector&& rhs) noexcept : data_{ rhs.data_ }, sz_{ rhs.sz_ }
{
    rhs.data_ = nullptr;
    rhs.sz_ = 0;
}
```

Notice the copy constructor allocates its *own* buffer and copies into it before touching any member of `*this` — if the copy throws partway through, the half-constructed object never becomes visible and no member has been left in a torn state. The move constructor, by contrast, just steals the pointer and neuters the source; there is nothing to fail, which is exactly why it can honestly be `noexcept`. Copy assignment can reuse the same allocate-then-swap discipline (`copy-and-swap`), which gets exception safety for free by construction instead of by careful bookkeeping in the assignment operator body.

### The Rule of 0/3/5 is really one rule with two trigger conditions

By default, every class gets a default constructor, copy constructor, copy assignment, move constructor, move assignment, and destructor. Two separate rules govern when the compiler takes any of those away:

* declaring **any** constructor suppresses the implicit default constructor;
* declaring a copy operation, a move operation, **or the destructor** suppresses the implicitly-declared move operations outright — they are not generated at all, full stop. The *copy* operations are treated more gently: declaring a destructor or a copy operation only makes the implicitly-generated copy constructor/assignment **deprecated** (since C++11), not deleted — and that remains true through C++23; no published standard has actually removed it, however often people repeat that it has. Only a user-declared **move** constructor or move assignment operator actually deletes the implicit copy constructor.

The practical, and genuinely deceptive, consequence: `std::is_move_constructible_v<T>` can be `true` for a `T` with no move constructor at all, because the (merely deprecated, still very much alive) copy constructor can bind an rvalue too — the trait only answers "can a `T` be constructed from a `T&&`", not "will that construction actually move anything". §2's `show_destructor_suppresses_move_not_copy()` proves this with a counting member: constructing from `std::move(a)` silently copies.

The practical consequence for class design is still the Rule of 0/3/5: either write **none** of the five special member functions and let the defaults manage your resources for you (Rule of 0 — e.g. the `complex` class above owns nothing non-trivial), or if you must manage a resource by hand, write **all** of destructor, copy constructor, and copy assignment together (the pre-C++11 Rule of 3), and in modern code add move constructor and move assignment too (Rule of 5) — because declaring the destructor alone already took the implicit moves away from you, leaving every "move" to silently, successfully compile as a copy instead.

### Guaranteed copy elision changed what a "return by value" actually does

```cpp
std::unique_ptr<int> ret_unique_ptr()
{
    std::unique_ptr<int> p(new int(10));
    return p;      // moves out of p — unique_ptr has no copy constructor to fall back on
}
```

Returning a named local object moves (or, for a genuine prvalue return expression, is constructed directly in place — no copy or move call happens at all since C++17's guaranteed copy elision). This is why the function compiles at all despite `unique_ptr` being non-copyable: the language doesn't need a copy constructor here, because — as of C++17 for the prvalue case — there is no temporary object to copy *from* in the first place.

### Functors and lambdas are how "pass behavior, not just data" looks in idiomatic C++

```cpp
auto less_than_x = [&](int a) { return a < x; };   // generates a function object equivalent to Less_than<int>{x}
```

A lambda expression is sugar for a compiler-generated class with `operator()` — the capture list (`[&]`, `[=]`, `[x]`, `[]`) controls exactly which enclosing names become members of that generated class and whether they're captured by reference or by value. Prefer function templates for *algorithms* and function objects/lambdas for the *policy* plugged into them — that split is what keeps `std::sort(v.begin(), v.end(), [](auto& a, auto& b){ ... })` readable instead of forcing a named comparator class for every call site.

### `using` aliases do for templates what `typedef` never could

```cpp
template<typename C>
using ElementType = typename C::value_type;   // typedef cannot be made into a template this way

template<typename Key>
using StringMap = std::map<Key, std::string>;  // a template alias, still open to one more parameter
```

`using` can introduce an alias *template*, keeping one template parameter open for the caller to supply later — something the old `typedef` syntax has no way to express. Reach for it whenever you want a uniform name for a family of related types without fully committing to one concrete instantiation.

---

## 5. Reference counting and copy-on-write: a technique modern C++ retired

### The problem COW strings solved

[`ref_count.cpp`](05_reference_counting/ref_count.cpp) implements the once-canonical Meyers reference-counted string in two shapes: a hand-rolled counter embedded in the string itself, and a cleaner version built on a reusable `rc_object` base and an `rc_ptr<T>` smart pointer. Both exist to answer one question from a world where copying meant "always duplicate the buffer": *why pay for a deep copy of a large string when the copy might never be written to?*

```cpp
rc_string(const rc_string& rhs) :
    _value((rhs._value->_shareable) ? (rhs._value) : (new string_value(rhs._value->_data)))
{
    if (rhs._value->_shareable)
        ++_value->_ref_count;     // copy = share the same buffer, bump a counter — no allocation
}

char& operator[](size_t index)   // non-const access: might mutate, so detach first
{
    if (_value->_ref_count > 1) {
        --_value->_ref_count;
        _value = new string_value(_value->_data);   // copy-on-write: pay the copy only here
    }
    _value->_shareable = false;
    return _value->_data[index];
}
```

A `const` access (`operator[] const`) never triggers a copy — it just reads the shared buffer. A non-const access *might* write, so it unconditionally detaches first ("copy-on-write"), then marks the detached buffer unshareable so a future copy of *this* object won't try to share stale state.

This has a sharp edge that is easy to miss when *writing a test* for this exact behavior (as [`ref_count.cpp`](05_reference_counting/ref_count.cpp)'s own `show_copy_on_write_sharing_and_detach()` does): indexing a non-`const` `rc_string` calls the non-`const` `operator[]` and detaches **unconditionally**, even if the caller only reads the returned reference and never writes through it. The function has no way to know you won't write — handing back a non-`const` reference *is* the commitment to "this might be mutated", so it pays the detach cost up front, on every non-`const` access, whether or not a write actually follows. Checking "are these two strings still sharing a buffer?" without destroying the very sharing you're checking for requires going through a `const` reference specifically — the const/non-const overload-resolution rule from §3 isn't incidental here, it's the only way to observe this class's internal state without perturbing it.

### The second version separates "being reference-counted" from "being a string"

```cpp
class rc_object { /* _ref_count, _shareable, add_ref(), remove_ref(), ... */ };

template <typename T>
class rc_ptr { /* re_init() on copy: detach-then-add_ref, remove_ref() on destruction */ };

class rc_string2
{
    // We don't need a copy constructor or operator= — rc_ptr's defaults do the job
    rc_ptr<string_value> _value;
};
```

Factoring the counting/sharing logic into `rc_object` + `rc_ptr<T>` means `rc_string2` itself needs no hand-written copy constructor, assignment operator, or destructor at all — the Rule of 0 applies *because* the resource-management work was pushed down into a dedicated handle type. That is the real lesson of this example, independent of reference counting specifically: wrap the resource once, in one place, and let ordinary composition give every class built on top of it correct-by-construction copy semantics.

### Why this technique is now mostly historical

Two things changed since this pattern was common:

* **Move semantics (C++11)** removed the original motivation. The whole point of COW was to make a *copy* cheap when the copy might not be written to. Move semantics instead make *transferring ownership* free, and in most of the call sites that used to rely on COW's implicit cheap-copy (returning a string by value, inserting into a container that reallocates), the compiler now moves instead of copying at all — no reference count, no shared buffer, no detach check on every non-const access.
* **The C++11 complexity/iterator-invalidation guarantees for `std::basic_string` made COW non-conforming for the standard library specifically.** The standard requires O(1) non-const `operator[]`/`data()` and requires that mutating one string not invalidate references into another that was copied from it — COW cannot satisfy both without an O(n) detach hiding behind what looks like an O(1) call. This is precisely why libstdc++'s pre-C++11 COW `std::string` had to be replaced (GCC 5's dual ABI, `_GLIBCXX_USE_CXX11_ABI`, exists entirely because of this change).

The technique itself is not "wrong" — it is a legitimate, general lazy-copy pattern, and you'll still see it (as copy-on-write, not reference counting per se) in large immutable data structures, some image/buffer types, and persistent data structures. For an ordinary owning value type in modern C++, prefer `std::string`/`std::vector`/move semantics first, and reach for `std::shared_ptr<const T>` when you genuinely want many owners of one immutable, heap-allocated object — it gives you the reference counting without hand-writing `add_ref`/`remove_ref`/detach logic yourself, and it is thread-safe in its control block in a way this example's `_ref_count` is not.

---

## Rules worth keeping in working memory

The recurring theme across all five examples is that a class's *interface* keeps a promise that its *implementation* must independently uphold — the compiler checks the syntax of that promise (does this function have `const`? does this constructor exist?) but not its substance (does this `const` function really leave the logical state alone? does this constructor really establish every invariant?).

> A class is a contract about lifetime and identity before it is a contract about operations.

### Core rules

1. **Decide Rule of 0 vs. Rule of 5 up front, not by accretion.** Either own nothing non-trivial and default everything, or write all five special members together.
2. **A declared destructor silently removes the implicit move operations.** If you need a custom destructor, decide explicitly whether copy and move still make sense for the type.
3. **`const` on a member function is a promise about `*this`, enforced only bitwise.** Use `mutable` to make an intentional conceptual-constness exception visible; never use it to paper over a design you haven't settled.
4. **Prefer composition of a resource-owning handle over hand-written copy/move logic in every class that uses the resource.** `rc_ptr<T>` in §5 and a smart pointer in general exist so that classes built on top of them can stay at Rule of 0.
5. **Don't multiply constructors speculatively.** Every constructor is another path that copy, move, exception safety, and invariants all have to hold for.

### Pitfalls at a glance

| Pitfall | What happens | Do instead |
|---|---|---|
| Declaring a copy constructor "just for logging" | Implicit move constructor/assignment silently disappear; every move becomes a copy | `=default` the ones you don't need to customize, or write all five together |
| Treating `mutable` as a const-correctness escape hatch | Hides real mutation of logical state behind a const-qualified call | Reserve `mutable` for caches/counters that don't affect observable state |
| Returning a base object by value where slicing is possible | Derived state silently discarded on assignment, no diagnostic | Delete the base's copy ops, or keep assignment `protected`/`private` |
| Writing a move constructor without `noexcept` | Standard containers fall back to copying instead of moving on reallocation | Mark move operations `noexcept` whenever the implementation genuinely cannot throw |
| Hand-rolling reference counting for an ordinary owned object | Reimplements what `std::shared_ptr`/move semantics already give you, without the thread safety | Prefer move semantics first; reach for `shared_ptr<const T>` for genuine shared ownership |

### Mental model summary

| Question | Language guarantee | Implementation detail | Application responsibility |
|---|---|---|---|
| What gets synthesized by default? | Precise suppression rules for each special member | Whether the synthesized version is efficient for this type | Decide Rule of 0 vs. Rule of 5 deliberately |
| What may a `const` method do? | Cannot mutate non-`mutable` members through `*this` | Whether `mutable` state is actually synchronized | Keep the logical/conceptual contract honest |
| Does a copy allocate? | Not specified by the language at all | COW vs. eager-copy vs. move — purely a class-author choice | Pick the strategy the usage pattern actually needs |

### Review checklist

- [ ] Does this class follow Rule of 0, or does it explicitly define all five special members together?
- [ ] If a destructor is user-declared, were copy and move operations re-examined, not just left to whatever the compiler still generates?
- [ ] Is every `const` member function conceptually const, not just bitwise const — and is every `mutable` member there on purpose?
- [ ] Are move operations `noexcept` wherever that is honestly true?
- [ ] Could this hand-rolled resource-management logic be replaced by composing an existing handle (`unique_ptr`, `shared_ptr`, a container) instead?

---

## Diagnostics, useful compiler settings and extensions

A class's construction/destruction order and special-member generation rules are mostly enforced by the compiler; what it cannot check is whether your const-correctness or invariants are *conceptually* honest. Warnings and sanitizers catch the mechanical mistakes; review catches the rest.

### Warnings

| Compiler | Flags | What they reveal |
|---|---|---|
| GCC | `-Wall -Wextra -Wnon-virtual-dtor -Wreorder -Weffc++` | Missing virtual destructors, mem-initializer order mismatches, broad Effective-C++ style issues |
| Clang | `-Wall -Wextra -Wnon-virtual-dtor -Wreorder -Wshadow-field` | Similar coverage, plus shadowing of a base member by a derived one |
| MSVC | `/W4 /w14265 /w14263` | Non-virtual destructor with virtual functions; member-function hiding |

### Sanitizers and runtime checks

| Tool | Setting | Best use |
|---|---|---|
| AddressSanitizer | `-fsanitize=address` | Use-after-free from a stolen pointer left in a "moved-from" object, double-`delete` |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | Calling a member function through a dangling/invalid `this`, misaligned access |
| Valgrind | default | Leaks from a reference-counted type whose `check_delete()`/detach logic has an off-by-one |

### Semantics-changing options

| Option | Effect | Recommendation |
|---|---|---|
| `-fno-elide-constructors` (GCC/Clang) | Disables non-mandatory copy elision, forcing extra copy/move calls | Use only to teach/verify where the standard still permits (but doesn't require) elision |
| `-std=c++17` vs. `-std=c++20` | Changes whether copy/move suppression by a user-declared destructor also removes implicit copy ops | Target at least C++20 for the final, non-deprecated suppression rules |
| `/permissive-` (MSVC) | Disables non-conforming extensions, including some constructor-overload-resolution quirks | Always enable for portable class code |

### Compiler extensions

| Extension | Compilers | Use | Portable approach |
|---|---|---|---|
| `__attribute__((nonnull))` / `_Ret_notnull_` | GCC/Clang / MSVC | Documents non-null constructor/factory parameters or returns | Prefer references, `not_null<T>` (GSL), or `[[nodiscard]]` combinations instead of relying on the attribute alone |
| `#pragma pack` / `__declspec(align)` | MSVC / GCC/Clang | Controls member layout for a class | Prefer `alignas` where the standard mechanism suffices |

### Libraries and tooling beyond the standard

| Tool | Purpose | When useful |
|---|---|---|
| clang-tidy (`cppcoreguidelines-special-member-functions`, `modernize-use-equals-default`) | Flags inconsistent Rule-of-5 sets and hand-written defaults | Every review touching a class's special members |
| Compiler Explorer | Inspect generated construction/copy/move code | Verifying whether elision, `noexcept`-driven move selection, or inlining actually happened |
| Microsoft GSL | `not_null`, `owner<T>` | Documenting ownership and nullability at class boundaries |

---

## Standards timeline

### C++98/03: the original model

Classes already had constructors/destructors, const member functions, `mutable`, access control, and the implicitly generated default constructor, copy constructor, copy assignment, and destructor. There was no move semantics and no `=default`/`=delete` — disabling copying meant declaring copy operations `private` and never defining them (see `NonCopyClass` in [`construct.h`](01_construction/construct.h)).

### C++11: delegating constructors, defaulted/deleted functions, move semantics

This is the release the whole chapter pivots on:

* delegating constructors (one constructor calling a sibling constructor by name);
* in-class/default member initializers (`int b{};`);
* `=default` and `=delete`, applicable not only to special members but to any function, including template specializations;
* move constructors and move assignment, plus the rule that a user-declared destructor, copy operation, or move operation suppresses the implicitly-declared move operations;
* `std::initializer_list` and brace-init (`{}`) as a uniform initialization syntax;
* `constexpr` constructors, enabling literal (compile-time-constructible) class types.

The reference-counted string in §5 predates all of this — it is solving a problem (cheap copies) that move semantics address at the language level instead.

### C++14: incremental refinement

Relaxed `constexpr` function bodies made `constexpr` constructors and member functions practical for more than trivial cases; no changes to the core construction/copy/move model itself.

### C++17: guaranteed copy elision, aggregates with base classes

* **Guaranteed copy elision for prvalues**: returning a prvalue (e.g. `return T{...};`, or a temporary materialized directly into its destination) no longer requires — or even permits assuming — a copy/move call at all; the type doesn't need an accessible copy or move constructor for that specific case. This is distinct from, and stronger than, the older "the compiler is permitted to elide this copy" rule for named returns, which remains merely *permitted*, not mandated.
* Aggregates were extended to allow public base classes, so a derived type with no user-declared constructors can still be brace-initialized.
* The rule that a user-declared destructor or copy-assignment operator makes the implicitly-generated copy constructor/assignment *deprecated* dates to C++11, and is **still only deprecated** — as of C++23 the standard has not actually removed it. Verify this precisely before relying on it: `std::is_copy_constructible_v` on a type with only a user-declared destructor is `true` on every mainstream compiler, generated copy op and all; only a user-declared *move* constructor or move assignment operator actually deletes the implicit copy constructor.

### C++20: three-way comparison, designated initializers, formal deprecation removed

* `operator<=>` lets a class define ordering once and have `<`, `>`, `<=`, `>=` (and, combined with a defaulted `==`, `!=`) synthesized from it — replacing the historical six-operator boilerplate for a value type like `complex` in §4.
* Designated initializers (`T{.x = 1, .z = 2}`) extend aggregate `{}`-init with field names, in declaration order.

### C++23: deducing `this`

Explicit object parameters (`template<class Self> auto&& value(this Self&& self) ...`) let one templated member function stand in for the usual const/non-const (and now also lvalue/rvalue) overload family that §3's accessor pattern otherwise requires writing by hand — directly relevant to the const/non-const overload pair discussed there, and worked through for real in [`06_deducing_this/main.cpp`](06_deducing_this/main.cpp). That example could not be compile-verified on this machine's toolchain (Xcode 14/AppleClang 14 rejects the `this`-parameter syntax outright, confirmed with an isolated test — the same category of gap as `06_quick_exit`/`08_expected_unexpected` in `06_exceptions`); it has its own `CXX_STANDARD 23` property and is written carefully against the published feature paper, but verify on a newer toolchain before trusting it blindly.

### C++26: proceed cautiously

At the time of writing, C++26 is finalized at the standard-committee level but tooling support is still catching up. The most significant class-adjacent addition is compile-time **reflection** (`^^`, `std::meta::info`, splicing), which affects how class members can be introspected and generated at compile time — a capability well beyond anything in this chapter's examples. There is also committee work extending designated initializers to name members of base classes, but the exact wording and target release have moved between successive papers; rather than assert a specific guarantee here, check `__cpp_designated_initializers` (and your compiler's own C++26 feature-support page) against the proposal you actually care about before relying on it.

---

## Migration note for Java / Python / C# developers

The single biggest adjustment is that in C++ an object's lifetime is a property of *where it was constructed*, not of how many references point to it — and nothing comparable to a garbage collector will clean up after you if you get that wrong.

### Where intuition transfers and where it breaks

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| Object destruction | GC decides when, if ever | refcounting + GC for cycles | GC decides when, if ever | Deterministic: destructor runs exactly at scope exit, `delete`, or container element destruction (RAII) |
| Default constructor | Always synthesized if no constructor is declared, with field defaults (`0`, `null`, ...) | `__init__` is opt-in; uninitialized attributes simply don't exist | Always synthesized similarly to Java | Synthesized **only if no constructor at all is declared**, and it is deleted outright if any member is a reference or a `const` without an initializer |
| Assignment (`a = b`) | Rebinds a reference; `a` and `b` can alias the same object | Rebinds a name; same aliasing | Rebinds a reference for class types | Value semantics by default: `a = b` copies (or moves) `b`'s state into `a`'s own storage, unless `a`/`b` are themselves pointers/references |
| "Copying an object" | Usually means `clone()`/copy constructor pattern you opt into | `copy.copy`/`copy.deepcopy`, both opt-in | `MemberwiseClone` or a copy constructor, opt-in | Copying is the **default** behavior for every class unless you delete or restrict it |

### Common wrong assumptions

Assuming every class gets a usable default constructor the way Java and C# synthesize one is wrong the moment the class has a `const` or reference member with no initializer — the implicit default constructor exists as a declaration but is defined as deleted, so `Foo f;` simply fails to compile with no runtime involved at all. Assuming assignment is "just like Java" is the second trap: `a = b;` for class types is not rebinding a handle, it is running `operator=`, which by default does a full memberwise copy of `b`'s state into `a` — two objects, not one object with two names. And assuming a destructor will "eventually" run the way a GC "eventually" collects is backwards: in C++ destructors are the primary resource-management mechanism (RAII), expected to run deterministically and promptly, and a type with no sensible destruction story (the reference-counted types in §5 are a historical example of exactly this concern, before `shared_ptr` existed to handle it generically) is a design gap, not a performance detail.

### Idiomatic C++ replacement

| Habit from managed languages | Idiomatic C++ |
|---|---|
| "Everything is a reference, GC cleans up" | Pick value semantics (Rule of 0, owning members) by default; use `unique_ptr`/`shared_ptr` only where shared or polymorphic ownership is the actual requirement |
| `clone()` method to get an explicit deep copy | The copy constructor *is* the clone — if you want to forbid accidental copies, delete it and provide `clone()` explicitly only where polymorphic copying through a base pointer is needed |
| Relying on a synthesized no-argument constructor | Supply in-class member initializers (or an explicit default constructor) for any `const`/reference member, or accept that the type is intentionally not default-constructible |
| Manual reference counting of "shared" objects | `std::shared_ptr<T>` (thread-safe control block, no hand-written `add_ref`/`remove_ref`) in place of a §5-style `rc_object`/`rc_ptr` |

The underlying translation is: a managed language gives you memory-safety and sharing as defaults and asks you to opt into ownership and value semantics when you want them; C++ gives you ownership and value semantics as defaults and asks you to opt into sharing and reference semantics (`shared_ptr`, raw references, polymorphic base pointers) when you actually need them.

---

## Further reading

* [language/constructor](https://en.cppreference.com/w/cpp/language/constructor)
* [language/destructor](https://en.cppreference.com/w/cpp/language/destructor)
* [language/default_constructor](https://en.cppreference.com/w/cpp/language/default_constructor)
* [language/copy_constructor](https://en.cppreference.com/w/cpp/language/copy_constructor)
* [language/move_constructor](https://en.cppreference.com/w/cpp/language/move_constructor)
* [language/rule_of_three](https://en.cppreference.com/w/cpp/language/rule_of_three)
* [language/function#Deleted_functions](https://en.cppreference.com/w/cpp/language/function)
* [language/initializer_list](https://en.cppreference.com/w/cpp/utility/initializer_list)
* [language/aggregate_initialization](https://en.cppreference.com/w/cpp/language/aggregate_initialization)
* [language/copy_elision](https://en.cppreference.com/w/cpp/language/copy_elision)
* [language/operator_comparison#Three-way_comparison](https://en.cppreference.com/w/cpp/language/operator_comparison)
* [language/member_functions#Explicit_object_parameter](https://en.cppreference.com/w/cpp/language/member_functions)
* [memory/shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr)

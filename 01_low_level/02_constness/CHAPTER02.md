# Chapter 02. Constness and Constant Evaluation: Types, Time, and Contracts

A programmer encounters `const` long before encountering most of the rules that make it interesting. The word appears to say that something cannot change. Yet a `const` pointer may designate a mutable object, a `const` member function may update a cache or even modify an external object, and a `const` integer may acquire its value only after the program starts. Conversely, a function with no `const` anywhere in its declaration may be evaluated entirely at compile time.

These are not exceptions to a simple rule; they are consequences of several different rules that happen to meet in the same programs. Constness is primarily a property of types and access paths. Constant evaluation is a property of an expression's evaluation context. Static initialization imposes a condition on the initialization of an object. Template branching decides which definitions participate in an instantiation. A well-designed interface should distinguish these questions instead of using the word *constant* as though it answered them all.

The companion programs explore the same territory experimentally: [runtime constness](01_runtime_constness/runtime_constness.cpp), [compile-time constness](02_compile_time_constness/compile_time_constness.cpp), [constness in classes](03_constness_and_classes/constness_and_classes.cpp), and [constant-expression evaluation](04_const_expr_eval/const_expr_eval.cpp). This chapter develops the reasoning behind those examples.

## 1. Read-only access is not compile-time knowledge

Consider an object initialized from an input operation:

```cpp
int read_from_socket();

const int reply = read_from_socket();
```

After initialization, the name `reply` cannot be used to modify the object. Nothing here requires the initializer to be evaluated during translation. The program may not know the value until a network request has completed. Thus the familiar shorthand “`const` means a compile-time constant” is wrong.

The precise statement is narrower: a cv-qualified access path restricts what may be modified *through that path*. For an ordinary `const` object this also has consequences for attempts to modify the object by casting away the qualification, but those consequences are not the same as requiring compile-time initialization.

A `const` integral or enumeration variable can sometimes be used in a constant-expression context if its initializer meets the relevant requirements:

```cpp
const int side = 4;
static_assert(side * side == 16);
```

It is the combination of the object category, initialization, and constant-expression rules that makes this possible. `const` alone does not.

A constant expression is an expression the language allows to be evaluated where a compile-time value is required. Such contexts include `static_assert`, non-type template arguments, array extents in `std::array`, and labels in a `switch` statement:

```cpp
template<int N> struct buffer {};

constexpr int extent = 8;
buffer<extent> b;
std::array<int, extent> values{};

int select(int x) {
    switch (x) {
    case extent: return 1;
    default:     return 0;
    }
}
```

Here `extent` meets a much stronger requirement than simple read-only access.

### 1.1 From `constexpr` to `consteval`

C++11 introduced `constexpr` for variables and functions. A `constexpr` variable must have an initializer appropriate for constant evaluation and meets additional type requirements. A `constexpr` function, however, is not required to execute at compile time on every call:

```cpp
constexpr int square(int x) {
    return x * x;
}

constexpr int a = square(4);
static_assert(a == 16);

int n = read_from_socket();
int b = square(n); // ordinary runtime evaluation
```

The source definition is the same; the context of the call differs. An optimizer may fold a runtime call when it knows the arguments, but optimization is not what makes an expression a language-level constant expression.

C++20 introduced `consteval` to express the stronger requirement that applicable calls of a function be evaluated immediately:

```cpp
consteval int power_of_two(unsigned bit) {
    if (bit >= 31)
        throw "bit index too large";
    return 1 << bit;
}

constexpr int mask = power_of_two(5);
// int dynamic_mask = power_of_two(n); // error: n is not a constant expression
```

The `throw` is a convenient way to reject an invalid compile-time input: executing it cannot produce a valid constant expression. The shift uses signed `int` intentionally and restricts the argument to avoid out-of-range results for the common minimum 32-bit `int` model; production interfaces should express their chosen result width and range precisely.

A `consteval` function is also a constexpr function in the language's classification, but its immediate-call rules make it an API for compile-time computation, not merely an optimization hint. A useful design test is whether accepting a runtime call would violate the *meaning* of the interface. If runtime use is legitimate, `constexpr` is normally the more flexible choice.

### 1.2 Initialization is a third question

Another C++20 keyword, `constinit`, does neither of those jobs. It requires a variable of static or thread storage duration to receive static initialization rather than dynamic initialization:

```cpp
constinit int request_count = 0;

void record_request() {
    ++request_count; // valid: the object is mutable
}
```

This matters when translation units have interacting global objects, or when a thread-local object would otherwise need dynamic-initialization machinery. It is not a promise of read-only access or of compile-time evaluation on every use. Nor does it prevent a nontrivial destructor from running at shutdown.

`constinit` may be combined with ordinary `const` when both guarantees are intended, but not with `constexpr` in the same declaration. The facilities overlap in initialization requirements while imposing different conditions on the declared object.

At this point it is useful to distinguish the central families: `const` constrains mutation through types and expressions; `constexpr` enables constant-expression use; `consteval` requires immediate evaluation of the relevant calls; and `constinit` constrains the *initialization phase* of a static or thread-local variable. The other mechanisms discussed later—`if constexpr`, `std::is_constant_evaluated()`, and `if consteval`—answer questions about template selection and the current mode of evaluation.

## 2. The constness of an access path

The declarator `const int* p` is easy to misread if one treats all instances of `const` as qualifying the nearest variable name. The type matters:

```cpp
int value = 0;

const int* p1 = &value;       // pointer to const int
int* const p2 = &value;       // const pointer to int
const int* const p3 = &value; // const pointer to const int
```

Through `p1`, the integer cannot be assigned, but `p1` may be reseated. Through `p2`, the integer can be assigned, but the pointer object cannot be reseated. Through `p3`, neither operation is permitted:

```cpp
p1 = nullptr; // OK
*p2 = 1;      // OK
// *p1 = 1;   // ill-formed
// p2 = nullptr; // ill-formed
```

We call qualification of the object itself *top-level const* and qualification encountered through an indirection *low-level const*. This vocabulary becomes important in implicit conversions, overload resolution, and deduction.

The debates about “east const” and “west const” concern spelling, not semantics. `const int*` and `int const*` describe the same type. A codebase may prefer the latter because the declaration `int* const p` then reads consistently from the declarator outward. Either convention is sound if applied consistently.

### 2.1 References and the object they denote

A reference is not reseatable, so applying `const` directly to a reference type is not how C++ models read-only reference access:

```cpp
int x = 1;
int& rw = x;
const int& ro = x;
// int& const impossible = x; // ill-formed
```

The meaningful qualification belongs to the referred-to type. `ro = 3` is invalid, but `rw = 3` can still change the same underlying object. One const reference does not impose deep immutability on every other alias.

A const lvalue reference can also bind to a temporary, sometimes extending the temporary's lifetime:

```cpp
const std::string& text = std::string("hello");
```

Here the temporary string survives as long as the directly initialized reference. This lifetime rule is deliberately limited. It cannot repair a function that returns a reference to its own temporary:

```cpp
const std::string& broken() {
    return std::string("temporary"); // dangling reference
}
```

The problem is even easier to overlook with a read-only view:

```cpp
std::string_view broken_view() {
    return std::string("temporary"); // view outlives the string
}
```

`std::string_view`, `std::span<const T>`, `const T*`, and `const T&` may restrict mutation, but none of them owns the referenced storage. Constness and lifetime are independent contracts.

### 2.2 Deduction removes some qualifications and preserves others

By-value deduction produces an independent object. Consequently, `auto` and an ordinary by-value function-template parameter ignore top-level cv-qualification of the source expression:

```cpp
const int original = 42;

auto copy = original;
static_assert(std::is_same_v<decltype(copy), int>);

template<class T> void consume(T);
consume(original); // T is int
```

When an expression is bound by reference, preserving the referred-to type matters:

```cpp
auto& a = original;  // const int&
auto&& b = original; // const int& after deduction/collapsing

template<class T> void observe(T&);
observe(original);   // T is const int
```

An `auto&&` declaration with the appropriate deduction form can be a forwarding reference, and lvalue deduction plus reference collapsing explain the result. These rules let generic interfaces propagate cv/ref information where copying would erase it.

`decltype` obeys a different rule. Applied to an unparenthesized id-expression, it reports the declared type; applied to a general expression, it accounts for the expression's value category:

```cpp
const int x = 42;
decltype(x) first = x;   // const int
decltype((x)) second = x; // const int&
```

The extra parentheses convert the argument of `decltype` into a general lvalue expression. This distinction is foundational for forwarding utilities and generic return types.

C++17's `std::as_const` supplies a concise way to request a const lvalue access path without copying:

```cpp
std::vector<int> numbers{1, 2, 3};
auto& read_only = std::as_const(numbers);
static_assert(std::is_same_v<
    decltype(read_only), const std::vector<int>&>);
```

It is especially useful to select a const overload deliberately. The library deletes its rvalue overload, avoiding a tempting way to obtain a dangling const reference from a temporary.

### 2.3 Casting away constness does not change an object's nature

`const_cast` can change the qualification of a pointer or reference expression, but it cannot turn an actually const object into a mutable one:

```cpp
int n = 1;
const int* view = &n;
int* writer = const_cast<int*>(view);
*writer = 2; // OK: n itself is non-const
```

Contrast this with:

```cpp
const int n = 1;
int* writer = const_cast<int*>(&n);
// *writer = 2; // undefined behavior
```

The difference is not whether the cast is syntactically allowed; both casts are. It is whether modification is permitted for the underlying object. The same caution applies when casting away `volatile`: a cast does not abolish the external hardware contract or make non-atomic inter-thread access safe.

### 2.4 Shallow constness and deep immutability

Constness ordinarily stops at a level of indirection:

```cpp
int number = 1;
const std::vector<int*> pointers{&number};
*pointers[0] = 2; // valid
```

The vector's interface does not allow replacing its pointer element, but the pointer leads to a separately mutable integer. Likewise, `const std::shared_ptr<T>` prevents reseating the smart pointer object; it does not make the managed `T` const. Calling `owner->mutate()` may remain valid.

If an API promises read-only access to the *pointee*, types such as `std::shared_ptr<const T>` or a container of `const T*` express a stronger restriction. Deep immutability of an entire object graph requires additional design: every reachable mutable alias and mutable internal state must be considered.

## 3. Constness as part of a class interface

A trailing `const` on a non-static member function qualifies the implicit object parameter. In an ordinary `const` member function, `this` has the conceptual type “pointer to const object”:

```cpp
class counter {
public:
    int value() const { return value_; }
private:
    int value_{};
};
```

That member function normally cannot assign to `value_` or call a non-const member function on `*this`. The return type `int` is unaffected. This is an access rule on the object receiving the call, not a promise that the function has no effects anywhere.

Overloading an accessor by object constness is one of the most useful expressions of this model:

```cpp
T& at(std::size_t i)       { return data_[i]; }
const T& at(std::size_t i) const { return data_[i]; }
```

A mutable receiver selects the mutable overload when applicable, while a const receiver can call only the const-qualified form. Standard containers use this principle extensively.

### 3.1 Value categories and ref-qualified access

C++11 added ref-qualifiers to non-static member functions. An accessor may now distinguish an lvalue object from an rvalue as well as distinguish const from mutable access:

```cpp
T& value() &;
const T& value() const &;
T value() &&;
```

Consider an object whose member string may be returned safely by reference only when the containing object is going to remain alive:

```cpp
class name {
public:
    const std::string& text() const & { return text_; }
    std::string text() && { return std::move(text_); }
private:
    std::string text_;
};
```

The rvalue overload returns an owned string, avoiding a reference into a temporary that will soon be destroyed. For robust APIs, examine const rvalue receivers too: a `const &&` overload may be required, or rvalue access can be deleted where returning an owned value is inappropriate. Ref qualification is not merely a performance embellishment; it is a lifetime-design tool.

C++23's explicit object parameters, frequently called *deducing this*, let a member function template deduce the cv/ref category of its object:

```cpp
#include <utility>

struct box {
    int value_{};

    template<class Self>
    decltype(auto) value(this Self&& self) {
        return std::forward_like<Self>(self.value_);
    }
};
```

For mutable and const lvalues, and for mutable rvalues, the expression can respectively yield `int&`, `const int&`, and `int&&`. This reduces repetitive overload families when a common implementation truly preserves the same semantics. It also requires care: an rvalue reference returned into a short-lived object can dangle if retained by the caller. Expressiveness does not remove lifetime responsibility.

### 3.2 Physical and logical constness

Sometimes a class must modify implementation details without changing its externally observable value. `mutable` was designed for such cases:

```cpp
class lazy_value {
public:
    int get() const {
        if (!cached_) {
            cache_ = calculate();
            cached_ = true;
        }
        return cache_;
    }
private:
    int calculate() const;
    mutable int cache_{};
    mutable bool cached_{};
};
```

Memoization, lazily constructed caches, diagnostic counters, and synchronization members are common reasons for `mutable`. The distinction is often called *logical constness*: the abstraction remains unchanged even though its physical representation changes.

Nothing about `mutable` creates synchronization. Concurrent calls to the example above can race on `cached_` and `cache_`, which is undefined behavior. A thread-safe version may use a `mutable std::mutex`, an atomic state protocol, `std::call_once`, or another appropriate mechanism. The lock can be mutable because taking it does not change the abstract value. A `const` method does not automatically become thread-safe.

Using `const_cast<widget*>(this)` to update a cache often hides the intended invariant. More seriously, the same method may be invoked on a genuinely const `widget`. Modifying one of its non-`mutable` members through that cast would be undefined behavior. Declare true logical cache state `mutable` rather than relying on callers never to create const objects.

### 3.3 Overriding, static members, and external effects

A derived override must match the cv/ref qualification required by the virtual function:

```cpp
struct base {
    virtual int value() const = 0;
};
struct derived : base {
    // int value() override; // error: does not override
    int value() const override { return 1; }
};
```

The `override` specifier is valuable because a missed qualification becomes a compiler diagnostic rather than an unnoticed separate member function.

A static member function has no implicit object parameter; it cannot be trailing-`const`. Static data members instead follow ordinary variable qualification rules:

```cpp
struct constants {
    static constexpr int width = 32;
};
```

Since C++17, a `constexpr` static data member is implicitly inline, greatly simplifying definitions in headers.

Finally, consider:

```cpp
struct handle {
    int* p{};
    void increment() const { ++*p; }
};
```

This const member function changes the pointee. That can be entirely legal because the pointee is not a non-mutable subobject made const by qualification of `handle`. Constness constrains a particular path through an interface; it cannot promise that the whole process remains unchanged.

## 4. Building computations that can run at compile time

A `constexpr` object is const-qualified. For example:

```cpp
constexpr int answer = 42;
static_assert(std::is_const_v<decltype(answer)>);
```

An initialized `constexpr` pointer is a const *pointer*, not necessarily a pointer to const data:

```cpp
static int counter = 1;
constexpr int* p = &counter;
// p = nullptr; // error
*p = 2;        // valid
```

The pointer behaves like `int* const`, not `const int*`. It is easy to reverse the relation when reasoning only from the keyword.

A `constexpr` function is implicitly `inline` in the language's ODR sense. That facilitates header definitions; it does not order the optimizer to inline machine code at call sites.

### 4.1 From restricted expressions to ordinary control flow

C++11's original constexpr-function body restrictions were severe. C++14 admitted local variables, loops, branches, and modification of local state during constant evaluation:

```cpp
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;
    return result;
}

static_assert(factorial(5) == 120);
```

Compile-time computation no longer needed to look like recursive template metaprogramming. C++20 widened the possibilities further, extending library support and allowing transient dynamic allocation under constant-evaluation rules.

That last qualification matters. The ability to allocate while computing a constant result does not generally allow a dynamically allocated object to survive beyond the end of the constant evaluation and become arbitrary runtime-owned heap storage. Allocation, deallocation, object lifetime, and the permitted evaluation path must still satisfy the standard's rules.

A user-defined literal type can take part in the same mechanism:

```cpp
struct point {
    double x, y;
    constexpr point(double xx, double yy) : x(xx), y(yy) {}
    constexpr double norm2() const { return x * x + y * y; }
};

constexpr point p{3.0, 4.0};
static_assert(p.norm2() == 25.0);
```

Notice that the constructor is `constexpr`, the observer is both `constexpr` and trailing-`const`, and the object is declared `constexpr`. Those decorations express three separate things: eligible computation, read-only member access, and constant initialization/value use.

A constexpr member function need not be const:

```cpp
struct counter {
    int value{};
    constexpr void increment() { ++value; }
};

constexpr int count() {
    counter c{};
    c.increment();
    return c.value;
}
static_assert(count() == 1);
```

Mutation of a suitable object *created during the constant evaluation* can be legal. C++20 also expanded constexpr capabilities for class operations, including permitted virtual calls and constexpr destructors under the applicable restrictions. An old C++11 checklist is not an accurate guide to contemporary constant evaluation.

C++23 relaxed constexpr-function restrictions again: a function body can contain constructs that are not themselves usable on a constant-evaluated path, provided the executed path remains valid. It also permits useful static constexpr local objects in constexpr functions, where the current standard's constant-expression rules permit their use:

```cpp
constexpr int lookup(unsigned index) {
    static constexpr int values[] = {10, 20, 30, 40};
    return values[index]; // caller must supply a valid index
}
```

The evolution is toward ordinary C++ algorithms with valid compile-time executions, rather than a separate miniature compile-time language. For portable libraries, the minimum language mode, feature-test macros, and compiler support must still be checked.

## 5. Immediate functions and compile-time contracts

`consteval` is appropriate for computations whose result is part of the program's compile-time structure: literal validation, fixed schema calculations, generated identifiers and hashes, compile-time format descriptors, constant configuration, and non-type template arguments.

```cpp
consteval unsigned checked_port(unsigned value) {
    if (value == 0 || value > 65535)
        throw "invalid port";
    return value;
}

constexpr unsigned http = checked_port(80);
```

A call with invalid input cannot succeed in constant evaluation. Type checking, conversions, overload resolution, and object-lifetime rules still apply. `consteval` is not a shortcut around ordinary C++ semantics.

A subtle point concerns parameters. In

```cpp
consteval int identity(int x) {
    return x;
}
```

the invocation must satisfy the immediate-function rules, but the name `x` inside the definition is still a function parameter. It is not automatically a usable non-type template argument or a separately declared `constexpr` variable. A compile-time *invocation* and a syntactically constant-expression *identifier* are distinct concepts.

Pointers and references to immediate functions are also restricted: one cannot simply take the address of a `consteval` function and let it escape into an ordinary runtime callback. C++23 refined the propagation of immediate-function obligations through certain enclosing contexts, particularly templates and constexpr functions. Implementations supporting the newer defect-resolution rules can be identified with the `__cpp_consteval` feature-test macro and direct tests.

### 5.1 Choosing a branch for the evaluation mode

C++20 provides `std::is_constant_evaluated()` in `<type_traits>`. It asks whether the invocation occurs during evaluation of a manifestly constant-evaluated expression or conversion:

```cpp
constexpr int mode() {
    return std::is_constant_evaluated() ? 1 : 0;
}

constexpr int compiled = mode();
static_assert(compiled == 1);
int runtime = mode(); // ordinarily 0
```

This is a property of the source-language evaluation context. A compiler may optimize a normal runtime expression into a literal machine-code constant without turning that expression into a manifestly constant-evaluated one.

An algorithm can deliberately use different implementations:

```cpp
int fast_runtime_square(int);

constexpr int square_dispatch(int x) {
    if (std::is_constant_evaluated())
        return x * x;
    return fast_runtime_square(x);
}
```

The distinction can be meaningful when a runtime SIMD intrinsic or another platform facility is unavailable during constant evaluation. The two branches must nevertheless implement the same promised semantics if the API claims a single result independent of evaluation mode.

Two traps deserve special attention. `static_assert(std::is_constant_evaluated())` is trivially true because the condition of `static_assert` is manifestly constant-evaluated. The same applies when `std::is_constant_evaluated()` is itself used directly as an `if constexpr` condition. That does **not** select a runtime/compile-time implementation path.

Static and thread-local initializers introduce another subtlety: an implementation may perform a trial constant evaluation to determine whether static initialization is possible. Code whose value depends on `std::is_constant_evaluated()` can therefore behave unexpectedly during initialization. It should not be used as an oracle for startup ordering or the optimizer.

C++23 made the intended dispatch explicit:

```cpp
consteval int compile_time_impl(int x) { return x * 2; }

constexpr int twice(int x) {
    if consteval {
        return compile_time_impl(x);
    } else {
        return x * 2;
    }
}
```

The true branch of `if consteval` is an immediate-function context and can invoke the `consteval` helper appropriately. The negated form `if !consteval` is also available. By comparison, ordinary `if` selects on a boolean expression, and `if constexpr` selects a branch at template instantiation according to a constant-expression condition. These are not interchangeable mechanisms.

## 6. Static initialization without immutability

A static object's initialization may be static or dynamic under the standard's initialization rules. Dynamic initialization can create difficult order dependencies between translation units. `constinit` expresses the requirement that the variable receive static initialization.

```cpp
constinit int global_count = 0;
thread_local constinit int per_thread_count = 0;

void function() {
    static constinit int local_count = 0;
    ++local_count;
}
```

All three objects have static or thread storage duration. An ordinary automatic local `constinit int x = 0;` would be ill-formed because the keyword does not apply to automatic storage.

The initialized objects remain mutable unless independently qualified const. That is precisely why `constinit` exists: `constexpr` would impose stronger restrictions on the variable, whereas many counters, tables, handles, and thread-local state objects need early initialization followed by modification.

```cpp
constexpr int initial_capacity() { return 64; }
constinit int capacity = initial_capacity();
constinit const int protocol_version = 3;
```

The initializers must satisfy the static-initialization requirement. Unlike `constexpr`, `constinit` need not impose constant-destruction requirements appropriate to constexpr objects. A type may be statically initialized even when it is unsuitable as a constexpr variable. For thread-local objects, the guarantee can allow implementations to dispense with some dynamic-initialization checks and guards, although the generated code remains an implementation choice.

The declaration is a contract about initialization *timing*, not about every subsequent access or the absence of runtime destruction.

## 7. Templates: evaluation and instantiation are different axes

The advantage of `constexpr` in generic programming is that one function template can serve both compile-time and runtime callers:

```cpp
template<class T>
constexpr T square(T value) {
    return value * value;
}

static_assert(square(5) == 25);
double input = read_double();
double result = square(input);
```

Whether a specialization can be constant-evaluated depends on the instantiated operations and the actual call. `constexpr` does not promise that every possible `T` supports multiplication or that every invocation of a valid specialization is a constant expression.

Constraints should therefore express what the body requires. For an integer greatest-common-divisor algorithm:

```cpp
template<std::integral T>
constexpr T gcd(T a, T b) {
    while (b != 0) {
        T next = a % b;
        a = b;
        b = next;
    }
    return a;
}
static_assert(gcd(48, 18) == 6);
```

A constraint is not a proof that every input avoids arithmetic edge cases. For example, signed division or remainder involving the minimum representable integer and `-1` requires care; a production GCD implementation must define its supported domain. Concepts clarify type requirements; values still need contracts.

An immediate function template is the stricter alternative:

```cpp
template<std::unsigned_integral T>
consteval T bit_mask(unsigned bit) {
    if (bit >= std::numeric_limits<T>::digits)
        throw "bit index out of range";
    return T{1} << bit;
}

constexpr auto bit = bit_mask<std::uint32_t>(7);
```

Each chosen specialization is an immediate function. This is useful when accepting runtime input is conceptually wrong, not just slower.

### 7.1 `if constexpr` changes which code is instantiated

C++17's `if constexpr` makes a compile-time decision about the branch appropriate to a specialization:

```cpp
template<class T>
constexpr auto normalized(T value) {
    if constexpr (std::is_floating_point_v<T>) {
        return value < T{} ? -value : value;
    } else if constexpr (std::is_signed_v<T>) {
        return value < 0 ? -value : value;
    } else {
        return value;
    }
}
```

For a particular `T`, discarded dependent branches are not instantiated in the ordinary way. This makes it possible to express type-specific operations without forcing every specialization to compile every branch. It does *not* guarantee that the selected call is constant-evaluated. Also, negation of the minimum signed integer can overflow; a robust magnitude interface must decide whether to return an unsigned magnitude or constrain the input.

C++20 structural class types widened the possibilities for non-type template parameters beyond earlier integral and enumeration values. Such arguments participate in template identity and instantiation:

```cpp
template<std::size_t N>
struct fixed_buffer {
    std::array<std::byte, N> storage{};
};

consteval std::size_t packet_size() { return 128; }
fixed_buffer<packet_size()> packet;
```

The value is part of a template argument, not an ordinary runtime integer the optimizer happened to fold.

C++14 variable templates provide a similar vocabulary for families of compile-time values:

```cpp
template<class T>
constexpr bool is_byte_like_v =
    std::is_same_v<std::remove_cv_t<T>, std::byte>;
```

Standard type traits' convenient `_v` forms follow this pattern. As always, distinguish the minimum version for the variable-template facility from the version introducing a particular trait or helper (`std::byte` and `std::is_same_v`, for instance, are C++17 vocabulary).

C++20 abbreviated templates, such as `void inspect(const auto& value);`, do not change deduction rules. `auto value` drops top-level const on a by-value parameter, `const auto&` provides a const read-only reference, and forwarding `auto&&` can preserve cv/ref categories.

Template definitions can also be needed in potentially constant-evaluated contexts even where a programmer did not expect a runtime call. The rules governing instantiation and constant evaluation interact; assuming “this branch will not execute at runtime, so its constexpr definition is unnecessary” is unsafe. In C++23, a template can combine `if consteval` with suitable compile-time and runtime implementations, allowing an immediate helper in the constant-evaluated branch while retaining one generic entry point.

The opposite design—changing an ordinary `constexpr` template to `consteval`—excludes runtime callers altogether. Do that only when it strengthens an actual API invariant.

## 8. Designing const-correct APIs without creating traps

A top-level `const` on a by-value return type generally hinders more than it helps:

```cpp
// Avoid:
const std::string make_name();

// Prefer:
std::string make_name();
```

The returned object belongs to the caller; the caller can make its local binding const if desired. Historically, top-level const on a class prvalue could interfere with move-aware or generic uses. It gives little useful protection at the interface.

Top-level const on an ordinary by-value parameter is different. These declarations do not distinguish overloads:

```cpp
void f(int value);
void f(const int value);
```

Within the *definition*, however, `const int value` prevents assignment to that local copy. That is an implementation choice, not a different callable interface.

Passing a large, nontrivial object by `const T&` often avoids copying while expressing required read-only access:

```cpp
void process(const large_object& value);
```

But references do not confer ownership, and small trivially copyable objects may be simpler and faster by value. A rule that every argument must be a const reference is no more justified than a rule that nothing should be const.

### 8.1 Iterator constness is not element constness

A const iterator object and an iterator over const elements are not equivalent:

```cpp
std::vector<int> items{1, 2, 3};

const std::vector<int>::iterator it = items.begin();
*it = 9; // valid: element is mutable
// ++it; // invalid: iterator object is const
```

By contrast, `std::vector<int>::const_iterator` can advance but yields read-only element access. The distinction is exactly the one encountered earlier between a const pointer and a pointer to const.

### 8.2 Constness is neither thread safety nor ownership

A const member function can update mutable state, modify an external object through a pointer, or observe an object that another thread changes. If those accesses race without synchronization, the result may be undefined behavior. Neither `const` nor a const view supplies mutexes or atomic ordering.

`volatile` is not the opposite of `const`; the qualifiers are orthogonal:

```cpp
volatile const std::uint32_t* status_register;
```

Such a declaration can model a read-only memory-mapped register under a platform-specific hardware contract. `volatile` does not replace `std::atomic` or synchronization in C++ concurrency, and it cannot by itself define the semantics of hardware access.

Likewise, `const T*`, `const T&`, `std::span<const T>`, and `std::string_view` are access descriptions, not ownership strategies. Every borrowed interface needs a separate answer to “who keeps the object alive?” In an API review, check that constness, lifetime, ownership, and synchronization are each expressed at the appropriate level.

## 9. Diagnostics, extensions, and implementation boundaries

Compiler diagnostics can catch many suspicious conversions and nonconforming expressions but cannot infer application invariants. GCC and Clang builds often begin with `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion`; MSVC projects commonly use `/W4 /permissive-`. More aggressive conformance checking may add `-pedantic-errors`. The exact diagnostics and coverage vary by compiler and release.

AddressSanitizer (`-fsanitize=address` or MSVC `/fsanitize=address`) detects many invalid accesses and lifetime errors when instrumented paths execute. GCC/Clang UndefinedBehaviorSanitizer (`-fsanitize=undefined`) covers selected undefined operations but is not comprehensive. clang-tidy, cppcheck, and MSVC `/analyze` can expose questionable interfaces, conversions, and object-lifetime assumptions statically. A clean report is evidence, not a proof.

Toolchain options can also change what code is accepted or what assumptions the optimizer makes. GNU language mode (`-std=gnu++20`) may accept extensions forbidden under `-std=c++20`. `-fno-strict-aliasing` limits certain alias-based optimizations without repairing invalid object lifetimes. `-fwrapv` specifies signed-overflow behavior for toolchains that implement it; the guarantee is not portable ISO C++. `-ffast-math` permits transformations incompatible with strict floating-point expectations. Such options should be tied to deliberate, documented contracts rather than treated as harmless speed flags.

Clang's `__builtin_assume` and MSVC's `__assume` tell the optimizer to trust a condition; they do not validate it. C++23's `[[assume]]`, where supported, expresses a similar obligation in standard syntax. GCC/Clang `__builtin_object_size` may estimate the storage available through a pointer, but explicit buffer extents remain preferable in APIs. Vendor `__attribute__` and `__declspec` facilities can supply linkage and layout hints that should be isolated behind wrappers and replaced by standard attributes where appropriate.

For engineering support, the C++ Core Guidelines help articulate contracts, clang-tidy and cppcheck review suspicious operations, Microsoft GSL provides bounds-oriented and non-owning vocabulary, and Compiler Explorer allows generated code to be checked rather than imagined. Feature-test macros such as `__cpp_*` are more reliable than compiler branding alone when choosing a language facility.

The most effective process combines expressive types, warnings, static analysis, focused testing, sanitizers, and narrowly scoped platform extensions. None of them makes an invalid `const_cast`, an escaping view, or a data race legitimate.

## 10. How the language developed

**C++98/03** established the central cv-qualification model: const references, const member functions, cv-qualified overloads, `mutable`, `const_cast`, and rules governing namespace-scope const objects and their linkage. These facilities were chiefly about type checking and interfaces, though integral constants were already useful in certain compile-time contexts.

**C++11** introduced `constexpr` variables, functions, constructors, and the associated literal-type model, giving compile-time computation a direct language mechanism alongside template metaprogramming. Its constexpr-function bodies were intentionally restricted. C++11 also supplied ref-qualified member functions.

**C++14** relaxed constexpr function bodies to allow common imperative constructs—loops, local variables, branching, and mutation of suitable local state—and introduced variable templates.

**C++17** introduced `if constexpr`, constexpr lambdas, inline variables, and implicitly inline constexpr static data members. Together these features made header-only compile-time facilities substantially easier to write and maintain. `std::as_const` also became available.

**C++20** introduced `consteval`, `constinit`, `std::is_constant_evaluated()`, concepts, and abbreviated function templates. Constant evaluation gained substantial library and language capabilities, including controlled dynamic allocation and broader constexpr class behavior. Structural types also widened non-type template arguments.

**C++23** introduced `if consteval` and `if !consteval`, relaxed additional constexpr restrictions, permitted appropriate static constexpr locals in constexpr functions, refined immediate-function propagation, and introduced explicit object parameters for cv/ref-sensitive interfaces.

**C++26** continues expanding constexpr-capable library facilities, but support is facility-specific. The applicable language mode, library revision, implementation feature tests, and defect reports matter more than the broad claim that a compiler “supports C++26.”

There is a clear progression: C++ increasingly allows ordinary algorithms to run in constant evaluation while retaining separate tools for expressing type constness, enforced immediate computation, initialization timing, and template-instantiation decisions.

## 11. Translating intuition from managed languages

Java, Python, and C# have constructs that resemble parts of C++ constness, but none maps onto the full mechanism. Java's `final` on a reference variable prevents rebinding but does not deeply freeze the referenced object. Python names can ordinarily be rebound; immutable objects and typing annotations such as `Final` are separate matters, with the latter not normally a runtime enforcement mechanism. C# `readonly` fields and `const` values have their own initialization and type restrictions. It is a mistake to treat all of them as synonyms for C++ `const`.

The differences become more consequential around failures and lifetimes. Managed languages commonly supply checked exceptions for invalid operations and some form of managed reachability. C++ has automatic, dynamic, and borrowed lifetimes, and a low-level misuse may be ill-formed, well-defined, unspecified, or undefined depending on the particular rule. An object is not kept alive merely because a `const T&` or `string_view` refers to its former storage.

The idiomatic translation is to select each guarantee explicitly. Use `const T&` or `std::span<const T>` for non-owning read-only access when its lifetime is established; choose values or owning containers when lifetime must be managed. Use `constexpr` when an algorithm should work in both execution modes, `consteval` when runtime invocation is invalid by design, `constinit` when initialization order is the central requirement, and `if constexpr` when the question concerns template instantiation rather than evaluation time.

Familiar surface syntax is a poor substitute for checking what each language actually guarantees.

## 12. A durable mental model

Most constness mistakes begin with a true but incomplete statement. “This parameter is const” says something about permitted access, not about object lifetime. “This function is constexpr” says something about possible constant-evaluated invocations, not about compulsory compile-time execution. “This object has constant initialization” says something about how it begins life, not whether it can change afterward.

In reviewing a low-level interface, separate the language guarantee from the implementation detail and the application requirement. The standard governs types, valid expressions, initialization, and evaluation contexts. The ABI, optimizer, and hardware determine concrete representations and strategies. The application must define ownership, value domains, lifetimes, synchronization, and the semantics of any external interface. None of those responsibilities can be delegated to a convenient keyword.

The core rules are therefore compact even if their consequences are extensive: keep const access separate from constant-expression eligibility; use `constexpr` for dual-mode functions and `consteval` only for immediate APIs; use `constinit` for guaranteed static initialization when later mutation may be appropriate; and never infer ownership, deep immutability, or thread safety from `const`.

A useful code review asks whether the claimed guarantees belong to the language or only to the observed build, whether conversions and lifetime preconditions are visible, whether edge cases can be tested without invoking undefined behavior, and whether the selected type expresses the intended contract. When a cast or an extension is needed, require an explanation of the specific guarantee it preserves.

> The strongest C++ interface is not the one with the most occurrences of `const`. It is the one that makes every relevant kind of constness—and every limit of that constness—unambiguous.

## Further reading

The precise rules are defined by the relevant revision of the C++ standard. For convenient cross-references, see [cv-qualification](https://en.cppreference.com/w/cpp/language/cv), [constant expressions](https://en.cppreference.com/w/cpp/language/constant_expression), [`consteval`](https://en.cppreference.com/w/cpp/language/consteval), and [`constinit`](https://en.cppreference.com/w/cpp/language/constinit). The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) supplement those rules with guidance on interface design, lifetime, and concurrency.

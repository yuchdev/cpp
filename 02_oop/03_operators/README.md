# 03. Operator Overloading

Operator overloading is the mechanism that lets a user-defined type participate in the same syntax as the built-in types: `a + b`, `m(i, j)`, `v[i]`, `++it`, `1.0_i`. It is not a separate language feature bolted onto classes — every overloaded operator is still an ordinary function (a member or a non-member) that the compiler calls instead of the built-in operator, chosen by the usual overload-resolution rules. That single fact explains almost everything else in this chapter: why some operators must be members, why some are better as free functions, why there is no runtime dispatch involved, and why getting the return type wrong silently breaks chaining or allows nonsense like `(a + b) = c`.

The examples in this directory are:

1. [`01_enum_operators/enum_overload.cpp`](01_enum_operators/enum_overload.cpp) — overloading `operator++` for a plain `enum`, including wrap-around logic. Builds and runs as-is.
2. [`02_complex_number/`](02_complex_number) — a `complex_t` class ([`complex.h`](02_complex_number/complex.h) / [`complex.cpp`](02_complex_number/complex.cpp)) and a `complex_matrix` class ([`complex_matrix.h`](02_complex_number/complex_matrix.h) / [`complex_matrix.cpp`](02_complex_number/complex_matrix.cpp)) that together demonstrate most of the chapter: compound-assignment members, friend arithmetic operators, prefix/postfix increment, conversion operators, `operator[]`/`operator()` indexing, deep-copy assignment with self-assignment checks, and friend declarations. **Note:** [`main.cpp`](02_complex_number/main.cpp) in this directory includes a `smart_ptr.h` from a sibling example that no longer exists at that relative path, so the directory is currently commented out of the build in [`CMakeLists.txt`](CMakeLists.txt) — it does not compile as one target today. `complex.h`/`complex.cpp`/`complex_matrix.h`/`complex_matrix.cpp` are self-contained and compile correctly on their own; they are the source of truth for the design notes below, and fixing `main.cpp` is a separate, future task.
3. [`03_user_defined_literals/main.cpp`](03_user_defined_literals/main.cpp) — deleting predefined class operators, a user-defined literal for a complex type, an explicit `operator bool`, template friends, and friend-lookup scoping rules. Builds and runs as-is.

---

## 1. What can be overloaded, and what the language refuses to let you touch

Almost every operator token can be given a user-defined meaning for a class or enumeration type:

```cpp
+  -  *  /  %  ^  &  |  ~
==  !=  <  >  <=  >=  <=>
+=  -=  *=  /=  %=  ^=  &=  |=  <<  >>  <<=  >>=
!  &&  ||
++  --
->  ->*  []  ()  ,
new  new[]  delete  delete[]
operator type()   // conversion operator
```

Four operators cannot be overloaded at all, under any circumstances:

```cpp
::   .   .*   ?:
```

The rationale is consistent across all four: `::` and `.` need to resolve names and members before any operand's type is even known, `.*` is tied to the same member-access machinery, and `?:` would need to evaluate only one branch depending on a user-defined condition — which contradicts ordinary function-call semantics where all arguments are evaluated.

A handful of other restrictions are easy to forget:

* You cannot invent a brand-new operator token or change precedence/associativity. `operator+++` is not a thing; `a + b * c` groups the same way regardless of what `+` and `*` do for your type.
* Operators cannot take default arguments — `operator+(const T&, int = 0)` is illegal, because it could silently turn what looks like a binary operator into something callable as unary, which would be deeply confusing to a reader who only sees `a + b`.
* An operator cannot be defined to work exclusively on pointer operands; at least one operand must be of class or enumeration type (or a reference/cv-qualified version of one).
* `operator=`, `operator()`, `operator[]`, and `operator->` **must** be non-static members — see the next section for why.

```cpp
// from complex.h — the class banner documents exactly this split
/*
Could be overloaded: + - * / = % ^ &| ~ == < > != <= >= += -= *= /= ...
Can't be overloaded: :: . .* ?:
*/
```

If you find yourself wanting an operator the language does not provide (say, `**` for exponentiation), the only legitimate escape hatch is a wrapper type with an ordinary named function, or occasionally a deliberately-named operator that is close enough in spirit (`^` is sometimes repurposed for this, as seen in math libraries) — but repurposing an operator for a meaning readers don't expect is exactly the trap "intuitive operators" warns against below.

---

## 2. Member, non-member, or friend — the three places an operator can live

C++ gives you three positions for an operator definition, and the choice is not stylistic — it changes what the first operand is allowed to be.

**Must be a member.** `operator=`, `operator()`, `operator[]`, `operator->`, and the conversion operators (`operator T()`) only make sense attached to a specific object, so the language requires them to be non-static members.

**Should usually be a non-member (often a friend).** Binary, commutative operators — arithmetic, comparisons, stream insertion — read better and behave more symmetrically as free functions. `complex.h` makes the case directly:

```cpp
// If operator+ is a member, `a + 5` works (int converts to complex_t)
// but `5 + a` does not, because overload resolution never gets a chance
// to convert the left-hand `5` into a complex_t.
friend const complex_t operator+(const complex_t& a, const complex_t& b)
{
    complex_t tmp(a);
    return tmp += b;
}
```

Note the pattern: the friend `operator+` is pure glue — it forwards to the already-tested compound-assignment member `operator+=`. Write `+=`, `-=`, `*=`, `/=` once as members, then implement `+`, `-`, `*`, `/` once each as one-line friends on top of them. Also note the **const** on the friend's return value: it makes `(a + b) = c` ill-formed, which is exactly what you want for a temporary that nobody should assign through.

**Can be either, pick by convention.** Overload resolution gives member and non-member candidates for an operator *equal* footing — there is no tie-break that prefers a member purely because it is a member, so this is a deliberate design choice, not a default.

A good rule of thumb distilled from the examples: compound-assignment and other operators that must mutate `*this` belong as members; binary arithmetic, stream, and comparison operators that treat both operands symmetrically belong as non-member (friend) functions; indexing and call-syntax operators are members because the language leaves no choice.

---

## 3. Overloading an operator for an `enum`

Enumerations are user-defined types too, and the built-in arithmetic/comparison operators do **not** automatically exist for them (unlike C, where an unscoped `enum` decays to `int` almost everywhere). `enum_overload.cpp` overloads prefix `++` for a plain weekday enum, with wrap-around:

```cpp
enum Wday { mon = 1, tus, wed, thr, frd, sat, sun };

Wday& operator++(Wday& d)
{
    return d = (d == sun) ? mon : Wday(d + 1);
}
```

A few things worth noticing in four lines:

* The operator is necessarily a **non-member**, because you cannot add member functions to an `enum`.
* It returns `Wday&`, matching the convention for prefix increment: the result is the object itself, after mutation, so chained use (`++(++d)`) and use as an lvalue remain legal.
* The cast `Wday(d + 1)` is required because `enum` arithmetic promotes to `int`; there is no implicit `int → Wday` conversion, so you have to ask for it explicitly even inside the enum's own increment operator.

This is the simplest possible case of operator overloading and a good first exercise before moving to a full class.

---

## 4. Compound assignment first, binary arithmetic second

`complex_t` (in [`complex.h`](02_complex_number/complex.h) / [`complex.cpp`](02_complex_number/complex.cpp)) is the chapter's main worked example for arithmetic operator design. The compound-assignment members come first and do the real work:

```cpp
complex_t& complex_t::operator+=(const complex_t& a)
{
    re += a.re; im += a.im;
    return *this;
}
```

Returning `*this` by reference is not a style preference — it is what makes `a += b += c` legal (each `+=` must produce something `+=`-able again) and it avoids manufacturing an unnamed temporary on every compound assignment. The binary `operator+` is then free to reuse it, as shown in section 2.

Increment/decrement follow the standard asymmetry:

```cpp
complex_t& complex_t::operator++()       // prefix: mutate, return reference
{
    re += 1.; im += 1.;
    return *this;
}

complex_t complex_t::operator++(int)     // postfix: save old value, return by value
{
    complex_t tmp = *this;
    re += 1.; im += 1.;
    return tmp;
}
```

The `int` parameter in the postfix form is never used for anything — it exists purely so the compiler can distinguish the postfix overload from the prefix one at the declaration level. Postfix is intrinsically more expensive (it must materialize and return a full copy of the old state), which is why `++it` is preferred over `it++` in generic/library code whenever the result value isn't actually needed.

Comparison operators are `const` because they only read state:

```cpp
bool complex_t::operator==(const complex_t& a) const { return re == a.re && im == a.im; }
bool complex_t::operator!=(const complex_t& a) const { return !(*this == a); }
```

Defining `!=` in terms of `==` (rather than duplicating the comparison) is the same "implement the primitive once, reuse it" discipline as the compound-assignment operators — and it is exactly the boilerplate that C++20's `operator<=>` (Standards timeline, below) was introduced to eliminate.

---

## 5. Indexing operators: `operator[]` and `operator()`

`complex_matrix` ([`complex_matrix.h`](02_complex_number/complex_matrix.h) / [`complex_matrix.cpp`](02_complex_number/complex_matrix.cpp)) shows the standard pattern for giving a container both mutable and read-only element access:

```cpp
complex_t* operator[](unsigned index);                      // mutable
const complex_t* operator[](unsigned index) const;          // read-only

complex_t& operator()(unsigned index_x, unsigned index_y);               // mutable
const complex_t& operator()(unsigned index_x, unsigned index_y) const;  // read-only
```

Two design points follow directly from the header's own comments:

* **Both a const and a non-const overload are needed**, exactly like the `at()` accessors in `01_low_level/02_constness`. The non-const overload must return a reference (not a value), or the result cannot be used as an lvalue — `m(0, 0) = complex_t(1., 2.)` requires `operator()` to hand back something assignable.
* **`operator()` is the right tool for multidimensional indexing**, not `operator[]`. Historically `operator[]` only ever accepted a single subscript, so a 2-D matrix needed either `m[i][j]` (via an intermediate row pointer, as `complex_matrix::operator[]` does here) or `m(i, j)` via a two-argument call operator. C++23 changes this (see Standards timeline) by finally allowing `operator[]` to take more than one index directly.

`operator[]` and `operator()` are two of the operators that the language *requires* to be non-static members — there is no free-function form, because both are fundamentally about "apply this access syntax to a specific object."

---

## 6. Resource-owning classes: assignment, copying, and the operators that guard them

`complex_matrix` owns a dynamically allocated 2-D array (`complex_t** _matrix`), which means its copy constructor and `operator=` cannot be left to the compiler-generated bitwise (shallow) versions — both must perform a **deep copy**:

```cpp
complex_matrix::complex_matrix(const complex_matrix& c)
    : _count_x(c._count_x), _count_y(c._count_y), _matrix(new complex_t*[c._count_y])
{
    for (unsigned i = 0; i < c._count_y; i++) {
        _matrix[i] = new complex_t[c._count_x];
        for (unsigned j = 0; j < c._count_x; j++)
            _matrix[i][j] = c(i, j);
    }
}
```

`operator=` has to additionally guard against **self-assignment** before it releases its own storage — `complex_matrix.cpp` shows both the naive version and a more robust copy-and-swap-flavored one behind an `#ifdef _NO_SWAP`:

```cpp
complex_matrix& complex_matrix::operator=(const complex_matrix& c)
{
    if (this == &c) {
        return *this;        // guard: without this, clear() below could
    }                         // destroy memory that `c` still needs
    complex_matrix* tmp = new complex_matrix(c);  // build the new state first
    clear();                                      // only then release the old one
    _matrix = tmp->_matrix;
    return *this;
}
```

Building the replacement before tearing down the original is the key exception-safety idea here: if construction throws, `*this` is left untouched. (A full copy-and-swap would go one step further and swap `_matrix` pointers with a locally-constructed temporary rather than reaching into it, avoiding the raw pointer hand-off — worth doing as an exercise on this class.)

Two more rules this class illustrates by consequence, not by comment:

* `operator=` is *not* in the "overloadable by non-member" camp — it is one of the operators the language forces to be a member, alongside `[]`, `()`, and `->`, precisely because it must be able to replace the state of an already-existing object.
* Every time you add a new data member to a class with a hand-written `operator=`, you must update it — the compiler will not remind you, and a forgotten member will silently keep its old value forever after every assignment.

---

## 7. Conversion operators: implicit by default, `explicit` by choice

`complex_t` declares two implicit conversion operators:

```cpp
complex_t::operator int()    { return static_cast<int>(re); }
complex_t::operator double() { return re; }
```

Conversion operators are declared with no return type at all (the type being converted to *is* the name of the operator), and — unusually for C++ — they take no value at the `return` either beyond the converted value itself; there is no `operator int() -> int` syntax to get confused with.

The danger with implicit conversions is ambiguity: with *both* `operator int()` and `operator double()` available, an expression like `1 + complex_number` can become ambiguous, because the compiler has two equally-valid ways to make the built-in `+` applicable. This is exactly the "ambiguity of operator overloading" problem the old design notes for this chapter warned about, and it is why real numeric types usually expose **one** preferred implicit conversion (or none) and name the rest as explicit, discoverable functions (`to_int()`, `to_double()`).

`03_user_defined_literals/main.cpp` shows the fix the language added in C++11 — **explicit conversion operators**:

```cpp
explicit operator bool() const noexcept { return p_ ? true : false; }
```

`explicit` here means the conversion only kicks in for direct-initialization and in contexts the standard specifically carves out as "boolean contexts" (`if (p1)`, `while (p1)`, `!p1`, operands of `&&`/`||`). It specifically forbids the surprising cases:

```cpp
if (p1) { /* OK — contextual conversion to bool */ }
// bool b = p1;       // error — not direct-init, not a boolean context
// int x = 1 + p1;    // error — exactly the kind of accident explicit prevents
```

This is the modern default for any "does this own something" style check (smart pointers, optional-like wrappers): prefer `explicit operator bool() const noexcept` over an implicit `operator int()`/`operator void*()` trick from pre-C++11 code.

---

## 8. User-defined literals

A **literal operator** maps a literal with a user-chosen suffix into a value of a chosen type, and it participates in constant evaluation when marked `constexpr`. `03_user_defined_literals/main.cpp` defines an imaginary-number literal:

```cpp
constexpr complex operator"" _i(long double d)
{
    return { 0, d };
}
// usage:
complex c2 = 1.0_i;
```

A literal operator's parameter list is restricted to one of a fixed set of signatures — informally, there are four literal "kinds", each with a small number of legal parameter forms:

| Literal kind | Legal parameter form(s) |
|---|---|
| Integer | `unsigned long long`, or `const char*`, or a template literal operator |
| Floating-point | `long double`, or `const char*`, or a template literal operator |
| String | `(const char*, std::size_t)` and the `wchar_t`/`char16_t`/`char32_t` equivalents |
| Character | `char`, `wchar_t`, `char16_t`, `char32_t` |

Two hard rules keep this from turning into syntax soup:

* **The standard library reserves every suffix that does not begin with an underscore.** Your own literal suffixes must start with `_` (`_i`, `_km`, `_b3`, ...); `1.0i` without the underscore is reserved and, for `std::complex`, already has a standard meaning.
* **You cannot redefine a built-in literal's syntax or an existing suffix's meaning** — user-defined literals are purely additive.

A rarer form — the **template literal operator** — takes the literal's digits as a non-type template parameter pack rather than a runtime parameter, which is the only way to give non-standard *meaning* to individual digits at compile time (for example, parsing a literal in a non-decimal base). The commented-out `_b3` ternary-literal example in `main.cpp` is a reasonable mental model even though it is currently disabled behind `#if 0` in that file.

---

## 9. Friends: declaration rules and why they matter for operators

Binary operators are the single most common reason a class declares a `friend`, because a free function needs private access to implement something like `operator+` efficiently without a public getter for every field. Two rules from `main.cpp` and `complex_matrix.h` are easy to get wrong:

* **A friend declaration can live in either the `public` or `private` section of the class — it makes no difference.** Friendship is not affected by access specifiers because a friend isn't a member; it is a grant of access to an outside entity.
* **Friend lookup is scope-sensitive.** A function (or class) named as a friend must already be visible — declared in an enclosing scope, or taking an argument of the class's own type — at the point of the friend declaration. `main.cpp`'s `show_find_friends` example demonstrates this directly: `f1`/`c1` (declared in the global namespace before `find_friends`) and `f3`/`c3` (declared in the same namespace, also before) are legitimately found as friends; `f4`/`c4`, declared in the enclosing namespace *after* `find_friends`, are not — the friendship grant silently does nothing, and the access that looked like it should work fails to compile instead.
* **A template parameter can itself be named as a friend** (`friend T;`), which is how a class can grant access specifically to whatever type it is instantiated with, without knowing that type in advance:

```cpp
template<typename T>
class X
{
    friend T;
    int x = 0;
};
```

The practical lesson: if a friend function seems to silently lose access after a refactor that moved its declaration later in the file (or into a different namespace), check friend-lookup visibility before assuming something else broke.

---

## Rules worth keeping in working memory

The thread running through every section above is that an overloaded operator is *just a function* — the syntax is special, the resolution and binding rules are completely ordinary. Every design question ("should this be a member?", "what should it return?", "should this conversion be implicit?") reduces to normal function-design questions once you see past the operator token.

> An operator's token tells the reader what syntax to expect. It guarantees nothing about what the operator actually does — that contract is entirely up to the author.

### Core rules

1. **`=`, `()`, `[]`, `->`, and conversion operators must be members; prefer non-members (often friends) for symmetric binary operators.**
2. **Implement compound-assignment operators (`+=`, `-=`, ...) first, as members that return `*this` by reference; build binary operators (`+`, `-`, ...) on top of them, returning by value (often `const` value).**
3. **Mark conversion operators `explicit` unless implicit conversion is a deliberate, central part of the type's design.**
4. **Don't overload `&&`, `||`, or `,` for class types** — the overloaded forms are ordinary function calls and lose both short-circuit evaluation and left-to-right sequencing guarantees that readers assume from the built-in operators.

### Pitfalls at a glance

| Pitfall | What happens | Do instead |
|---|---|---|
| Two implicit conversion operators on one type (`operator int()` + `operator double()`) | Mixed-type expressions like `1 + x` become ambiguous | Keep at most one implicit conversion target; name the rest (`to_int()`) or mark them `explicit` |
| `operator=` forgets to check self-assignment on a resource-owning class | `a = a` frees memory the right-hand side still needs, then reads/writes freed storage | Compare `this == &rhs` (or build the replacement before releasing the old state) before touching `*this`'s resources |
| Postfix `++`/`--` used in a hot loop where the result is discarded | An unnecessary copy is made every iteration | Prefer prefix `++it` whenever the old value isn't needed |
| Friend declared but defined later, in an enclosing/different namespace from where it was declared | The access silently fails to be granted; the "friend" can't actually reach private members | Declare (or pre-declare) the friend function/class in the exact enclosing scope before the friend declaration |
| `operator[]` used for 2-D indexing pre-C++23 | Needs an intermediate row-pointer hop (`m[i][j]`) or `operator()` instead | Use `operator()(i, j)` on pre-C++23 toolchains; adopt multidimensional `operator[]` once on C++23 |

### Mental model summary

| Question | Language guarantee | Implementation detail | Application responsibility |
|---|---|---|---|
| Which function does `a + b` call? | Resolved by ordinary overload resolution among member and non-member candidates — no operator-specific preference | Which translation unit defines it, inlining, ABI | Make the operator actually mean "+" in the domain sense |
| Can this operator mutate its operands? | Only if it is declared non-`const` / returns non-const reference | Whether the implementation actually performs the mutation efficiently | Document and keep the mutating/non-mutating split honest (`+=` mutates, `+` doesn't) |
| Does an implicit conversion fire here? | Governed by `explicit`, overload resolution, and conversion-sequence ranking | None — this is purely a language-level answer | Decide whether implicit conversion is a feature or a trap for this type |

### Review checklist

- [ ] Is every operator that must be a member (`=`, `[]`, `()`, `->`, conversions) actually declared as one?
- [ ] Do mutating operators return `*this` by reference, and do value-producing binary operators avoid returning something assignable (`const` value or a plain value)?
- [ ] Is every conversion operator either the single obviously-correct implicit conversion, or marked `explicit`?
- [ ] Does a hand-written `operator=` on a resource-owning class guard against self-assignment and stay exception-safe?
- [ ] Were `&&`, `||`, and `,` left alone (or at least not overloaded without a very deliberate reason)?

---

## Diagnostics, useful compiler settings and extensions

Operator misuse rarely produces a hard error — ambiguous conversions, forgotten self-assignment guards, and sequencing assumptions tend to compile cleanly and fail at runtime or only under specific operand combinations. Treat the compiler's operator-specific warnings as a first filter, not a proof of correctness.

### Warnings

| Compiler | Flags | What they reveal |
|---|---|---|
| GCC | `-Wall -Wextra -Wuseless-cast -Wold-style-cast -Wconversion` | Implicit conversions through conversion operators, redundant/old-style casts around operator results |
| Clang | `-Wall -Wextra -Wconversion -Wimplicit-int-float-conversion` | Similar conversion-sequence issues, with Clang's own ambiguity diagnostics for overload sets |
| MSVC | `/W4 /permissive-` | Non-conforming overload resolution, surprising conversions, nonconforming extensions |

### Sanitizers and runtime checks

| Tool | Setting | Best use |
|---|---|---|
| AddressSanitizer | `-fsanitize=address` | Catching self-assignment / double-free bugs in hand-written `operator=` and destructors on classes like `complex_matrix` |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | Signed overflow or invalid casts reached through conversion operators |
| Valgrind (memcheck) | default | Leak and use-after-free detection for deep-copy assignment operators that manage raw arrays |

### Semantics-changing options

| Option | Effect | Recommendation |
|---|---|---|
| `-std=c++20` / `-std=c++23` | Determines whether `operator<=>`, multidimensional `operator[]`, or deducing-`this` syntax is even available | Pin the standard explicitly rather than relying on compiler defaults |
| `-fno-elide-constructors` (GCC/Clang) | Disables some copy elision, which changes how many times operators returning by value actually construct/copy/move | Use only to stress-test that an operator's return path has no hidden side effects |
| `/permissive-` (MSVC) | Enforces standard-conforming two-phase lookup and overload resolution instead of MSVC's historical extensions | Enable for portable operator-overload code |

### Compiler extensions

| Extension | Compilers | Use | Portable approach |
|---|---|---|---|
| `__attribute__((warn_unused_result))` | GCC/Clang | Flags discarding the result of an operator like `operator+` that should never be ignored | Prefer `[[nodiscard]]` on the declaration, standard since C++17 |
| `__declspec(noinline)` / `__attribute__((noinline))` | MSVC / GCC/Clang | Forces an operator's body out of line, useful when inspecting generated code | Keep behind a build-only macro; never ship as a performance default |

### Libraries and tooling beyond the standard

| Tool | Purpose | When useful |
|---|---|---|
| clang-tidy (`cppcoreguidelines-*`, `misc-unconventional-assign-operator`) | Flags `operator=` that doesn't return `*this&`, missing self-assignment guards, and similar Core Guidelines violations | Reviewing any hand-written special member/operator |
| Compiler Explorer | Inspect what code a conversion operator or friend operator actually generates | Verifying that "zero-cost abstraction" claims hold for a specific operator design |
| cppcheck | Static analysis for common operator misuse (missing `const`, missing reference return) | Quick local pass before a full review |

---

## Standards timeline

### C++98/03: the baseline model

Member vs. non-member operator rules, the mandatory-member list (`=`, `[]`, `()`, `->`, conversions), implicit (non-`explicit`) conversion operators, and friend declarations were already the complete model. There was no language-level way to mark a conversion operator `explicit`, and no user-defined-literal mechanism — `1.0_i`-style suffixes did not exist.

### C++11: explicit conversions and user-defined literals

Two additions land directly in this chapter's examples:

* **Explicit conversion operators** — `explicit operator bool() const` and friends, restricting a conversion operator to direct-initialization and specific boolean contexts, exactly as used in `03_user_defined_literals/main.cpp`'s `smart_ptr::operator bool()`.
* **User-defined literals** — the `operator""` family, including the four literal "kinds" and the underscore-prefix reservation rule described in section 8.

### C++14: relaxed `constexpr`

`constexpr` function bodies were relaxed to allow loops, local variables, and branching. This matters for operators because a `constexpr` conversion operator or a `constexpr` literal operator (like `operator"" _i`) can now be written with ordinary imperative code instead of recursive, template-only tricks.

### C++17: stricter evaluation order for overloaded operators

C++17 tightened the sequencing rules specifically for operators called through operator syntax: every overloaded operator follows the sequencing of the built-in operator it overloads. For shift operators in particular, the left operand's value computation and side effects are now sequenced before the right operand's — so `std::cout << a() << b() << c()` is guaranteed to call `a()` before `b()` before `c()`, which was previously unspecified.

### C++20: `operator<=>` and default comparisons

The three-way comparison ("spaceship") operator, `operator<=>`, lets a class define ordering once and have the compiler synthesize `<`, `<=`, `>`, `>=` (and, together with a defaulted `operator==`, `==`/`!=`) from it via rewritten-candidate rules — directly removing the "implement `==` then `!=` in terms of it" boilerplate shown in section 4 for `complex_t`, and generalizing it to the full ordering set. A class can also simply `= default` both `operator==` and `operator<=>` to get member-wise comparison for free.

### C++23: multidimensional `operator[]`, static call operators, and deducing `this`

* **Multidimensional `operator[]`** — `operator[]` may now take more than one subscript directly (`m[i, j]`), removing the historical reason `complex_matrix` has to fall back on `operator()(i, j)` or a two-level `operator[]`-via-row-pointer for 2-D indexing.
* **`static operator()` and `static operator[]`** — these two operators may now be declared `static` when the call doesn't need access to `*this` at all (useful for stateless function objects), relaxing the "must be non-static member" rule just for this case.
* **Explicit object parameters ("deducing `this`")** — a single operator template with a `this Self&&` parameter can replace the const/non-const (and lvalue/rvalue) overload pairs that `complex_matrix::operator[]`/`operator()` currently duplicate by hand.

### C++26: ongoing direction, confirmed by WG21

Static reflection (`P2996`) was voted into the C++ standard in 2025 for C++26, introducing a brand-new prefix operator, `^^` (the "reflection operator"), which produces a `std::meta::info` value from a language construct. It is a genuinely new operator token at the language level — but it is a *compiler-provided* operator, not something user code overloads; it does not change any of the overloading rules in this chapter. Beyond that, expect continued refinement of C++23's explicit-object-parameter and multidimensional-subscript rules as compilers catch up, rather than new operator-overloading syntax.

---

## Migration note for Java / Python / C# developers

The instinct to ask "which operators can I overload, and how does the runtime dispatch to my version?" is itself the wrong question in C++ — there is no runtime dispatch. Operator overloading is resolved entirely at compile time by ordinary overload resolution against the static types of the operands; a C++ operator call is exactly as "virtual" as the function it resolves to (that is: not, unless you separately make it call a `virtual` function).

### Where each language lands

| Language | Can you overload operators? | Mechanism | Dispatch |
|---|---|---|---|
| Java | No (not for user classes) | N/A — `+` works only for numeric types and `String` concatenation is hard-wired into the language | N/A |
| Python | Yes, extensively | "Dunder" methods (`__add__`, `__iadd__`, `__eq__`, `__getitem__`, ...) looked up on the object's type | Runtime, via the type's method table — can differ per instance's actual type |
| C# | Yes, but restricted | `public static T operator+(T a, T b)` — operators must be `static`, and many (like `[]`, which maps to an indexer) are off-limits to overload directly | Resolved at compile time, similar in spirit to C++, but the restricted operator set and mandatory-`static` rule remove several of the member/non-member design questions this chapter covers |

Python is the closest useful comparison: `__add__`/`__radd__` mirror the "member vs. commutative free function" problem from section 2 almost exactly — Python's `__radd__` exists specifically to handle `5 + a` when `a`'s `__add__` isn't tried first or returns `NotImplemented`, which is the same asymmetry that pushes C++ binary arithmetic operators toward free (friend) functions instead of members.

### Common wrong assumptions coming from managed languages

* **"The compiler/runtime will figure out the right overload dynamically, like Python's duck typing."** It will not — C++ overload resolution happens once, at compile time, against static types; there is no fallback query at the call site the way `__radd__` or Python's `NotImplemented` protocol provides.
* **"Operators are virtual by default, like a Java/C# interface method would be."** They are ordinary (non-virtual) functions unless you explicitly make the underlying implementation call something `virtual`. Overload resolution for `a + b` is based on the static types of `a` and `b`, not their dynamic types.
* **"If I don't define `==`, I automatically get a sensible one, like Python's default identity-based `__eq__` or C#'s default reference equality."** A C++ class with no user-declared comparison operators simply has none — `a == b` is a compile error, not a (possibly wrong) default identity comparison, unless you explicitly `= default` it (C++20) or write it.

### Idiomatic C++ replacement

| Habit from managed languages | Idiomatic C++ |
|---|---|
| Define `__eq__`/`__lt__` (Python) or override `Equals`/`CompareTo` (C#) once, get ordering "for free" | `= default` or hand-write `operator<=>` (C++20) and `operator==`; let the compiler rewrite `<`, `<=`, `>`, `>=` |
| Implicit numeric-tower conversions (Python `int` → `float` everywhere) | Decide explicitly whether a conversion operator should exist, and mark it `explicit` unless implicit conversion is the actual design intent |
| Operator overloading via a universally-dispatched method table | Ordinary overload resolution — keep operand types narrow and avoid relying on ambiguous multi-way implicit conversions (section 7) |

---

## Further reading

* [language/operators](https://en.cppreference.com/w/cpp/language/operators)
* [language/operator_comparison](https://en.cppreference.com/w/cpp/language/operator_comparison)
* [language/operator_member_access](https://en.cppreference.com/w/cpp/language/operator_member_access)
* [language/operator_assignment](https://en.cppreference.com/w/cpp/language/operator_assignment)
* [language/operator_incdec](https://en.cppreference.com/w/cpp/language/operator_incdec)
* [language/user_literal](https://en.cppreference.com/w/cpp/language/user_literal)
* [language/explicit](https://en.cppreference.com/w/cpp/language/explicit)
* [language/default_comparisons](https://en.cppreference.com/w/cpp/language/default_comparisons)
* [language/friend](https://en.cppreference.com/w/cpp/language/friend)
* [language/eval_order](https://en.cppreference.com/w/cpp/language/eval_order)

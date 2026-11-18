# 05. Advanced Inheritance: Multiple Bases, RTTI, and Pointers to Members

Single inheritance hides most of the hard questions about class hierarchies. The moment a class has more than one base — or the program needs to ask what an object *actually is* at runtime instead of what the pointer's declared type claims — four separate questions appear that look related but are answered by different mechanisms:

* How is an ambiguous name, or an ambiguous base subobject, resolved when two base classes contribute the same thing?
* How many times does a shared base actually exist inside the final object, and which constructor runs it?
* What is the *dynamic* type behind a base pointer or reference, and can the program safely ask?
* Can a value refer to "a member of some object", resolved against a concrete object only later — and what does that value actually contain?

Treating these as one blurry topic called "advanced inheritance" is how the usual multiple-inheritance folklore gets started. The rules behind each question are precise and checkable; the folklore is mostly what happens when nobody checks them.

> **Build status.** All four directories build and run. This section was originally authored as a single, shared example — `virtual.h`, `simple.h`, `final.h`, `multimethod.h`, and `member_pointer.h` all included from one `main.cpp` — and later split mechanically into the four topic directories below, without finishing the split: `simple.h`, `final.h`, and `multimethod.h` had been lost entirely, and three of the four `main.cpp` files referenced headers that no longer existed anywhere on disk. The repair wrote a focused `main.cpp` per leaf and reconstructed the three missing headers from scratch (`simple.h`, `final.h`/`multimethod.h`), informed by how the original, broken `main.cpp` files used them; `member_pointer.h`/`member_pointer.cpp` were already complete and untouched. `02_virtual_inheritance`'s old `virtual.h`/`virtual.cpp` turned out to be a stray, unused duplicate of the virtual-*function* material already used correctly in [`04_inheritance_polymorphism/02_polymorphism`](../04_inheritance_polymorphism/02_polymorphism/virtual.h) — not actually about virtual *inheritance* — so this leaf's real content is its new `diamond.h`. A later pass added `assert`-verified proofs (construction-order counts, captured-and-compared output, `typeid`/`dynamic_cast` identity checks) to every one of these four `main.cpp` files, which previously only printed their results for a human to eyeball; `member_pointer.h`'s pre-existing `member_derived` overrides were also updated from repeated `virtual` to `override`.

The examples, as they stand:

1. [`01_multiple_inheritance/simple.h`](01_multiple_inheritance/simple.h), [`simple.cpp`](01_multiple_inheritance/simple.cpp) — plain (non-virtual) multiple inheritance over a `control_t` / `button_t` / `slider_t` / `window_draw` hierarchy: the duplicated-base-subobject problem, construction order, and resolving the resulting ambiguity by explicit qualification.
2. [`02_virtual_inheritance/diamond.h`](02_virtual_inheritance/diamond.h), [`diamond.cpp`](02_virtual_inheritance/diamond.cpp) — a minimal virtual-vs-non-virtual diamond (`base_v`/`left_v`/`right_v`/`bottom_v` vs. `left_n`/`right_n`/`bottom_n`), plus a self-contained progression of local classes in `main.cpp` measuring exactly what virtual inheritance costs in `sizeof()`.
3. [`03_typeid_rtti/codegen.h`](03_typeid_rtti/codegen.h), [`codegen.cpp`](03_typeid_rtti/codegen.cpp) — a richer virtual-inheritance diamond (`code_generator_base`/`cpp_code_generator`/`php_code_generator`/`code_formatter`) built specifically to give `dynamic_cast` upcasting, downcasting, and crosscasting to do, plus `typeid`, `std::type_info`, `std::bad_cast`, and `std::bad_typeid`.
4. [`04_member_pointer/member_pointer.h`](04_member_pointer/member_pointer.h), [`member_pointer.cpp`](04_member_pointer/member_pointer.cpp) — self-contained from the start; a pure virtual destructor with a body, and the typedefs that distinguish a pointer to a non-static member from a pointer to a static member. This leaf also gained [`final.h`](04_member_pointer/final.h) (the pre-C++11 "no further derivation" idiom) and [`multimethod.h`](04_member_pointer/multimethod.h) (Visitor-style double dispatch).

A short map of the facilities involved:

| Facility | Problem it solves |
|---|---|
| `using Base::member;` | Resolve an ambiguous *name* inherited from multiple bases |
| `Base::member()` | Resolve an ambiguous *call* by explicit qualification |
| virtual base (`: public virtual Base`) | Make a shared base exist exactly once in the final object |
| `dynamic_cast` | Convert along the *actual* (dynamic) type safely, or fail observably |
| `typeid` / `std::type_info` | Ask what the dynamic type actually is |
| pointer-to-member (`T::*`) | Refer to "a member", resolved against a specific object later |

The shortest reliable mental model for the whole section is:

> Everything here is about telling the compiler and the runtime apart. Ambiguity resolution and virtual inheritance are compile-time name/layout problems. `dynamic_cast`/`typeid` are the one place the language exposes genuine runtime type information. Pointers to members are compile-time-resolved *descriptions* of a member, not addresses.

---

## 1. Multiple inheritance: combining interface and implementation

### A class may inherit from more than one base

```cpp
class control_t
{
public:
    virtual void draw();
    virtual void hide();
    virtual void show();
};

class clickable_t
{
public:
    virtual void push();
};

class button_t : public control_t, public clickable_t
{
public:
    void draw() override;
    void push() override;
};
```

`button_t` is-a `control_t` and is-a `clickable_t` at the same time. Each base contributes its own subobject, its own set of virtual functions (so a polymorphic `button_t` can carry more than one vptr — see §2), and its own constructor call.

### Multiple inheritance is often used to separate interface from implementation

A common, well-behaved pattern is to inherit publicly from an abstract interface and privately (or by composition) from a concrete implementation:

```cpp
class drawable            // interface
{
public:
    virtual ~drawable() = default;
    virtual void draw() const = 0;
};

class gdi_backend          // implementation detail
{
protected:
    void blit() const;
};

class widget : public drawable, private gdi_backend
{
public:
    void draw() const override { blit(); }
};
```

Callers see only `drawable`; `widget` privately reuses `gdi_backend`'s machinery without exposing it. This is the multiple-inheritance idiom that ages well — the shared base (`drawable`) stays an interface, never a second concrete implementation competing for the same slot.

### Ambiguous *names* are resolved with a using-declaration

If two bases declare the same name and neither is more derived than the other, the name is ambiguous — simply naming it is an error, independent of whether it is ever called with matching arguments:

```cpp
struct A { void f(); };
struct B { void f(); };

struct C : A, B
{
    using B::f; // pick B::f as the name `f` found through C
};
```

Without the `using`-declaration, `C c; c.f();` fails to compile — not because the call is wrong, but because the *name lookup* itself is ambiguous. `using` resolves which declaration the derived class exposes under that name.

### Ambiguous *calls* can also be resolved by explicit qualification, at the call site

Instead of fixing the name once in the class, a single call can disambiguate itself:

```cpp
struct C : A, B
{
};

void demo(C& c)
{
    c.B::f(); // bypass ambiguity for this one call
}
```

This resolves the specific call without changing what `c.f()` means anywhere else.

### Access control participates in ambiguity resolution — even `private` members

A subtle and easy-to-forget rule: lookup finds the ambiguity (or the override) *before* access control is checked. If `A::f` is private and `B::f` is public, `c.f()` is still ambiguous — the compiler does not quietly prefer the accessible one.

> Changing a member's access specifier should never change what a well-formed program *means*. If it can turn an ambiguity error into a working call (or vice versa), the design was relying on an access modifier to do a job that `using`-declarations and qualified calls exist for.

### Construction order follows declaration order, not the inheritance list's intent

Base and member constructors run in the order the bases are *declared* in the class head and the members are *declared* in the class body — never the order they happen to be written in a constructor's initializer list:

```cpp
struct base_1 { base_1() { /* ... */ } };
struct base_2 { base_2() { /* ... */ } };

struct derived : base_1, base_2
{
    derived() : base_2(), base_1() // order here is cosmetic
    {
    }
};
```

`base_1` still constructs before `base_2`, because that is the order in `struct derived : base_1, base_2`. Writing the initializer list in a different order is legal but misleading, and most compilers warn about it (`-Wreorder` / `/W4`). Destruction runs in the reverse of construction order.

### Protected data is usually a design mistake, not a design tool

The old notes are blunt about this, and the bluntness holds up:

> Declaring a data member `protected` means every present and future derived class can quietly break the base class's invariants. It is almost always a sign that the base should expose a protected *operation* instead of a protected *field*.

```cpp
class counter_base
{
protected:
    int count_; // any derived class can set this to anything

public:
    void bump() { ++count_; }
};
```

Prefer a protected accessor/mutator pair, or (better) no shared mutable state at all — push it into a private member reached only through base-class member functions that derived classes call.

### If a class defines `operator delete`, it needs a virtual destructor — and vice versa

```cpp
class resource_owner
{
public:
    virtual ~resource_owner() = default; // without this, deleting through a base* is UB
    void* operator new(std::size_t);
    void operator delete(void*);
};
```

`delete` through a base pointer calls the *static* destructor unless the destructor is virtual — which also determines which `operator delete` overload runs, since deallocation is resolved together with the (possibly virtual) destructor call. A class that customizes allocation but is only ever used through a base pointer and has a non-virtual destructor is a classic source of silent leaks or corruption.

### Multimethods are deliberately not part of C++

A function whose overload is selected on the *dynamic* type of more than one argument — a "multimethod" or "multiple dispatch" — is not a language feature in C++. Bjarne Stroustrup, in *The Design and Evolution of C++*, is explicit about why:

> "I rejected multi-methods with regret, because I liked the idea, but couldn't find an acceptable form under which to accept it."

C++ overload resolution and virtual dispatch are both single-dispatch: they pick a function based on static argument types plus (for one distinguished argument, the implicit object) the dynamic type of `*this`. Dispatching on the dynamic type of *two* arguments has to be built by hand. §3 closes with the idiomatic way to build it — Visitor-style double dispatch — once the RTTI alternative (branching on `typeid`/`dynamic_cast`) has been covered and rejected for the right reasons.

---

## 2. Virtual inheritance and the diamond problem

### Repeated non-virtual bases duplicate the shared base subobject

Diamond-shaped inheritance, without any `virtual` keyword, gives the most-derived object one full copy of the shared base *per path* that leads to it:

```cpp
struct control_t      { int a, b; };               // 8 bytes
struct button_t  : control_t { int c; };            // control_t + 4 = 12 bytes
struct slider_t  : control_t { int c, d, e; };       // control_t + 12 = 20 bytes

struct window_draw : button_t, slider_t              // 2x control_t + ... = 36 bytes
{
};
```

`window_draw` contains **two independent `control_t` subobjects** — one through `button_t`, one through `slider_t` — and its constructor runs `control_t()` twice. An unqualified `wd.draw()` would even be ambiguous, because there are two paths to a `draw()`. [`01_multiple_inheritance/simple.h`](01_multiple_inheritance/simple.h)/[`main.cpp`](01_multiple_inheritance/main.cpp) build exactly this shape for real — printing the construction order and measuring it with `sizeof()` — though the real classes there carry a virtual destructor (for safe polymorphic cleanup), so their actual `sizeof()` values run larger than this trivial-struct illustration's 8/12/20/36 would suggest; the vtable pointer each `control_t` sub-object carries is extra cost on top of the repeated-base problem being illustrated here, not a contradiction of it.

### Virtual inheritance makes the shared base exist exactly once

Declaring the inheritance edge `virtual` tells the compiler that, no matter how many paths lead to that base, the final object contains exactly one instance of it:

```cpp
class code_generator_base
{
public:
    explicit code_generator_base(int id);
    virtual ~code_generator_base();
    virtual void generate_base() const;
};

class cpp_code_generator : public virtual code_generator_base { /* ... */ };
class php_code_generator  : public virtual code_generator_base { /* ... */ };

class code_formatter : public cpp_code_generator,
                        public php_code_generator
{
};
```

This is, almost verbatim, the real hierarchy in [`03_typeid_rtti/codegen.h`](03_typeid_rtti/codegen.h) — built there specifically so §3's `dynamic_cast` demonstration has a genuine crosscast to perform. `code_formatter` has one `cpp_code_generator` and one `php_code_generator` — and, because both inherit `code_generator_base` *virtually*, exactly one `code_generator_base`, shared by both. [`02_virtual_inheritance/diamond.h`](02_virtual_inheritance/diamond.h) builds the same idea in its simplest possible form instead — `base_v`, virtually-inherited by `left_v` and `right_v`, shared by `bottom_v` — alongside a non-virtual `base_v`/`left_n`/`right_n`/`bottom_n` diamond for direct comparison, and `main.cpp` there also runs a self-contained progression of local classes measuring exactly what each incremental step (an empty diamond, a non-virtual diamond, one virtual branch, both branches virtual, a virtual destructor added) costs in `sizeof()`.

### The virtual base's constructor is called by the most-derived class, not by the intermediate classes

This is the rule that surprises people who write virtual inheritance the first time:

```cpp
code_formatter::code_formatter(int id) :
    code_generator_base(id),   // <-- this one actually runs; the others' mentions of it are ignored
    cpp_code_generator(id),
    php_code_generator(id)
{
}
```

`cpp_code_generator(id)` and `php_code_generator(id)` each *mention* `code_generator_base` in their own constructor's responsibility when built standalone, but when either is a base of a more-derived class, the language ignores their attempt to construct the shared virtual base. Only the most-derived class — the one that actually knows there is exactly one `code_generator_base` to build — gets to call its constructor. If `code_formatter` did not explicitly mention `code_generator_base(id)` in its own initializer list, `code_generator_base`'s *default* constructor would run instead (and if it has none, that is a compile error). This is the real constructor in [`03_typeid_rtti/codegen.cpp`](03_typeid_rtti/codegen.cpp).

This single rule is why "just forward the constructor argument from each intermediate base" is the wrong mental model for virtual inheritance: the intermediate bases' opinions about the virtual base's constructor arguments are discarded.

### Virtual inheritance adds a layer of indirection to the object's layout

A typical layout for this hierarchy stores, inside each class that virtually inherits the shared base, a vptr plus a pointer (or vbase offset) to the one shared `code_generator_base` subobject, which lives once, typically at the end of the object:

```
cpp_code_generator:  [ vptr | ptr-to-code_generator_base ]
php_code_generator:   [ vptr | ptr-to-code_generator_base ]
code_formatter:       [ vptr ]
code_generator_base:  [ vptr | int ]
```

The exact representation (an extra pointer vs. a precomputed offset table) is an ABI detail, but the consequence is universal: a polymorphic object built from virtual inheritance can contain *multiple vptrs* — one per polymorphic base that is not laid out contiguously with the most-derived object — and an extra indirection to reach the shared base. This is real cost, not a free safety feature; virtual inheritance is for solving the diamond problem, not a default style choice.

### The dominance rule: an unchanged inherited virtual function is used only if one most-derived override exists

Suppose `code_formatter` inherited a `print()` unchanged from both `cpp_code_generator` and `php_code_generator` (it does not, in the real header — `generate_cpp()`/`generate_php()` are deliberately distinct names there, specifically to sidestep this rule — but the rule itself applies whenever it does happen): that is fine *only* if neither sibling's override disagrees with the other — the single declaration at `code_generator_base` dominates, and both paths to it agree. If one sibling overrides `print()` and the other does not, that override is the one used (it "dominates" the inherited declaration it shadows). MSVC calls this out explicitly:

```
warning C4250: 'code_formatter': inherits 'cpp_code_generator::print' via dominance
```

which is informative, not necessarily a bug — but it is worth reading every time, because it means the compiler picked a winner among several candidates on your behalf.

### Ambiguity returns the moment one branch of the diamond is non-virtual

Virtual inheritance is a property of *each inheritance edge*, not of the base class in isolation. Mixing virtual and non-virtual edges to the same ultimate base reinstates the exact ambiguity virtual inheritance exists to remove:

```cpp
struct Base       { virtual void f_base() = 0; };
struct Derived1 : public          Base { virtual void f1() = 0; }; // non-virtual edge
struct Derived2 : public virtual  Base { virtual void f2() = 0; }; // virtual edge

struct Final : public Derived1, public Derived2
{
    void f_base() override;
    void f1() override;
    void f2() override;
};
```

`Final` still contains two notions of `Base` here — the one reached through `Derived1` (non-virtual) and the shared virtual one reached through `Derived2` — because one edge opted out of sharing. Virtual inheritance only collapses the bases that actually say `virtual`; it is not retroactive and not transitive by accident.

### Multiple inheritance is best managed when the shared base is abstract

Every example above shares a pattern worth stating as a rule: the class being virtually inherited (`code_generator_base`, `Base`) has no data to speak of and exists to be an interface. Virtual inheritance over a stateful, concrete base multiplies the places that single shared state can be reached from, without multiplying the clarity about who is responsible for it. Keep the diamond's apex abstract, and the sharing stays easy to reason about.

### A practical trick that depends on this rule: making a class non-derivable, before `final` existed

C++11's `final` specifier (§1) exists precisely because, before it, forbidding further derivation needed a workaround — and the workaround is a direct application of "the most-derived class alone constructs a virtual base":

```cpp
class finalizer_lock
{
    friend class finalizer;
    finalizer_lock() {} // private, but finalizer is a friend
};

class finalizer : private virtual finalizer_lock
{
public:
    finalizer() {} // fine: finalizer is finalizer_lock's friend
};

class more : public finalizer
{
    // more's implicit default constructor must construct finalizer_lock
    // directly (it is a virtual base), but more is not finalizer_lock's
    // friend -- so more cannot be default-constructed, and in practice
    // cannot be constructed at all. The error surfaces at the point
    // `more` tries to come into existence, not at `more`'s declaration.
};
```

`finalizer` itself constructs normally — it is the one class `finalizer_lock` named as a friend. Any class that tries to derive from `finalizer`, however, inherits `finalizer_lock` as a virtual base too, and by the rule above, *it* — not `finalizer` — would need to construct `finalizer_lock` directly. It cannot: the friendship was never extended to it. [`04_member_pointer/final.h`](04_member_pointer/final.h) implements exactly this, with the failing derivation attempt kept nearby in a commented-out block (following this course's convention for code that must not compile), and [`main.cpp`](04_member_pointer/main.cpp) constructs a real `finalizer` to show the non-forbidden half works fine.

---

## 3. RTTI: `dynamic_cast`, `typeid`, and `type_info`

### `dynamic_cast` needs a polymorphic type — that is what makes "ask the dynamic type" possible at all

C++ does not attach type information to arbitrary objects; it attaches a vtable (and, through it, type information) only to *polymorphic* classes — those with at least one virtual function:

```cpp
struct non_polymorphic { int x; };
struct polymorphic_base { virtual ~polymorphic_base() = default; };
```

```cpp
non_polymorphic np;
// dynamic_cast<non_polymorphic*>(&np); // ill-formed: source is not polymorphic
```

If a class tree never declares a virtual function, `dynamic_cast` on it cannot even be attempted — there is no vtable to carry the runtime type. This is the single fact migrants most often miss (see the migration note below): in C++, "ask the runtime type" is an opt-in cost tied to polymorphism, not an ambient property of every object.

### Upcasting, downcasting, and crosscasting

Given the virtual-inheritance diamond from §2, `dynamic_cast` can move in any direction the actual object supports — this is, in substance, [`03_typeid_rtti/main.cpp`](03_typeid_rtti/main.cpp)'s real `show_casts()`:

```cpp
code_formatter formatter(1);

// upcast: derived -> base, always safe, dynamic_cast not strictly required
cpp_code_generator* cpp_gen = dynamic_cast<cpp_code_generator*>(&formatter);

// crosscast: sideways, between two bases of the same most-derived object
php_code_generator* php_gen = dynamic_cast<php_code_generator*>(cpp_gen);

// downcast: base -> derived, needs the runtime check
code_formatter* cf = dynamic_cast<code_formatter*>(php_gen);
```

Crosscasting — converting from one base subobject to an unrelated sibling base of the same most-derived object — is the case that makes clear `dynamic_cast` is not just "an `static_cast` with a safety check": there is no fixed compile-time offset between `cpp_code_generator*` and `php_code_generator*` in general, so a runtime lookup through the shared RTTI/vtable data is the only way to find the most-derived object and re-derive the target subobject's address.

### Pointer `dynamic_cast` fails quietly; reference `dynamic_cast` fails loudly

The two forms deliberately differ, because a pointer can represent "no object" but a reference cannot:

```cpp
some_unrelated_type* maybe = dynamic_cast<some_unrelated_type*>(ptr);
if (maybe == nullptr)
{
    // conversion is not valid for the actual dynamic type
}
```

```cpp
try
{
    some_unrelated_type& definitely = dynamic_cast<some_unrelated_type&>(obj);
}
catch (const std::bad_cast& e)
{
    // conversion is not valid, and there was no "null reference" to return instead
}
```

Treat a failed pointer `dynamic_cast` and `std::bad_cast` as the same underlying event through two different call shapes — never as "the pointer form is safe, the reference form is risky" or the reverse.

### `typeid` names a type exactly; `type_info::name()` is not a string you may parse

```cpp
#include <typeinfo>

const std::type_info& ti = typeid(formatter);
std::cout << ti.name() << '\n'; // implementation-defined spelling (often mangled)
```

`typeid(expr)` on a glvalue of polymorphic type yields the `type_info` for the object's *dynamic* type (requiring a runtime lookup, exactly like `dynamic_cast`); `typeid(expr)` on anything else — a non-polymorphic type, or a type name directly — yields the `type_info` for the *static* type, resolved at compile time. `type_info::name()`'s returned string is implementation-defined: GCC/Clang typically return a mangled name (demanglable with `abi::__cxa_demangle`, see the diagnostics section below), MSVC a more human-readable one. Compare types with `operator==`, not by comparing `name()` strings.

`type_info::before()` gives an implementation-defined (but consistent within one program run) total ordering over types — it exists so types can be sorted or used as a key, not to express any particular ranking.

### `typeid` of a dereferenced null polymorphic pointer throws `std::bad_typeid`

This is exactly what [`03_typeid_rtti/main.cpp`](03_typeid_rtti/main.cpp)'s `show_typeid()` demonstrates, with a real, deliberately-null `code_generator_base*`:

```cpp
code_generator_base* base_ptr = nullptr;

try
{
    const std::type_info& ti = typeid(*base_ptr); // dereferencing null polymorphic ptr
}
catch (const std::bad_typeid& e)
{
    std::cout << e.what() << '\n';
}
```

This is one of the few places the language turns what looks like a null-pointer dereference into a *defined*, catchable exception rather than undefined behavior — specifically because `typeid` on a glvalue of polymorphic type must read the vptr to find the dynamic type, and the standard requires that attempt to be checked.

### `std::type_index` makes `type_info` usable as a container key

`std::type_info` objects are not copyable or default-constructible, which makes them awkward in associative containers. `<typeindex>` (C++11) wraps a `type_info` reference in a small, copyable, hashable, comparable value:

```cpp
#include <typeindex>
#include <unordered_map>

std::unordered_map<std::type_index, std::string> names;
names[std::type_index(typeid(int))] = "int";
names[std::type_index(typeid(code_formatter))] = "code_formatter";
```

This is the standard way to build a type-keyed registry — factory maps, visitor dispatch tables, serialization type tags — without hand-rolling a wrapper around `type_info`.

### RTTI is a tool for the few places that genuinely need dynamic type — not an escape hatch from virtual dispatch

A chain like this is the most common RTTI misuse:

```cpp
void handle(shape* s)
{
    if (dynamic_cast<circle*>(s))      { /* ... */ }
    else if (dynamic_cast<square*>(s)) { /* ... */ }
    // every new shape subclass means editing this function
}
```

If `handle` already knows the closed set of shapes it must react to, a virtual function on `shape` does the same job at compile-checked call sites, without a chain of runtime casts that silently does nothing when a new subclass is added. RTTI earns its keep in a different kind of situation: generic containers that must recover a concrete type (`std::any`, serialization frameworks), plugin/debugging infrastructure that cannot know the derived types at compile time, and genuinely heterogeneous collections where no single virtual function captures the needed behavior. If the set of cases is closed and known at the call site, reach for virtual dispatch first.

### Visitor and double dispatch emulate multimethods without RTTI

§1 ended on multimethods being deliberately absent from C++. The idiomatic replacement — when a function truly must select behavior based on the dynamic types of *two* related objects — is the Visitor pattern, which turns a single dispatch question (virtual function call) into two:

```cpp
struct visitor
{
    virtual void visit(circle&) = 0;
    virtual void visit(square&) = 0;
};

struct shape
{
    virtual ~shape() = default;
    virtual void accept(visitor&) = 0;
};

struct circle : shape
{
    void accept(visitor& v) override { v.visit(*this); } // dispatch #1: dynamic type of *this
};
```

The first virtual call (`accept`) resolves the dynamic type of the shape; inside it, `v.visit(*this)` is itself a virtual call on `v` that overload-resolves against the now-statically-known `circle&` — the second dispatch. Neither call needs `typeid` or `dynamic_cast`, and the compiler enforces that every `visitor` implementation handles every shape, which a chain of `dynamic_cast` checks cannot do.

[`04_member_pointer/multimethod.h`](04_member_pointer/multimethod.h)/[`multimethod.cpp`](04_member_pointer/multimethod.cpp) build the same idea in a slightly more direct shape than the `visitor`/`accept` names above: `base` declares `multiop(base&)`/`multiop(derived_a&)`/`multiop(derived_b&)` directly on itself rather than through a separate visitor type, and `derived_a`/`derived_b::multiop(base& other)` perform the first dispatch by calling back into `other.multiop(*this)` — the same two-virtual-calls mechanism, with one less named concept to track.

---

## 4. Pointers to class members

### A pointer to a member is not a pointer to an object

```cpp
class member_pointer
{
public:
    virtual ~member_pointer() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

// pointer to a (non-static) member function:
using p_mem = void (member_pointer::*)();
```

(This is `04_member_pointer/member_pointer.h`'s own typedef, renamed here for clarity.) `p_mem` is not a function pointer in the ordinary sense — it cannot be called on its own, has no address you could hand to C code, and is not interchangeable with `void (*)()`. It describes "a member of `member_pointer`", with no object attached yet.

### Think of it as an offset, not an address

The mental model that survives contact with the ABI is: a pointer to a non-static data member is conceptually an *offset* from the start of an object (or of the appropriate base subobject) to that member's storage; a pointer to a non-static member function is similar but must also account for virtual dispatch. Neither is "the address of the member" the way `&object.member` is, because neither has an object yet.

### Obtaining and using a pointer to member

A pointer to member is formed from a fully-qualified member name, and applied to a specific object with `.*` or `->*`:

```cpp
p_mem m = &member_pointer::start; // note: &Class::member, not &object.member

member_derived md;
member_pointer& base_ref = md;

(base_ref.*m)();     // calls through a reference
member_pointer* bp = &md;
(bp->*m)();           // calls through a pointer
```

`m` does not know, and does not care, which concrete object it will eventually be applied to — only that the object is (or derives from) `member_pointer`. The actual function that runs is resolved at the point of the call, through ordinary virtual dispatch if `start` is virtual (as it is here).

### A pointer to a non-static member function cannot be stored in a plain function pointer

```cpp
// void (*plain)() = &member_pointer::start; // ill-formed
```

A non-static member function always needs an object to run against — conceptually a hidden `this` argument — so its type is fundamentally different from an ordinary function pointer's type, not merely a stricter version of it. No implicit or `reinterpret_cast`-free conversion exists between the two.

### Implementation reality: address (or none, for a virtual target), `this`-adjustment, and a vtable index

The language only promises that pointer-to-member-function values can be formed, compared, and called correctly; it says nothing about their representation. In practice, a compiler represents one as a small struct holding, roughly:

* the target function's address — or, if the target is virtual, no address at all;
* an offset to adjust `this` by before the call (relevant with multiple/virtual inheritance, where the subobject the member pointer was formed against is not at offset zero);
* if the target is virtual, a vtable index to look the actual override up through, instead of a fixed address.

This is exactly why `sizeof(p_mem)` is routinely larger than `sizeof(void*)`, and why it can even depend on how polymorphic/multiply-inherited the class is.

### Vendor ABIs for pointer-to-member genuinely differ — this is not a theoretical caveat

Several C++ compiler vendors shipped materially different representations for the same language construct, which is a good illustration of just how much the standard leaves unspecified here:

* Borland and Watcom used a fixed three-word layout: `{ function address, this-adjustment, vtable-index-or-zero }`.
* GNU's g++ used a space-optimized two-word union: the "address" word is either a real function address (even value) or, with its low bit set, a vtable index in disguise (odd value) — plus the `this`-adjustment word.
* Microsoft/Intel's general-case representation adds a fourth word, `vtordisp`, to handle virtual-base adjustment correctly together with virtual dispatch, on top of `{ function address, this-adjustment, vtable-index-or-zero }`.

None of this is observable from portable C++ — you cannot `memcpy` a pointer-to-member between compilers and expect it to mean anything — but it explains why low-level delegate/callback libraries that try to "unpack" a pointer-to-member for speed are inherently compiler-specific. See *Further reading* below for the two classic write-ups.

### Pointer to a *static* member is an ordinary function/object pointer

```cpp
using static_member_pointer = void (*)(); // a plain function pointer

static_member_pointer sm = &member_derived::static_init;
sm(); // call directly, no object, no .*/->*
```

A static member has no implicit object parameter, so a pointer to it is exactly as ordinary as a pointer to a free function — this is the typedef `member_pointer.h` defines right next to `p_mem` specifically to make the contrast visible.

### Templates do not treat pointer-to-member types specially

Partial specialization and type-trait machinery see `T::*` member-pointer types as just another kind of type to pattern-match on explicitly — there is no implicit "this looks like a member pointer, let me unwrap the class and member types for you" behavior unless a template is written to do exactly that:

```cpp
template<class T> struct member_type_of;

template<class C, class M>
struct member_type_of<M C::*>
{
    using class_type  = C;
    using member_type = M;
};
```

Without a specialization written this way, `member_type_of<p_mem>` tells you nothing more than that `p_mem` is "some type".

---
---

## Rules worth keeping in working memory

Everything in this section traces back to the same two distinctions: a *name* ambiguity is not the same problem as a *subobject* ambiguity, and a *compile-time type* is not the same thing as a *runtime type*. Keeping those two separate explains almost every rule above.

> Ambiguity in C++ is resolved by being explicit about which declaration or which subobject you mean — never by hoping the compiler guesses the "obviously intended" one.

### Core rules

1. **Resolve ambiguous names with `using`-declarations, and ambiguous calls with qualified names — access specifiers do not participate in that resolution.**
2. **Reach for virtual inheritance only when the shared base is effectively an abstract interface; it adds real layout and construction-order cost.**
3. **Require a polymorphic (vtable-bearing) type before expecting `dynamic_cast`/`typeid` to answer a dynamic-type question at all.**
4. **Treat a pointer to a non-static member as a compile-time description resolved against an object later, never as an address.**

### Pitfalls at a glance

| Pitfall | What happens | Do instead |
|---|---|---|
| Forwarding a virtual base's constructor argument from an intermediate class | The intermediate class's attempt is silently ignored; only the most-derived class's call runs | Have the most-derived class construct the virtual base explicitly |
| Branching on `dynamic_cast`/`typeid` over a closed, known set of subclasses | Works today, silently does nothing when a new subclass is added | Add a virtual function instead; reserve RTTI for genuinely open-ended cases |
| Assuming every object carries runtime type information | `dynamic_cast`/`typeid(expr)`-for-dynamic-type require a polymorphic (vtable-bearing) type | Give the hierarchy a virtual function (often the destructor) before relying on RTTI |
| Comparing `type_info::name()` strings to identify a type | The string is implementation-defined (often mangled) and not guaranteed stable | Compare `type_info` objects with `operator==`, or key containers with `std::type_index` |
| Trying to store `&Class::member` in a plain function pointer | Ill-formed — a non-static member function pointer is not a function pointer | Keep it as a pointer-to-member and call it through `.*`/`->*`, or use a static member/free function |

### Mental model summary

| Question | Language guarantee | Implementation detail | Application responsibility |
|---|---|---|---|
| Which declaration does an inherited name mean? | Ambiguous unless resolved by `using` or qualification; access control checked afterward | None — purely a lookup rule | Disambiguate deliberately instead of relying on which one "happens" to compile |
| How many copies of a shared base exist? | Exactly one per non-virtual path, or exactly one overall if every path is virtual | Vptr/offset layout chosen by the vendor ABI | Decide, by design, whether the base should be shared |
| Can the dynamic type be recovered safely? | Only for polymorphic types; pointer-cast fails as `nullptr`, reference-cast throws `std::bad_cast` | Vtable/RTTI-table representation, `name()` spelling | Use RTTI only where virtual dispatch cannot express the need |
| What does a pointer to member actually contain? | Only that it can be formed, compared, and called correctly via `.*`/`->*` | Address/offset/vtable-index struct, differing across vendors | Never assume a representation; never serialize one across compilers |

### Review checklist

- [ ] Is every multiple-inheritance ambiguity resolved explicitly (`using`, qualified call), rather than left to compile by coincidence?
- [ ] If a base is inherited virtually anywhere in the hierarchy, does the most-derived class explicitly construct it with the right arguments?
- [ ] Does every `dynamic_cast`/`typeid` call target a genuinely polymorphic type, for a case virtual dispatch could not express more simply?
- [ ] Are pointer-to-member values kept as pointer-to-member values end to end, instead of being reinterpreted, serialized, or compared by representation?

---

## Diagnostics, useful compiler settings and extensions

Most of this section's failure modes are either rejected at compile time (ambiguity, ill-formed casts) or detectable by the right warning flag; the remainder — a wrong assumption about which base's constructor "wins", or treating a failed `dynamic_cast` as impossible — needs a sanitizer or a test that actually exercises the dynamic type.

### Warnings

| Compiler | Flags | What they reveal |
|---|---|---|
| GCC | `-Wall -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -Wreorder` | Missing virtual destructor on a polymorphic base, a shadowed (not overridden) virtual, constructor-order mismatches against the initializer list |
| Clang | `-Wall -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -Wreorder -Winconsistent-missing-override` | Same as GCC, plus catching a derived override that drops `override` |
| MSVC | `/W4` (surfaces `C4250` dominance, `C4266` hidden virtual) | Which base wins a dominance tie; non-virtual functions that hide, rather than override, a base's virtual |

### Sanitizers and runtime checks

| Tool | Setting | Best use |
|---|---|---|
| UndefinedBehaviorSanitizer, vptr check | `-fsanitize=vptr` (Clang/GCC; requires linking as C++ and is incompatible with `-fno-rtti`) | Catches calling a virtual function, or `dynamic_cast`/`typeid`-ing, through a vptr whose object's lifetime has not begun, has ended, or does not match the static type claimed |
| UndefinedBehaviorSanitizer, general | `-fsanitize=undefined` | Several other categories; does not replace the `vptr` check above, which moved out of the `undefined` group in recent Clang releases |
| AddressSanitizer | `-fsanitize=address` | Use-after-free/scope issues around objects that participate in multiple/virtual inheritance |

### Semantics-changing options

| Option | Effect | Recommendation |
|---|---|---|
| `-fno-rtti` (GCC/Clang) / `/GR-` (MSVC) | Disables `dynamic_cast` (for polymorphic targets) and `typeid`'s dynamic-type form; `typeid` on a static type and `std::type_info` as a type generally still compile | Only for environments that have deliberately opted out of RTTI's size/runtime cost (some embedded/kernel targets); never combine with `-fsanitize=vptr` |
| `/vmb`, `/vmg`, `/vmm`, `/vms`, `/vmv` (MSVC) | Choose the pointer-to-member representation MSVC uses, from "best case for the hierarchy actually seen" to "general case that works even for an as-yet-incomplete type" | Leave at the default (`/vmb`) unless a header genuinely needs to declare a pointer-to-member before the class is fully defined |
| `-fstrict-vtable-pointers` (Clang) | Lets the optimizer assume a vptr does not change except through construction/destruction | Safe under correct code; can mask the exact bugs `-fsanitize=vptr` is meant to catch if both are reasoned about loosely |

### Compiler extensions and standard traits worth knowing here

| Facility | Compilers | Use | Portable approach |
|---|---|---|---|
| `std::has_virtual_destructor<T>`, `std::is_polymorphic<T>` | Standard (`<type_traits>`) | Check, at compile time, whether a type is safe to `dynamic_cast`/`typeid`-for-dynamic-type on, or safe to `delete` through a base pointer | Prefer these over compiler-specific introspection; they are already portable |
| `abi::__cxa_demangle` | GCC/Clang (Itanium C++ ABI) | Turn a mangled `type_info::name()` string into a readable one for logging/debugging | Treat as debug-only tooling, never parse the demangled string for program logic |
| `__attribute__((visibility(...)))` / `/EHsc` RTTI interaction | GCC/Clang / MSVC | Cross-DSO RTTI comparisons can fail (`typeid`/`dynamic_cast` returning "different type") when visibility or runtime-type-info export settings differ between shared libraries | Export RTTI consistently for any polymorphic type crossing a shared-library boundary |

### Libraries and tooling beyond the standard

| Tool | Purpose | When useful |
|---|---|---|
| clang-tidy (`cppcoreguidelines-virtual-class-destructor`, `misc-noexcept-move-constructor`, etc.) | Static analysis tuned to exactly these inheritance pitfalls | Reviewing new base classes and override hierarchies |
| Compiler Explorer | Inspect generated vtables/vtable layouts and pointer-to-member representations across vendors | Verifying the ABI claims in §4 against the compiler actually in use |
| `nm -C` / `c++filt` (GCC/Clang toolchains) | Demangle symbol names, including `type_info` symbols, from built binaries | Debugging cross-DSO RTTI mismatches without a debugger session |

---

## Standards timeline

### C++98/03: the original model

`dynamic_cast`, `typeid`, `std::type_info`, virtual base classes, the dominance rule, and pointer-to-member syntax (`T::*`, `.*`, `->*`) are all original C++ facilities. `type_info::before()` and `type_info::name()` already existed, with `name()`'s representation already implementation-defined.

### C++11: sharper wording, `override`/`final`, and `std::type_index`

* `dynamic_cast` and `typeid`'s operand rules were restated in terms of glvalues (rather than only lvalues) to work correctly with the newly introduced rvalue references; a related defect report (CWG 1269) fixed a gap where the runtime check had not been performed for an xvalue operand converted to an rvalue-reference target.
* `std::type_info::operator==`/`operator!=` became `noexcept`, and `type_info` gained `hash_code()`, making it directly usable with hash-based containers.
* `<typeindex>` and `std::type_index` were added specifically to make `type_info` usable as a key in ordered and unordered associative containers (see §3).
* The contextual keywords `override` and `final` let a derived class's intended override — exactly the kind of override the dominance rule in §2 cares about — be checked by the compiler instead of assumed.

### C++14: no changes tracked for this topic

Nothing specific to multiple/virtual inheritance, `dynamic_cast`, `typeid`, `type_info`, or pointer-to-member shipped in C++14; the facilities in this section were stable between C++11 and C++14.

### C++17: formalized temporary materialization for `typeid`

`typeid`'s rule for a prvalue operand was restated in terms of C++17's temporary materialization (the operand, if a prvalue, is materialized into a temporary, which must be destructible where the `typeid` expression appears). This is a wording clarification following C++17's broader value-category rework, not a behavior change for ordinary code.

### C++20: `constexpr` reaches `dynamic_cast` and `typeid`; `type_info::operator!=` is synthesized

* The `__cpp_constexpr == 201907L` feature means polymorphic `typeid` and `dynamic_cast` may now appear in a constant expression, provided the dynamic type involved is itself knowable at compile time during that evaluation.
* As part of C++20's defaulted-comparison rules, `std::type_info::operator!=` is no longer separately specified; it is synthesized from `operator==` instead.

### C++23: `type_info::operator==` becomes `constexpr`

`std::type_info::operator==` is `constexpr` since C++23, consistent with the C++20 work that let `typeid` itself appear in constant expressions.

### C++26: reflection is a distinct, compile-time mechanism — not a change to RTTI

C++26 adds a static reflection facility (`std::meta`, the `^^` reflection operator, voted in via P2996), which lets code introspect types, members, and other language constructs *at compile time*, producing values of type `std::meta::info`. This is conceptually adjacent to "asking what something is" but is a different mechanism from runtime RTTI: no verified change to `dynamic_cast`, `typeid`, `std::type_info`, or `std::type_index` themselves ships as part of it. Readers should not conflate "C++26 has reflection" with "RTTI changed in C++26" — they solve related-sounding but genuinely different problems (compile-time introspection vs. runtime dynamic-type recovery).

No standard version introduced a portable representation for pointer-to-member values (§4); that remains an unspecified, vendor-chosen ABI detail across every standard listed above.

---

## Migration note for Java / Python / C# developers

The single biggest adjustment is that in Java and C#, *every* object inherently carries full runtime type metadata — there is no "did you remember to make this polymorphic" step. In C++, that metadata exists only for classes with at least one virtual function; a plain struct has none, and `dynamic_cast`/the dynamic-type form of `typeid` simply cannot be used on it.

### Where the hierarchy model itself differs

| Concept | Java / C# | Python | C++ reality |
|---|---|---|---|
| Multiple inheritance of classes | Not allowed; only one base class. Interfaces (Java `interface`, C# `interface`) supply multiple *contracts* with no shared state | Allowed directly, resolved by method resolution order (C3 linearization) — a genuinely different mechanism from C++'s named-ambiguity-plus-virtual-inheritance model | Allowed directly; ambiguity is resolved per-name (`using`) or per-call (qualification), and a shared base is deduplicated only if every inheriting edge is marked `virtual` |
| The "diamond problem" | Does not arise for classes (single base); can arise for *default interface methods* in Java/C#, resolved by requiring an explicit override | Arises, and MRO deterministically linearizes it — there is no equivalent of C++'s virtual-vs-non-virtual base distinction | Arises for both data and virtual dispatch; solved (or not) explicitly via virtual inheritance, not by any automatic linearization |
| Runtime type metadata | Always present on every object | Always present on every object (`type(obj)`) | Present only for polymorphic (vtable-bearing) types |

### Dynamic type checks look similar but mean something narrower in C++

`instanceof` (Java), `is` (C#), and `isinstance` (Python) all answer "is this object's runtime type compatible with that type?", and they always have an answer because the runtime always has type metadata to consult. `dynamic_cast`/`typeid(expr)` answer the same question in C++, but:

* they require a polymorphic source type to even compile against a dynamic-type question (§3) — there is no managed-runtime fallback;
* a pointer `dynamic_cast` failure is a `nullptr`, and a reference `dynamic_cast` failure is a thrown `std::bad_cast` — two different call shapes for the same underlying event, not one uniform exception the way a failed cast typically is in those languages;
* C++ additionally distinguishes an object's *static* type (the declared type of the expression, resolved at compile time, used for overload resolution and non-virtual calls) from its *dynamic* type (what `dynamic_cast`/`typeid` report). Java, C#, and Python have a comparable static-type concept for overload resolution, but every object is still fully runtime-introspectable regardless — there is no "this type opted out of having a dynamic type" possibility the way a non-polymorphic C++ class does.

### Pointers to members have no real equivalent

Java's `Method`/`MethodHandle`, C#'s `Delegate`/`MethodInfo`, and Python's bound methods/`getattr` all let code refer to "a method, to be invoked on some object later" — but all of them resolve through a runtime reflection/metadata table, carry name- and signature-lookup overhead, and are built to tolerate the open-ended "any object, any method" case. A C++ pointer-to-member is resolved entirely at compile time against one specific class, calls through an ordinary (if ABI-specific) offset/vtable-index mechanism with no name lookup at call time, and cannot be formed against a member that does not exist for that exact class. Reaching for a managed-language method-handle mental model when reading `T::*` syntax is the most common source of confusion for migrants — expect "an offset the compiler already resolved", not "a handle the runtime will look up".

### Common wrong assumption to retire

The most expensive one: assuming that because a cast to a derived type "usually works" when tested against today's class hierarchy, it will still return a meaningful answer once a sibling class is added, or that `dynamic_cast` is as universally safe to call as `instanceof`/`isinstance` are — without first checking that the source type is actually polymorphic and that a failure (null, or `std::bad_cast`) is handled wherever the cast is attempted.

---

## Further reading

### Standard and language reference

* [language/derived_class](https://en.cppreference.com/w/cpp/language/derived_class) — base-class lists, virtual base classes, the dominance rule
* [language/using_declaration](https://en.cppreference.com/w/cpp/language/using_declaration) — resolving inherited name ambiguity
* [language/dynamic_cast](https://en.cppreference.com/w/cpp/language/dynamic_cast)
* [language/typeid](https://en.cppreference.com/w/cpp/language/typeid)
* [types/type_info](https://en.cppreference.com/w/cpp/types/type_info)
* [types/type_index](https://en.cppreference.com/w/cpp/types/type_index)
* [language/pointer](https://en.cppreference.com/w/cpp/language/pointer) — includes pointer-to-member syntax and semantics
* [language/operator_member_access](https://en.cppreference.com/w/cpp/language/operator_member_access) — `.*` and `->*`

### Books and classic write-ups

* Bjarne Stroustrup, *The Design and Evolution of C++* — the multimethods discussion quoted in §1
* Raymond Chen, ["Pointers to member functions are very strange animals"](https://devblogs.microsoft.com/oldnewthing/?p=40713) — The Old New Thing
* Don Clugston, ["Member Function Pointers and the Fastest Possible C++ Delegates"](https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible) — CodeProject, the vendor-by-vendor ABI survey §4 summarizes

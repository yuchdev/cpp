# 04. Inheritance and Polymorphism

Inheritance and polymorphism are not one feature either. They are a family of related mechanisms that answer different questions:

* does a derived class reuse a base class's *interface*, its *implementation*, or both?
* when I call a member function through a base pointer or reference, which function body actually runs?
* is that decision made by the compiler at the call site, or by the object at run time?
* does converting a derived object to its base type lose information?
* can a type be named but never instantiated on its own?
* does inheriting from a class mean "is-a", "has-a", or just "reuses some code"?

Confusing those questions is how innocent-looking class hierarchies turn into Liskov Substitution Principle violations, sliced objects, and crashes through non-virtual destructors.

The examples in this directory are:

1. [`01_simple_inheritance/classes.h`, `classes.cpp`](01_simple_inheritance/classes.h) — an `Employee`/`Manager` hierarchy showing plain, non-virtual inheritance: what is and is not inherited, name hiding, explicit base-class calls, and (via `TemporaryEmployee`) C++11 inheriting constructors as the contrasting case to writing one by hand.
2. [`02_polymorphism/virtual.h`, `virtual.cpp`](02_polymorphism/virtual.h) — static vs. dynamic binding, virtual destructors, pure virtual destructors, default-argument binding, covariant return types, the vtable/vptr model, and `override`/`final` (including the one declaration that must *not* compile with `override`, and the function- and class-level forms of `final`, both proven with real negative-compile cases kept behind `#if 0`).
3. [`03_upcasting/currency.h`, `currency.cpp`](03_upcasting/currency.h) — a `Currency`/`Usd` hierarchy showing upcasting, downcasting, reference polymorphism for free functions, and when operators are worth making virtual.
4. [`04_interfaces/abstract.h`, `abstract.cpp`](04_interfaces/abstract.h) — a `shape`/`circle`/`rectangle`/`sphere`/`cube` hierarchy showing abstract classes, pure virtual interfaces, and the square/rectangle anti-pattern.
5. [`05_adapter/adapter.h`, `adapter.cpp`](05_adapter/adapter.h) — the Adapter design pattern built from private and protected inheritance over a `deque`-backed type.

**A note on build status.** All five directories build and run. They originally pre-dated being wired into the CMake build — each `main.cpp` either referenced another example's headers by bare name, exercised the wrong example's classes entirely, or simply didn't exist — a leftover from an incomplete split of what used to be one flat set of examples. Every supporting class (`Employee`/`Manager`, `virtual.h`'s binding demos, `Currency`/`Usd`, the `shape` hierarchy, `adapter.h`'s four adapter variants) was already correct, so the repair was a matter of writing a real `main()` driver for each leaf rather than inventing any new class design. Everything below is grounded in that actual, working code.

A useful first map is:

| Mechanism | Governs | Decided |
|---|---|---|
| Non-virtual inheritance | Shared implementation, shared storage layout | At compile time, by static type |
| Virtual functions | Shared interface with per-type behavior | At run time, through the vtable |
| Pure virtual functions | Interface only, no implementation guaranteed | Must be supplied by some concrete derived class |
| Private/protected inheritance | "Has-a" with reusable implementation | At compile time; not part of the public `is-a` hierarchy |

The shortest reliable mental model is:

> Non-virtual members are inherited as compiled-in implementation. Virtual members are inherited as a contract, fulfilled by whichever object is actually there at run time.

The rest of this document expands that distinction.

---

## 1. Simple inheritance: what is and is not inherited

### A derived class is one block of memory, not a diagram

`Manager : public Employee` reads as if the compiler keeps the relationship around at run time. It mostly does not. From the compiler's point of view, a `Manager` object is a single monolithic block of memory containing the base's data members followed by (or interleaved with) the derived class's own:

```cpp
class Employee
{
public:
    Employee(const char* name);
    ~Employee();
    void show_name();
protected:
    int salary_;
private:
    std::string name_;
    short department_;
};

class Manager : public Employee
{
public:
    Manager(const char* name, short level, int salary);
    ~Manager();
    void show_name();
    void show_level();
protected:
    int salary_;          // shadows Employee::salary_, see below
private:
    std::vector<Employee*> group_;
    short level_;
};
```

There is no per-object inheritance metadata and no memory overhead beyond the members actually declared. The inheritance *tree* matters to the compiler only when it performs type conversions — deciding whether a `Manager*` converts to an `Employee*`, and at what offset.

### Constructors, destructors, and operators are never inherited

A derived class's constructor and destructor are always either compiler-generated or written explicitly — they are never "inherited" the way a member function can be:

```cpp
Manager::Manager(const char* name, short level, int salary)
    : Employee(name), level_(level), salary_(salary)
{
}
```

If the base has no default constructor, the derived constructor must call a base constructor explicitly in its member-initializer list, as above. Destructors run automatically in reverse order of construction: the most-derived destructor body runs first, then each base destructor up the chain, with no explicit call needed.

"Never inherited" has had one escape hatch since C++11: a derived class can explicitly *opt in* to a base's constructor set with `using Base::Base;` instead of writing forwarding constructors by hand. `classes.h`'s `TemporaryEmployee` does this, right alongside `Manager`'s hand-written constructor, as a direct contrast (Standards timeline, below, has the mechanics).

`operator=` and friend functions follow the same rule: a derived class does not inherit its base's assignment operator or friends. If you need one, write it, and have it call the base's version explicitly.

### Shadowing: a derived member of the same name hides, it does not merge

`Manager` above declares its own `salary_`, with the same name as `Employee::salary_`. This is legal and almost never what you want:

```cpp
Manager::Manager(const char* name, short level, int salary)
    : Employee(name), level_(level), salary_(salary)
{
    // Employee::salary_ was never touched by this constructor —
    // it is a *separate* subobject that happens to share a name.
    std::cout << "Employee::salary_=" << Employee::salary_ << '\n';
}
```

Two distinct `int` subobjects now exist inside every `Manager`. Ordinary unqualified lookup inside `Manager`'s members finds the derived one; the base one is still there, fully addressable as `Employee::salary_`, just hidden from unqualified name lookup. The same shadowing rule applies to member functions:

```cpp
void Manager::show_name()
{
    std::cout << "Manager: ";
    Employee::show_name();   // explicit call to the hidden base version
}
```

`Manager::show_name()` does not override anything in the polymorphic sense (nothing here is `virtual` yet) — it simply declares a function with the same name that hides the base one for callers who have a `Manager` (or who call through `Manager::`). A caller holding an `Employee&` that actually refers to a `Manager` still gets `Employee::show_name()`, because this binding is resolved by static type.

### The Liskov Substitution Principle and why slicing matters

Public inheritance is supposed to mean "is-a": any code that works with a `Base&` or `Base*` should keep working, unchanged, when handed any `Derived` object, without downcasting or type-checking to special-case it. That is the Liskov Substitution Principle, and it is a design discipline the compiler does not enforce.

What the compiler *does* enforce is layout, and that is exactly why passing polymorphic objects by value is dangerous:

```cpp
void describe(Employee e);   // by value — don't do this with polymorphic types

Manager m("Grace", 3, 95000);
describe(m);                 // only the Employee part of m is copied
```

`describe` receives a plain `Employee`, built by invoking `Employee`'s copy constructor on the `Employee` subobject of `m`. The `Manager`-specific data (`level_`, `group_`) is silently dropped — this is *object slicing*. The fix is to take a reference or pointer, never a by-value `Employee`, whenever the parameter might legitimately be a derived type.

### `public`, `private`, and why most hierarchies should use classes

Public inheritance expresses "is-a" and participates in implicit upcasting. Private inheritance (covered in depth in [section 5](#5-the-adapter-pattern-via-private-and-protected-inheritance)) expresses "has-a, implemented in terms of" and deliberately does *not* let outside code treat the derived object as a base object.

Either kind of inheritance is legal on a `struct`, but by convention `struct` is reserved for plain data aggregates and hierarchies are built from `class`. This is a convention, not a language rule — the only actual difference between `struct` and `class` is the default access specifier and the default inheritance kind.

A base class's data should usually be `private`, accessed only through member functions, even in a hierarchy. `protected` data is a standing invitation for every current and future derived class to depend on the base's internal representation; prefer `protected` member *functions* when a derived class genuinely needs controlled access to base internals.

### `using` can re-expose a base name

A `using`-declaration inside a derived class pulls one specific base member into the derived class's own visible interface, which is particularly useful after `private`/`protected` inheritance has hidden everything (see [section 5](#5-the-adapter-pattern-via-private-and-protected-inheritance)):

```cpp
class limited_manager : private Employee
{
public:
    using Employee::show_name;   // re-opens just this one member
};
```

A `using`-declaration cannot reach past access control to open a genuinely `private` base member it has no access to; it can only re-expose something the derived class could already see.

---

## 2. Virtual functions, dynamic dispatch, and the vtable model

### Static binding: the call site decides, once, forever

Without `virtual`, a member function call is resolved purely from the static type of the expression — the type the compiler sees written down, not whatever object happens to be there at run time:

```cpp
struct Base
{
protected:
    int func(int i) { return 2 * i; }
public:
    int call_protected_function(int i) { return func(i) + 1; }
};

struct Derived : Base
{
protected:
    int func(int i) { return i * i; }   // hides Base::func, does not override it
};
```

`Base::call_protected_function` always calls `Base::func`, even when invoked through a `Derived` object, because `func` is not `virtual` and the call is compiled against `Base`'s definition. `Derived::func` is a separate, unrelated function that merely shares a name — this is the same shadowing rule from section 1, not polymorphism.

### Dynamic binding: the object decides, at every call

Mark the function `virtual` and the story inverts:

```cpp
struct BaseVirt
{
protected:
    virtual int func(int i) { return 2 * i; }
public:
    int call_protected_function(int i) { return func(i) + 1; }
};

struct DerivedVirt : BaseVirt
{
protected:
    int func(int i) override { return i * i; }
};
```

Now `call_protected_function`, still compiled once against `BaseVirt`, dispatches to whichever `func` belongs to the object's actual dynamic type. `virtual`-ness is inherited: once a base declares a function virtual, every override down the hierarchy is virtual too, whether or not the keyword is repeated. Repeating `virtual` in a derived override is legal but discouraged — write `override` instead, so the compiler verifies you actually matched a base signature rather than accidentally declaring an unrelated overload.

An override's signature must match the base's exactly, except that the return type may legally differ in the single, specific way covered below (covariant returns). If the parameter list differs at all, you have not overridden anything — you have hidden the base function with an unrelated overload, exactly as in the non-virtual case, and `override` is what catches that mistake at compile time instead of at a confused debugging session.

`final` (also C++11) goes one step further than `override` in the opposite direction: written on a specific virtual function, it forbids any *further* override of that one function in any class deriving from this one; written directly after a class name, it forbids deriving from the class at all. Both are independent of `override` — a function can be `override final` (confirms it overrides something, *and* stops the chain here), and marking one function `final` has no effect on any other virtual function in the same class, which still overrides normally further down the hierarchy.

### Polymorphic classes need a virtual destructor

If a class is meant to be deleted through a base pointer, its destructor must be `virtual`, full stop:

```cpp
struct A
{
    A();
    virtual ~A();
    virtual std::ostream& f() const;
};

struct B : A
{
    B();
    ~B() override;
    std::ostream& f() const override;
};
```

Deleting a `B` through an `A*` whose destructor is *not* virtual only ever runs `~A()` — `~B()` never executes, and any resources `B` owns leak. This is listed as one specific case of generally undefined/incorrect behavior for polymorphic deletion, not merely a performance detail. If a class has any virtual function, give it a virtual destructor as a reflex, even if that destructor's body is empty.

### Virtual calls from constructors and destructors use only static binding

This is the single most counter-intuitive rule in the whole topic. While a base class constructor or destructor is running, the object is, respectively, not yet a derived object or no longer one — so a virtual call made during that window dispatches to the **currently executing class's own version**, never to an override further down:

```cpp
A::A() { std::cout << "A(): "; f(); }   // always calls A::f() here,
A::~A() { std::cout << "~A(): "; f(); } // even if *this is actually a B
```

Constructing a `B` prints `A::f` during `A`'s constructor (the `B` part does not exist yet), then `B::f` during `B`'s own constructor. Destroying it prints `B::f` during `~B()`, then `A::f` during `~A()` (the `B` part is already gone). The vtable pointer itself effectively tracks construction progress: it is set to point at the currently-constructing class's own vtable as each base/derived layer's constructor runs, and stepped back down as each destructor runs.

### Pure virtual destructors make a class abstract without touching inheritance

A destructor can be declared pure virtual and still have a body:

```cpp
struct VBase
{
    VBase();
    virtual ~VBase() = 0;   // pure, but may still be defined
    void pure_default();    // one way to implement a would-be pure virtual: give it a body
};

VBase::~VBase() { /* ... */ }   // legal: the = 0 only forbids direct instantiation
```

`VBase` cannot be instantiated directly because of the `= 0`, which is exactly what makes a class abstract. But a destructor — pure or not — is never inherited (section 1), so a derived class that omits its own destructor does **not** become abstract by inheriting `VBase`'s pure virtual destructor; the compiler-generated destructor for the derived class is a perfectly ordinary, concrete one. Abstractness comes from *having an unsatisfied pure virtual member somewhere in the class*, not from the destructor specifically, and a derived class only remains abstract if some other pure virtual member is still unimplemented.

There are, in general, four ways to "implement" a pure virtual function so that a hierarchy compiles: give it a body anyway (as above) and call it a default implementation; provide a common partial implementation, typically `protected`, for derived classes to opt into via an explicit qualified call; implement it to unconditionally throw, documenting that some derived branch must override it before the type is usable; or simply leave it pure and never instantiate the class without an override. Section 4 returns to this with a worked interface example.

### Default arguments on virtual functions are bound statically

Default arguments are not stored per-override in the vtable — there is no slot for them — so they are resolved from the **static type of the call expression**, not the dynamic type of the object:

```cpp
struct Base1
{
    virtual void g(int i = 10) { std::cout << "Base1::g " << i << '\n'; }
};

struct Derived1 : Base1
{
    void g(int i = 20) override { std::cout << "Derived1::g " << i << '\n'; }
};

Derived1 d;
Base1& b = d;

b.g();   // prints "Derived1::g 10" — dynamic dispatch picks Derived1::g,
         // but the *default argument* comes from Base1's declaration
```

The function that actually runs is still chosen dynamically. Only the default-argument value is static. Relying on this is confusing to read; prefer repeating an explicit argument, or avoiding default arguments on virtual functions altogether.

### Covariant return types

An override's return type may differ from its base's in one specific, legal way: if the base returns `Base&` or `Base*`, an override may return `Derived&` or `Derived*` for any `Derived` that is a public, unambiguous base-reachable type of the override's declaring class:

```cpp
struct Base1
{
    virtual A* pf(int i = 1) { return new A(); }
};

struct Derived1 : Base1
{
    B* pf(int i = 2) override { return new B(); }   // B derives from A — legal covariant return
};
```

This is why "virtual constructor" patterns — a virtual `clone()` member that each derived class overrides to return a pointer to its own concrete type — are idiomatic: the override's more specific return type is both legal and useful to callers who know the concrete type.

### Overriding can relax access, but not change what the vtable slot does

The access specifier of a virtual function in a derived override is independent of the base's:

```cpp
struct Base1
{
protected:
    virtual void open_me() const { /* ... */ }
public:
    virtual void hide_me() const { /* ... */ }
};

struct Derived1 : Base1
{
public:
    void open_me() const override { /* ... */ }   // now public in Derived1
protected:
    void hide_me() const override { /* ... */ }    // now protected in Derived1
};
```

Access control is checked against the **static type** used to name the call, not the dynamic type of the object and not the override's own access specifier. A caller holding a `Base1&` can still call `hide_me()` even though `Derived1::hide_me` is `protected` — because as far as that caller's compiler is concerned, it is calling the `public` member declared in `Base1`, and which override's *body* actually runs at run time has no bearing on that compile-time access check.

### You can "virtualize" an external function through a base reference

A free function cannot itself be virtual, but if it takes its polymorphic argument by reference (or pointer) to the base type, any virtual call it makes inside behaves polymorphically on the caller's behalf:

```cpp
inline std::ostream& operator<<(std::ostream& out, A& value)
{
    return value.f();   // value.f() dispatches dynamically even though operator<< is not virtual
}
```

`operator<<` itself is resolved by ordinary overload resolution exactly once; the polymorphism lives entirely in the `value.f()` call inside it.

### What virtuality costs

None of this is free: a polymorphic class carries a hidden vtable pointer (vptr) per object — one pointer, not one per virtual function — initialized by the constructor essentially as its first act. There is ordinarily one statically-allocated vtable per polymorphic class, built once at compile time (the whole hierarchy is visible to the compiler, so it can lay all the tables out in advance), and a virtual call becomes an indirect call through that table rather than a direct jump:

```text
(*object->vptr[slot])(object, args...)
```

The exact representation is entirely ABI-defined: typical Itanium-ABI compilers (GCC, Clang) place the vptr at the start of the object and store RTTI as (often) the first entry of the vtable itself; MSVC's ABI differs in its own documented ways. With multiple inheritance from more than one polymorphic base, a single object can end up with more than one vptr, and the vtable additionally has to carry `this`-pointer adjustment offsets so a call through one base's vtable slot still finds the right subobject to operate on. None of this is something portable code should rely on structurally — but knowing it exists explains why `sizeof` grows by one pointer the moment a class gains its first virtual function, and why a virtual call is strictly more expensive than a direct one. Don't mark a function `virtual` "just in case" — pay the cost when you actually need per-type behavior, not speculatively. As a rule of thumb: reach for polymorphism when *behavior* must differ per type at run time; reach for templates when it is the *stored type*, not the behavior, that varies, and the set of types is known at compile time.

One corner case worth knowing: `inline` is simply ignored on a virtual function. It is not an error — the keyword just has no effect, because a virtual function must have a stable address to put in the vtable, which is fundamentally incompatible with being expanded inline at every call site through a base pointer. (A compiler can still inline a particular *call* that it can prove is non-virtual at that call site, e.g. a call made through a concrete, final object — that is a code-generation decision, separate from the `inline` keyword's own meaning.)

---

## 3. Upcasting, downcasting, and operator virtuality

### Upcasting is implicit and (usually) a smell when done explicitly

Converting a derived object, pointer, or reference to its base type — upcasting — is always implicit and always safe, because every derived object genuinely contains a base subobject:

```cpp
class Currency
{
public:
    Currency();
    Currency(double amount);
    virtual ~Currency();   // section 2's rule applies the moment operator*= below is virtual
    Currency& operator+=(const Currency& other);
    virtual Currency& operator*=(const Currency& other);
    // ...
protected:
    double amount_;
};

class Usd : public Currency
{
public:
    Usd(double amount);
    Usd& operator=(const Currency& other);   // "downcasting" assignment, see below
    Usd(const Currency& other);              // "downcasting" constructor, see below
};
```

A `Usd` converts to `Currency` for free, anywhere a `Currency` is expected. Writing an explicit upcasting constructor or `operator=` that takes a *base* reference and constructs/assigns a *derived* object — as `Usd(const Currency&)` and `Usd::operator=(const Currency&)` do above — is technically legal but should be treated as a design smell in production code: it invites exactly the kind of implicit base-to-derived conversion that normally requires an explicit, deliberate downcast. When you do write one, call the base's own constructor or `operator=` explicitly, in functional-call form, rather than re-implementing its logic:

```cpp
Usd::Usd(const Currency& other) : Currency(other) {}

Usd& Usd::operator=(const Currency& other)
{
    Currency::operator=(other);   // explicit functional-form call to the base operator
    return *this;
}
```

### Reference polymorphism lets free operators skip virtuality entirely

`Currency`'s free `operator+`, `operator-`, `operator*`, and `operator/` are friend functions that take `const Currency&` parameters and are implemented purely in terms of the member compound-assignment operators:

```cpp
Currency operator+(const Currency& a, const Currency& b)
{
    Currency result(a);
    return result += b;
}
```

None of this needs to be virtual. Because the parameters are references, a `Usd` passed in is accepted *as* a `Currency` by the signature, and `result += b` dispatches to whichever `operator+=` is appropriate — which, if `operator+=` itself is non-virtual, simply means "the one defined for the static type `Currency`", and if it *is* virtual, dispatches dynamically. Either way, the free function itself never needs to know or care. This is the general rule for operators in a hierarchy: if the behavior in every derived class is the same as the base's, do not make the operator virtual at all — reference/pointer parameters already give you polymorphism for the member call inside, for free.

### When an operator is worth making virtual

`Currency::operator*=` and `operator/=` are declared `virtual` specifically so that `Usd` can supply different behavior:

```cpp
virtual Currency& operator*=(const Currency& other);   // Currency's own, ordinary math

// in Usd:
Currency& operator*=(const Currency& other) override
{
    Currency::operator*=(other);   // still delegates, just as an illustration
    return *this;
}
```

This is worth doing only when a genuinely complex hierarchy needs different arithmetic semantics per derived type — a rare situation in practice. Making every operator virtual "to be safe" mixes the costs from section 2 (vptr, indirect calls, one more thing to get right in every override) into code paths that usually do not need per-type behavior at all.

### `dynamic_cast` is how you ask an object what it actually is

Where upcasting is implicit, the reverse direction — treating a base reference or pointer as some more specific derived type — requires an explicit, checked cast. `static_cast` performs that conversion blindly, trusting the programmer and never inspecting the actual object; `dynamic_cast` instead examines the object's real, dynamic type (through the same RTTI that lives alongside the vtable) and only succeeds if the requested type is actually compatible:

```cpp
Currency& ref = get_some_currency();

if (Usd* usd = dynamic_cast<Usd*>(&ref))
{
    // ref really does refer to a Usd
}
```

A `dynamic_cast` to a pointer type that fails yields `nullptr` rather than failing to compile or invoking undefined behavior; a `dynamic_cast` to a reference type that fails throws `std::bad_cast`. `dynamic_cast` requires the source type to be polymorphic (it needs that RTTI to inspect) and can convert between sibling types in a hierarchy, or from a virtual base down into a specific derived class, situations `static_cast` simply cannot handle because `static_cast` never looks at the object at all. A `dynamic_cast<void*>` on a polymorphic glvalue is also a legal, useful way to recover the address of the *complete* object, which need not be the same address as the subobject you are currently looking at through a base pointer in a multiple-inheritance hierarchy.

From a design standpoint, a `dynamic_cast` check is best read as "does this object provide a given interface?" rather than "what exact class is this?" — and `typeid`, returning a `std::type_info` (`<typeinfo>`) that can be compared or hashed through `std::type_index`, is the tool for the latter, narrower question. Both are legitimate tools; both are also exactly the kind of type-checking the Liskov Substitution Principle in section 1 asks you to avoid needing. Reach for RTTI only when the alternative — adding a virtual function that expresses the real question you're asking — genuinely does not fit.

---

## 4. Abstract classes and interfaces

### A pure virtual function declares interface without implementation

The `shape` hierarchy is built around one idea: a base class that defines *what* every shape can do, with no commitment to *how*:

```cpp
class shape
{
public:
    virtual void draw() = 0;
protected:
    shape() {}
    virtual ~shape() {}
};
```

`shape` cannot be instantiated — any attempt produces a compile error, because `draw()` has no implementation. This is the point: `shape` is a pure contract. Its constructor and destructor are deliberately `protected`, which reinforces "you cannot make one of these directly" even beyond what the pure virtual function alone already guarantees, and documents that intent for a reader who has not scrolled down to check which functions are pure.

### Keep data out of the interface layer

`shape` itself holds no data members at all, and the comment in the source is explicit about why: a base class that accumulates "useful" fields over time quietly becomes a dependency magnet, and every derived class inherits storage it may not need. The hierarchy here deliberately separates concerns into layers:

```cpp
class shape_info : public shape       // adds color data, still no draw() — still abstract
{
protected:
    int shape_color_;
    int border_color_;
};

class shape2d : public shape_info     // adds the 2D-specific interface
{
public:
    virtual void rotate(point& p) = 0;
};

class circle : public shape2d         // first concrete, leaf class
{
public:
    void draw() override;
    void rotate(point& p) override;
private:
    point center_;
    int radius_;
};
```

`shape_info` does not need `shape::draw()`, so it neither implements nor re-declares it — which means `shape_info`, and everything derived from it that doesn't supply `draw()`, remains abstract automatically. Only `circle`, `rectangle`, `sphere`, and `cube` — the leaves — implement both `draw()` and the appropriate `rotate()` overload and become concrete. As a rule, non-leaf classes in a hierarchy like this should stay abstract; letting an interior class be instantiable usually means it is quietly also doing a concrete class's job.

### The square/rectangle trap is a Liskov violation, not a syntax error

The comment at the top of `abstract.h` calls out the classic failure mode by name: a `square` must not inherit from a `rectangle` that exposes independent width/height setters, because any code that treats a `rectangle&` generically and changes one side would silently turn a `square` into a non-square — breaking the invariant the `square` type exists to guarantee, with no compile error anywhere. This is exactly the Liskov Substitution Principle from section 1: the *shape* of the type hierarchy was fine; the *behavioral contract* assumed by existing `rectangle`-using code was not actually honored by `square`. The fix is never purely structural — it means asking, before inheriting, "does every operation the base promises still make sense, unconditionally, for the derived type?"

### Four ways to give a pure virtual member a body, revisited

Section 2 listed the options for a pure virtual *destructor* specifically; the same menu applies to pure virtual members generally, and `v_base::pure()` in `virtual.h` demonstrates one of them directly — a non-pure virtual function with a perfectly ordinary default implementation, callable and overridable like any other:

```cpp
struct VBase
{
    virtual ~VBase() = 0;   // 1. pure virtual destructor (always legal to define)
    void pure() { /* ... */ }  // not pure at all — a default implementation
};
```

Putting it all together, a pure virtual member can be satisfied by: a pure virtual destructor (legal to define, as shown); a non-pure virtual function with a default implementation that derived classes may simply inherit unchanged; a `protected` partial implementation meant to be invoked explicitly from an override, rather than inherited wholesale; or, for a function that genuinely cannot have a sensible default, a body that unconditionally throws, so skipping the override is caught at run time with a clear diagnosis rather than silently doing the wrong thing. Which to choose is a design decision about whether skipping the override should be a compile error (truly pure, no body at all), a quiet inheritance of sensible default behavior, or a loud failure.

### Design guidance for the non-virtual/virtual/pure-virtual spectrum

Three different promises are available to a base class author, and mixing them up is a common source of surprise:

* A **non-virtual** function is inherited as-is, by everyone, with no customization point at all.
* A **pure virtual** function inherits only the *interface* — every leaf class must supply its own behavior, and the base makes no promise about what that behavior is.
* A **virtual function with a default implementation** inherits that default unless a derived class chooses to override it — which is risky if the default is not universally appropriate, because skipping the override then compiles cleanly and silently does something wrong.

When a default truly isn't appropriate for every derived class, two disciplined alternatives exist: require derived classes to inherit from a class that *does* provide a correct implementation rather than from the one with the risky default, or expose the behavior only under an explicitly different, non-virtual name (e.g., a method literally named so that calling the fallback is a visible, intentional choice) so that forgetting to specialize becomes a compile error instead of a quiet logic bug.

Two smaller conventions round this out: an abstract class's `operator=` should itself be `protected`, for the same reason its constructor/destructor often are — it prevents a caller holding a base reference from assigning through it and slicing or mixing up derived types, while still letting each concrete derived class have its own public, correctly-typed assignment; and, as section 1 already argued more generally, an abstract class is a particularly good place to keep data out entirely and delegate storage to a dedicated class, exactly as `shape`/`shape_info` do above.

---

## 5. The Adapter pattern via private and protected inheritance

### Private inheritance means "has-a", implemented through inheritance instead of a member

`adapter.h` builds a stack-like interface on top of a `deque_t` without exposing `deque_t`'s full interface — and it does so with inheritance, not composition, specifically so it can reach `deque_t`'s members directly instead of forwarding each one through a member variable:

```cpp
class deque_t
{
public:
    void push_back(int v);
    void push_front(int v);
    int pop_back();
    int pop_front();
    bool is_empty();
private:
    std::deque<int> deque_;
};

class adapter_stack_t : protected deque_t   // could be private; protected chosen for further derivation
{
public:
    void push(int v);
    int pop();
    bool is_empty();
};
```

Under private (or protected) inheritance, every member of `deque_t` that was `public` or `protected` becomes, respectively, `private` or `protected` as seen through `adapter_stack_t`. Outside code holding an `adapter_stack_t` cannot call `push_back`/`push_front`/`pop_front` at all — only the narrower `push`/`pop`/`is_empty` interface `adapter_stack_t` chooses to expose publicly. This is the Adapter pattern: reuse an existing implementation while presenting a deliberately different, narrower interface.

### Private inheritance deliberately breaks the implicit upcast

With public inheritance, converting a derived pointer/reference to its base is implicit and unremarkable (section 3). Private inheritance switches that off for code outside the class: there is no legal, implicit (or even `static_cast`-based, from outside) conversion from `adapter_stack_t*` to `deque_t*`, because `adapter_stack_t` is emphatically not advertising an "is-a `deque_t`" relationship to the world — only an internal "is-implemented-using" one. Protected inheritance relaxes this one notch: a further-derived class (like `write_only_adapter_stack_t` below) can still see and use the inherited interface, which is exactly why `adapter_stack_t` chooses `protected` over `private` — it wants to support being a base for `write_only_adapter_stack_t` without opening itself up to arbitrary outside code.

```cpp
class write_only_adapter_stack_t : protected adapter_stack_t
{
public:
    void push(int v) { adapter_stack_t::push(v); }   // reaches the inherited protected member fine
};
```

### `using` can re-expose exactly the members you want, without renaming

Instead of writing forwarding member functions one by one, a `using`-declaration inside a privately-inherited class can promote specific inherited members back to public visibility under their original names, wherever that is preferable to defining new method names:

```cpp
class another_adapter_stack_t : private deque_t
{
public:
    using deque_t::push_back;
    using deque_t::pop_back;
    // push_front/pop_front/is_empty remain inaccessible — not re-exposed
};
```

A `using`-declaration can appear in any section of the class and affects the whole class regardless of where it's written, but — as noted in section 1 — it can only re-open access to something the derived class could already see; it cannot reach into a base's genuinely private members that private inheritance had no access to in the first place.

### Composition is the safer default; reach for inheritance when you need more

The same adapter is also expressible with an ordinary member instead of inheritance:

```cpp
class composite_t
{
public:
    void push(int v) { deque_.push_back(v); }
    int pop() { return deque_.pop_back(); }
    bool is_empty() { return deque_.is_empty(); }
private:
    deque_t deque_;
};
```

This composition-based version is generally considered the safer default for the Adapter pattern: it is easier to reason about exception safety (a strong exception guarantee in `operator=` is simpler to implement for an ordinary member than across an inherited base), it makes the "has-a" relationship visually explicit at the point of declaration, and it avoids every one of the private-inheritance subtleties above (which members got re-hidden, which casts got disabled, which `using`-declarations were needed). Prefer private/protected inheritance over composition specifically when the derived class genuinely needs something composition cannot give it cheaply — direct access to a `protected` base member, or participation in the base's own polymorphic hierarchy — not merely because the base happens to have the methods you want to forward.

---
---

## Rules worth keeping in working memory

The central distinction in this topic is between what a hierarchy's *shape* guarantees (what converts to what, what is accessible from where) and what its *contract* promises (what substituting a derived object actually does to calling code). A diagram that compiles cleanly can still violate Liskov; a vtable that dispatches correctly can still be called from a constructor and silently use static binding. Carrying that distinction from design through to the specific override in front of you avoids most of the surprises above.

> A derived class that compiles against its base is not the same thing as a derived class that may safely replace its base everywhere the base is used.

### Core rules

1. **Constructors, destructors, `operator=`, and friends are never inherited — write or default each one per class.**
2. **Give any class with a virtual function a virtual destructor, as a reflex.**
3. **Virtual calls from a constructor or destructor body use only static binding — never assume an override runs there.**
4. **Prefer references/pointers over passing polymorphic types by value — by-value silently slices.**
5. **Keep non-leaf classes in a hierarchy abstract, and keep data out of interface-only base classes.**

### Pitfalls at a glance

| Pitfall | What happens | Do instead |
|---|---|---|
| Non-virtual destructor, deleted through a base pointer | Derived destructor never runs; resources leak | Make the destructor `virtual` the moment any member function is |
| Passing a polymorphic object by value | Derived-only data is silently dropped (slicing) | Take `const Base&` or `Base*`/smart pointer |
| Relying on a virtual call inside a constructor/destructor | Static binding runs the *current* class's version, not an override | Avoid virtual calls there, or accept the static-binding result deliberately |
| Giving an override a different parameter list "by accident" | It hides the base function instead of overriding it; silently compiles | Always write `override` so the compiler checks the match |
| Treating square-from-rectangle-style inheritance as purely structural | Base's behavioral contract breaks even though the hierarchy "makes sense" | Verify every base operation remains valid for the derived type, not just that it compiles |

### Mental model summary

| Question | Language guarantee | Implementation detail | Application responsibility |
|---|---|---|---|
| Which function body runs? | Virtual → dynamic type; non-virtual → static type | vtable layout, slot order, `this`-adjustment | Mark intent with `virtual`/`override`/`final`, not by convention alone |
| What happens on conversion? | Upcast always implicit and safe; downcast requires an explicit, checked cast | Object layout, RTTI representation | Choose `dynamic_cast`/`static_cast` deliberately; prefer a virtual function over a type check |
| Can this be substituted? | Nothing — the compiler checks shape, not behavior | N/A | Verify the Liskov Substitution Principle by hand for every public base |

### Review checklist

- [ ] Does every class with a virtual function have a virtual destructor?
- [ ] Is any polymorphic type ever passed, returned, or stored by value where slicing could occur?
- [ ] Does any override rely on a default argument, and if so, is the static-binding behavior intentional?
- [ ] Is every non-leaf class in the hierarchy actually abstract?
- [ ] Would substituting any derived class for its base silently change observable behavior that calling code depends on?
- [ ] Is inheritance used here because the derived type genuinely *is* the base, or only because the base happened to have convenient code to reuse (in which case, should this be composition instead)?

---

## Diagnostics, useful compiler settings and extensions

A compiler can catch a signature mismatch that breaks virtuality, or a missing virtual destructor, but it cannot evaluate whether a hierarchy actually honors Liskov substitutability — that stays a design review question no flag answers for you.

### Warnings

| Compiler | Flags | What they reveal |
|---|---|---|
| GCC | `-Wall -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -Wsuggest-override` | Missing virtual destructors, accidental hiding instead of overriding, missing `override` |
| Clang | `-Wall -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -Winconsistent-missing-override -Wsuggest-override` | Same failure modes, with Clang's own override-consistency checks |
| MSVC | `/W4 /w44263 /w44264 /w44265` | Hidden base functions, non-virtual destructor in a polymorphic base, inconsistent overrides |

### Sanitizers and runtime checks

| Tool | Setting | Best use |
|---|---|---|
| UndefinedBehaviorSanitizer `vptr` check | `-fsanitize=vptr` (GCC/Clang) | Catches calling a virtual function on an object whose vptr doesn't match its declared type (e.g. during construction/destruction misuse, or after certain casts) |
| AddressSanitizer | `-fsanitize=address` | Use-after-free through a dangling base pointer/reference after a derived object is destroyed |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | Invalid downcasts and other instrumented undefined operations; coverage is incomplete |

### Layout and ABI-related options

| Option | Effect | Recommendation |
|---|---|---|
| `-fdump-class-hierarchy` (GCC) | Dumps the actual vtable/layout the compiler generated for a class | Use when you need to confirm, not guess, how a hierarchy is laid out |
| `/vd0`, `/vd1`, `/vd2` (MSVC) | Controls vtordisp thunk generation for virtual bases | Leave at the default unless you have a specific virtual-inheritance ABI problem |
| `-Wpadded` (GCC/Clang) | Flags padding introduced by member/base ordering | Useful when checking that adding a vptr or reordering bases didn't bloat a type unexpectedly |

### Compiler extensions

| Extension | Compilers | Use | Portable approach |
|---|---|---|---|
| `__has_virtual_destructor(T)` | GCC/Clang/MSVC (as a builtin feeding `std::has_virtual_destructor`) | Query virtual-destructor-ness programmatically | Prefer `std::has_virtual_destructor_v<T>` from `<type_traits>` |
| `__builtin_offsetof` / layout introspection builtins | GCC/Clang | Inspect subobject offsets for debugging multiple-inheritance layouts | Keep behind a debug-only wrapper; never rely on a specific offset in portable code |
| `__declspec(novtable)` | MSVC | Suppresses vtable generation/initialization for a pure-interface base that is never directly constructed | Prefer an ordinary abstract base; use only as a measured optimization |

### Libraries and tooling beyond the standard

| Tool | Purpose | When useful |
|---|---|---|
| Compiler Explorer | Inspect generated vtables and virtual-call codegen side by side across compilers | Verifying the vtable/vptr claims in section 2 against a real toolchain |
| clang-tidy (`cppcoreguidelines-virtual-class-destructor`, `modernize-use-override`, `cppcoreguidelines-slicing`) | Static analysis for exactly the pitfalls in this chapter | Reviewing hierarchy changes before they land |
| C++ Core Guidelines (the `C.*` inheritance rules) | Design-level guidance that doesn't reduce to a compiler flag | Resolving "should this really be inheritance" questions like the ones in section 5 |

---

## Standards timeline

### C++98/03: the original model

Virtual functions, pure virtual functions, abstract classes, multiple and virtual inheritance, `dynamic_cast`, `typeid`, and `static_cast` were all already part of the language. There was no `override`/`final` specifier and no inherited-constructor syntax — a derived class that needed a base's constructor signature had to write it out by hand.

### C++11: `override`, `final`, and inheriting constructors

C++11 added the `override` and `final` context-sensitive (not reserved) specifiers for member functions: `override` lets the compiler verify that a declaration actually overrides a base virtual function, and `final` forbids further overriding of that one function — or, applied directly after a class name, forbids further derivation from the class at all. Neither specifier is part of a function's type, and neither may be repeated in an out-of-class definition. [`02_polymorphism/virtual.h`](02_polymorphism/virtual.h) now uses `override` throughout wherever a declaration genuinely overrides something (and deliberately omits it on `Derived::func`, where `Base::func` isn't virtual and there is nothing to override — adding `override` there is a compile error, which is the point), and demonstrates both forms of `final` with the actual compile errors they produce, kept disabled behind `#if 0` per this repo's convention for code that must not compile.

C++11 also introduced inheriting constructors (`using Base::Base;`), letting a derived class opt in to a base's constructor set instead of re-declaring forwarding constructors by hand. [`01_simple_inheritance/classes.h`](01_simple_inheritance/classes.h)'s `TemporaryEmployee` does exactly this, deliberately alongside `Manager`, which writes its own constructor by hand — the same base class, two different, equally legitimate choices.

### C++14: no changes to this topic

No language changes to inheritance, virtual dispatch, or abstract classes landed in C++14; the relevant `constexpr`-relaxation work in that release is a separate topic (see the `02_constness` chapter).

### C++17: inheriting constructors were fixed, not extended

A defect report against C++11 (P0136R1) retroactively changed *how* inheriting constructors work, without changing the feature's surface syntax. The original C++11 semantics worked by synthesizing extra constructor declarations in the derived class, which caused redundant argument copies/moves and interacted badly with SFINAE; from C++17 onward, inherited base constructors are instead found directly through name lookup. Code that used `using Base::Base;` under either rule set looks identical; the C++17 rewording mainly fixed subtle cases that used to misbehave.

### C++20: concepts as a compile-time-adjacent alternative

C++20 introduced concepts and constrained templates, which do not change anything about virtual dispatch itself but do offer a genuine design alternative to it for the specific case in section 2's rule of thumb: when the *stored type* varies but is fully known at compile time, a concept-constrained template can express "any type providing this interface" without paying for a vtable, a vptr, or dynamic dispatch at all. It is not a replacement for run-time polymorphism when the concrete type truly isn't known until run time — only a cleaner compile-time substitute for the cases where it was.

### C++23: explicit object parameters ("deducing `this`")

C++23 (P0847) lets a non-static member function declare its implicit object parameter explicitly, with the `this` keyword prefixing the parameter: `void f(this Self&& self)`. This is most directly useful for deduplicating `const`/non-`const`, lvalue/rvalue overload families into one templated member — a problem independent of inheritance — but it is also documented as enabling a cleaner variant of the CRTP (Curiously Recurring Template Pattern) for static/compile-time polymorphism, giving generic code one more tool that sits alongside, rather than inside, the run-time virtual-dispatch machinery this chapter covers. A function with an explicit object parameter specifically **cannot** be `virtual` — the feature is additive next to ordinary virtual dispatch, not a replacement for it.

### C++26: constexpr virtual inheritance

C++26 adopts P3533R2, which removes the long-standing prohibition on `constexpr` constructors and destructors for types with virtual bases — previously, any type using virtual inheritance was flatly excluded from constant evaluation. This is a narrow but real change specific to this chapter's subject matter (it affects virtual inheritance specifically, not virtual functions or ordinary single inheritance in general), and it is the only C++26 development in this area confirmed against current standards tracking at the time of writing. Beyond that, no further C++26 changes to ordinary virtual dispatch, abstract classes, or private/protected inheritance have landed; this timeline otherwise runs confidently only through C++23, and that should be stated plainly rather than papered over with speculation.

---

## Migration note for Java / Python / C# developers

Readers coming from managed, reference-semantics languages tend to carry over assumptions that mostly apply to those languages' object models specifically, not to OOP in general. In C++, several things those languages bake into the language itself are, instead, patterns you build out of more primitive pieces — and some things that are automatic there require a deliberate choice here.

### Where intuition transfers and where it breaks

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| Virtual dispatch | Every method is virtual by default (`final` opts out) | Every method is effectively dynamically dispatched | Methods are non-virtual unless marked `virtual`; `override` is required on the derived side | Non-virtual by default; `virtual` opts in, `override`/`final` are optional but strongly recommended |
| Multiple inheritance | Single class inheritance; multiple *interfaces* | Multiple inheritance of classes is allowed (with MRO) | Single class inheritance; multiple *interfaces* | Multiple inheritance of classes is fully supported, including multiple polymorphic bases |
| Passing an object "by value" | Impossible for objects — always a reference | Impossible for objects — always a reference | Reference types behave the same way; only `struct` value types copy | Slicing is possible — a derived object passed/returned as a base *by value* loses its derived part |
| Abstract class / interface | A first-class language construct (`abstract class`, `interface`) | No static enforcement; "interface" is a convention (duck typing, ABCs) | A first-class language construct (`abstract class`, `interface`) | Not a language feature — a design pattern built from pure virtual functions and careful constructor/destructor access |

### Common wrong assumptions

The most common assumption is that "every method can be overridden" transfers directly — in C++ it does not; a non-virtual function hides rather than overrides, and that distinction compiles cleanly either way, so nothing short of `override` catches the mistake. A second is assuming an object handed to a function "by value" behaves like it does in Java/Python/C#, where there is no such thing as passing an object by value at all, only passing a reference; in C++, passing a polymorphic type by value is legal, compiles, and silently slices. A third is expecting `abstract`/`interface` to be checked by the language the way Java's or C#'s is; in C++ there is no dedicated keyword — "abstract" simply means "has an unimplemented pure virtual function", and the protections described in section 4 (protected constructor, no data in the interface layer) are conventions the class author has to apply by hand, not guarantees the compiler hands you for free.

### Idiomatic C++ replacement

| Habit from managed languages | Idiomatic C++ |
|---|---|
| `interface`/`abstract class` keyword | A class with (only) pure virtual functions, a `protected` constructor/destructor, and no data members |
| Assume every override "just works" | Mark every intended override `override`; mark a deliberately final one `final` |
| Pass objects around freely, trusting reference semantics | Pass polymorphic types by reference/pointer/smart pointer; reserve by-value for concrete leaf types you intend to copy |
| Rely on the runtime to pick the right `virtual` method automatically everywhere | Remember that constructors/destructors are a specific, documented exception — they use static binding even on an otherwise fully virtual object |

---

## Further reading

* [language/derived_class](https://en.cppreference.com/w/cpp/language/derived_class)
* [language/virtual](https://en.cppreference.com/w/cpp/language/virtual)
* [language/abstract_class](https://en.cppreference.com/w/cpp/language/abstract_class)
* [language/override](https://en.cppreference.com/w/cpp/language/override)
* [language/using_declaration](https://en.cppreference.com/w/cpp/language/using_declaration)
* [language/dynamic_cast](https://en.cppreference.com/w/cpp/language/dynamic_cast)
* [language/typeid](https://en.cppreference.com/w/cpp/language/typeid)
* [language/member_functions (explicit object parameter / "deducing this")](https://en.cppreference.com/w/cpp/language/member_functions)
* [language/crtp](https://en.cppreference.com/w/cpp/language/crtp)

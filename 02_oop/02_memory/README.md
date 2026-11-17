# 02. Memory Management in C++ Classes

Dynamic memory in C++ is not a single switch you flip on `new`/`delete`. It is a small family of distinct customization points that answer different questions:

* how much raw storage does this type need, and at what address alignment?
* who supplies that raw storage — the default global allocator, a user-defined one, or memory the caller already owns?
* what happens when the allocator cannot satisfy a request?
* how is the raw storage released once the object's lifetime ends, and does that release pair correctly with how it was obtained?

`new X(args)` and `delete p` look like single operators, but each one is really two separate steps bundled together: a call to an allocation/deallocation function (`operator new` / `operator delete`, which *you can overload*), and construction/destruction of the object (which you *cannot* intercept the same way). Confusing the two steps is the source of most of the bugs in this area.

The examples in this directory are:

1. [`01_new_forms/new_forms.cpp`](01_new_forms/new_forms.cpp) — the standard forms of `new`: default, placement, and `nothrow`, plus `std::set_new_handler` and `std::bad_alloc`.
2. [`02_operator_new_delete/`](02_operator_new_delete/) — overloading `operator new`/`operator delete` per class (`user_alloc.h`/`.cpp`, `memory.cpp`), and a CRTP mixin giving every class its own `new_handler` (`new_handler_mixture.h`).
3. [`03_placement_delete/placement_delete.cpp`](03_placement_delete/placement_delete.cpp) — custom placement `operator new`/`operator delete` pairs, and when placement delete actually gets called.
4. [`04_memory_pool/memory_pool.cpp`](04_memory_pool/memory_pool.cpp) — a fixed-size pre-allocated pool used as a class-specific allocator.
5. [`05_alignment/align.cpp`](05_alignment/align.cpp) — struct padding and why byte-wise comparison of objects is unsafe.

A useful first map:

| Facility | Since | Main question |
|---|---:|---|
| `operator new` / `operator delete` (global, overloads) | C++98 | Who supplies/releases raw storage? |
| Placement `new` / placement `delete` | C++98 | Construct in storage I already own; release only on construction failure |
| `std::nothrow_t` | C++98 | Report allocation failure by null instead of exception |
| `std::set_new_handler` | C++98 | What runs when the default allocator cannot satisfy a request? |
| sized `operator delete(void*, size_t)` | C++14 | Give the deallocator the object's size without a lookup |
| `alignof` / `alignas` | C++11 | Query/declare a type's required alignment |
| over-aligned `new`, `std::align_val_t` | C++17 | Allocate storage for a type whose alignment exceeds the default |
| `std::pmr::*` | C++17 | Decouple containers from a concrete allocator strategy at runtime |
| destroying `operator delete(T*, std::destroying_delete_t)` | C++20 | Let `operator delete` itself run the destructor |
| `std::allocator_traits<A>::allocate_at_least` | C++23 | Let a container accept more capacity than it asked for |

The shortest reliable mental model is:

> `new`/`delete` expressions bind storage management (`operator new`/`operator delete`) and object lifetime (constructor/destructor) together. Overloading changes only the storage half of that contract.

The rest of this document expands that distinction.

---

## 1. The forms of `new`: default, placement, and `nothrow`

### `new` is an expression, `operator new` is a function

```cpp
MyClass* p = new MyClass;
```

performs, conceptually, two independent steps:

```cpp
void* raw = operator new(sizeof(MyClass)); // allocation function — overloadable
MyClass* p = ::new (raw) MyClass();        // placement-new construction — not overloadable
```

The `new`-expression itself cannot be redefined. What you overload is the allocation function it calls, and — separately — you can call the built-in placement form directly to construct into memory you already have.

### Default form: throws on failure

```cpp
int* i = new int(6);
delete i;
```

If no storage is available, the default global `operator new` throws `std::bad_alloc`. `01_new_forms/new_forms.cpp`'s `show_bad_alloc()` demonstrates catching it:

```cpp
try {
    allocate_a_lot();
}
catch (std::bad_alloc&) {
    std::cout << "no more memory: bad_alloc handler\n";
}
```

### Placement form: you already own the storage

```cpp
char buffer[sizeof(MyClass)];
MyClass* p = new (buffer) MyClass;
p->~MyClass();
```

The placement `new`-expression calls `void* operator new(std::size_t, void*) noexcept` — a reserved overload the implementation always provides — which does nothing but return its second argument. No allocation happens; only construction. Because no memory was allocated on this path, there is nothing to `delete`: the object must be destroyed with an explicit destructor call, as shown above and in `new_forms.cpp`'s `show_new_delete()`.

This is the standard way embedded and real-time code gets precise control over *where* an object lives — a fixed hardware buffer, a stack arena, a memory-mapped region — without asking the heap for anything.

### `nothrow` form: failure reported by null, not exception

```cpp
MyClass* p = new (std::nothrow) MyClass;
if (p == nullptr) {
    // handle failure without a try/catch
}
delete p;
```

This calls `operator new(std::size_t, const std::nothrow_t&) noexcept`, which returns `nullptr` on failure instead of throwing. It predates widespread exception use in some codebases and is still relevant where exceptions are disabled.

### `std::set_new_handler`: a hook, not a guarantee

When the default allocator cannot satisfy a request, it does not go straight to `bad_alloc`. First it repeatedly invokes the current *new-handler*:

```cpp
void no_more_memory()
{
    std::cout << "no more memory: new_handler\n";
    std::exit(1); // or: free some memory, or set a different handler, or throw bad_alloc
}

std::set_new_handler(no_more_memory);
```

A new-handler's contract is to do one of: free some memory and return (allocation is retried), install a different handler and return, throw `std::bad_alloc` (or a derived type), or terminate the program. Returning normally without having freed anything causes the allocator to call the handler again — forever. If the handler is the null pointer, failure goes straight to `bad_alloc`.

---

## 2. Overloading `operator new`/`operator delete` per class

### What overloading actually buys you

`02_operator_new_delete/user_alloc.h`/`.cpp` overload `operator new`/`operator delete` on a single class. The usual motivations are performance (many small, same-size objects benefit from a dedicated allocator — see section 4) and instrumentation (counting/logging/poisoning allocations for a specific type). The *interface* `new X` / `delete p` does not change at the call site; only which function actually hands out the bytes changes.

### A class-level overload is inherited — design for that

```cpp
class user_alloc
{
public:
    static void* operator new(std::size_t s);
    static void operator delete(void* p);
    // ...
};
```

Any class deriving from `user_alloc` inherits these unless it overloads its own. `02_operator_new_delete/new_handler_mixture.h`'s `NewHandlerSupport<T>` relies on exactly this, but through CRTP instead of ordinary inheritance — see the next section. If a base class's `operator new` assumes a fixed size, a larger derived object allocated through it will be undersized. Either make the base allocator size-agnostic (as `universal_allocate` in `user_alloc.cpp` is, since it just forwards `s` to `malloc`) or give each derived class its own overload.

### `operator new` for arrays is a separate function

```cpp
static void* operator new[](std::size_t s);
static void operator delete[](void* p);
```

`operator new` and `operator new[]` are unrelated overload sets — providing one does not provide the other, and `new X[10]` will not use a class's scalar `operator new`. `user_alloc.cpp` implements both, forwarding to the same `universal_allocate` helper; in general they are free to differ (for instance, `operator new[]` cannot assume the request size is `sizeof(T)`, since it also carries array-cookie overhead).

### Overloading one non-placement form hides the others

Once a class declares *any* `operator new`, all the global forms — plain, array, nothrow — stop being visible by name lookup for that class unless re-declared:

```cpp
// If only this exists:
static void* operator new(std::size_t s, std::new_handler p);

// then plain `new X` on this class is now an error, not a fallback
// to the global operator new — unless the plain form is also overloaded.
```

`user_alloc` sidesteps this by overloading every form it wants reachable: plain, array, placement-by-pointer, and the new-handler variant shown in `memory.cpp`.

### Placement `operator new`/`operator delete` must be declared in pairs

```cpp
static void* operator new(std::size_t s, void* place);
static void operator delete(void* p, void* place);
```

For every placement allocation function `operator new(size_t, P2, ..., Pn)` there should be a matching placement deallocation function `operator delete(void*, P2, ..., Pn)` with the same extra parameter list. The pairing matters only for one specific situation.

### Placement delete runs only when construction throws

```cpp
user_alloc* y = new(x) user_alloc(); // placement new overload
```

If the constructor of `user_alloc` throws here, the runtime automatically calls the matching placement `operator delete(void*, void*)` to undo the allocation — this is the *only* way that function is invoked implicitly. Once the object is fully constructed, `delete y` calls the plain (non-placement) `operator delete`, never the placement one. To invoke a placement delete deliberately, you must call it in explicit function-call syntax, exactly as `memory.cpp` does:

```cpp
user_alloc::operator delete(y, x); // explicit call — this is the only other way to reach it
```

Chapter 3 (`03_placement_delete`) is dedicated entirely to this rule.

### `operator delete` as a template must be a placement form

If a class-level `operator delete` is a template, the standard requires its return type to be `void`, its first parameter to be `void*`, and it to take two or more parameters — in other words, only placement-style `operator delete`s may be templated. A non-placement, single-argument `operator delete` cannot be a template.

### `operator delete` needs a virtual destructor to be reliable in inheritance

`operator delete` is unusual: it is a static/non-member-like function, yet which overload runs for `delete p` (through a base pointer) is effectively dispatched dynamically — through the *destructor*, not through the function itself. `delete p` first destroys the pointed-to object, then calls `operator delete` for the object's actual size and type. If `~Base()` is not `virtual`, `delete p` destroys only the `Base` part and calls `Base::operator delete`, even when `p` actually points at a `Derived`. `user_alloc.h` makes its destructor `virtual` specifically so this resolves correctly:

```cpp
virtual ~user_alloc(); // without this, operator delete() may not work properly
```

### `new_handler` can be per-class, via CRTP

The global `std::set_new_handler` is a single, process-wide hook. `02_operator_new_delete/new_handler_mixture.h` gives each class its own:

```cpp
template <typename T>
class NewHandlerSupport
{
public:
    static std::new_handler set_new_handler(std::new_handler p);
    static void* operator new(std::size_t size);
    static void operator delete(void* p);
    static void no_more_memory();
private:
    static std::new_handler _current_handler;
};
```

Deriving `LargeObject : public NewHandlerSupport<LargeObject>` gives `LargeObject` its own static `_current_handler`, independent of `SmallObject : public NewHandlerSupport<SmallObject>`'s. This only works *because* it is CRTP: the template parameter `T` makes `NewHandlerSupport<LargeObject>` and `NewHandlerSupport<SmallObject>` genuinely different instantiations with their own static data member, not one shared base class. A plain (non-template) mixin base would give every derived class the same single handler slot.

The implementation installs its own handler as the *global* one for the duration of the allocation, then restores whatever was there before:

```cpp
std::new_handler global_handler = std::set_new_handler(_current_handler);
try {
    mem = ::operator new(sz);
}
catch (const std::bad_alloc&) {
    std::set_new_handler(global_handler);
    throw;
}
set_new_handler(global_handler);
```

This is a borrow-and-restore pattern, not true per-class concurrency of handlers — the global new-handler slot is still a single process-wide variable at any instant.

---

## 3. Placement delete semantics

### The pairing rule, proven by a failing constructor

`03_placement_delete/placement_delete.cpp` builds a minimal, deliberately-throwing type:

```cpp
struct X
{
    X() { throw std::runtime_error("X object never be created"); }

    static void* operator new(std::size_t sz, bool b)
    {
        std::cout << "custom placement new called, b = " << b << '\n';
        return ::operator new(sz);
    }
    static void operator delete(void* ptr, bool b)
    {
        std::cout << "custom placement delete called, b = " << b << '\n';
        ::operator delete(ptr);
    }
};

X* p1 = new (true) X; // allocates, then the constructor throws
```

Running this prints the placement-new message, then the placement-delete message, then the caught exception text — in that order. The allocation function ran, the constructor threw before `p1` could be assigned, and the language automatically unwound the partial allocation by calling the *matching* placement `operator delete(void*, bool)`. If that overload did not exist, the raw memory from the successful `operator new` call would simply leak; there would be no deallocation function with a compatible signature for the compiler to call.

### Placement delete is otherwise reachable only by an explicit call

Outside of the exception-unwinding case above, a placement `operator delete` is dead code unless you call it yourself in function-call syntax (`X::operator delete(ptr, true)`), because an ordinary `delete p1` expression always resolves to a *non-placement* `operator delete`. This asymmetry — placement new is written with special `new`-expression syntax, placement delete is almost never invoked by any expression syntax at all — is the single most common source of confusion in this topic.

### Design consequence: placement overloads are an exception-safety seam, not a release path

Because placement delete's only automatic trigger is constructor failure, do not design a class assuming its placement `operator delete` is a general release mechanism for placement-constructed objects. For an object placement-constructed into caller-owned storage (section 1), releasing the storage is the caller's job, done with an explicit destructor call — not with `delete` or with the placement `operator delete`.

---

## 4. A fixed-size memory pool as a custom allocator

### Why bypass the general-purpose allocator at all

`04_memory_pool/memory_pool.cpp` implements `memory_pool<T>`, a fixed number (`pool_size`) of fixed-size (`sizeof(T)`) slots, pre-allocated once:

```cpp
memory_pool() :
    block_size(sizeof(T)),
    head(::operator new(block_size * pool_size))
{
    ::memset(head, 0x0, block_size * pool_size);
}
```

A general-purpose allocator has to handle arbitrary sizes, arbitrary lifetimes, and (in a multi-threaded program) contention over shared bookkeeping structures. An application that creates and destroys huge numbers of same-sized, short-lived objects — parser nodes, small messages, graph edges — pays that generality tax on every allocation. A pool amortizes it: ask the system for one big slab once, then hand out and reclaim same-sized slots with a trivial scan.

### The allocator is wired in through the same class-level overload mechanism as section 2

```cpp
class memory_pool_item
{
    static void* operator new(std::size_t s);
    static void operator delete(void* p, std::size_t s);
    static memory_pool<memory_pool_item> pool;
};

void* memory_pool_item::operator new(std::size_t s) { return pool.alloc(s); }
void memory_pool_item::operator delete(void* p, std::size_t s) { pool.free(p, s); }
```

Note `operator delete` here takes a `size_t` — this is the *sized deallocation* form, standardized in C++14 (see the Standards timeline), which hands the deallocator the object's size without having to recompute or store it separately. `memory_pool<T>::free` uses it to fall back to the global allocator for anything oversized.

### A free slot is marked by convention, not by a real allocator header

```cpp
void* alloc(std::size_t s)
{
    if (s > sizeof(T))
        return ::operator new(s);

    unsigned char* ret = static_cast<unsigned char*>(head);
    for (std::size_t i = 0; (*ret != 0); ++i) {
        ret += block_size;
        if (i == pool_size)
            return ::operator new(s); // pool exhausted, fall back
    }
    return ret;
}
```

A slot is "free" when its first byte is `0`. This is a simplification worth naming explicitly as a teaching shortcut: it assumes `T`'s first byte is never legitimately `0` for a live object, which is not a safe assumption for an arbitrary `T` (consider a `T` whose first data member, or whose vptr's first byte on some ABI, can be zero while still constructed). A production pool tracks free slots with an explicit free-list (an intrusive singly-linked list threaded through the unused slots) rather than a magic byte value. The comment in the example ("this works for teaching purposes") is implicit in how narrowly it is scoped — treat the zero-byte trick as illustrative, not as a pattern to copy into real code.

### A pool changes failure and fragmentation behavior, not just speed

Beyond raw throughput, pools are also reached for because they bound fragmentation in long-running processes: fixed-size slots from one slab cannot fragment amongst themselves the way a general heap's mixed-size allocations can over time. This is why pool- and arena-style allocation shows up heavily in embedded/real-time systems (deterministic allocation cost, no page faults from a growing heap) and in high-throughput servers (see Further reading for jemalloc and the Windows Low-Fragmentation Heap, both production answers to the same problem this toy pool sketches).

---

## 5. Alignment: `alignof`, `alignas`, and padding

### Class size is not the sum of its members

```cpp
struct S { char c; int i; };
```

`sizeof(S)` is commonly `8`, not `5`. The compiler inserts padding after `c` so that `i` lands on an address satisfying `int`'s alignment requirement (typically 4), and may add trailing padding so arrays of `S` keep every element correctly aligned too.

### `memcmp`-ing objects is unsafe because padding bytes are indeterminate

`05_alignment/align.cpp` keeps this fact as a cautionary one-liner:

```cpp
struct S { char c; int i; };

// Wrong compare
bool is_equal(S a, S b) { return 0 == memcmp(&a, &b, sizeof(S)); }
```

The padding bytes between `c` and `i` are not guaranteed to hold any particular value, and are not guaranteed to be equal between two objects that are logically equal in every named member. `memcmp` compares *all* bytes in the object representation, padding included, so two `S` values with identical `c` and `i` can legitimately compare unequal under `memcmp`. The correct comparison is member-wise: `a.c == b.c && a.i == b.i`.

### `alignof` queries, `alignas` demands

C++11 (see Standards timeline) gives these directly to the language instead of leaving them to compiler-specific extensions:

```cpp
alignof(int);                 // the platform's required alignment for int, typically 4
alignas(16) char buffer[64];  // buffer is now 16-byte aligned, e.g. for SIMD loads
```

`alignas(N)` can only strengthen a type's natural alignment, never weaken it below what the type would require anyway.

### Alignment is also why a typed pointer over raw/packed bytes is dangerous

The old notes for this chapter flagged a realistic bug shape worth restating precisely, because it connects directly to the padding discussion above:

```cpp
std::uint32_t decode(const void* p)
{
    const auto pv = static_cast<const std::uint32_t*>(p); // forms an over-aligned-looking pointer
    std::uint32_t v = 0;
    std::memcpy(&v, p, sizeof(v));                          // this part is fine
    return v;
}
```

Forming `pv` from an arbitrary (possibly packed/network-order, possibly odd-offset) `void*` is itself a latent bug even though this snippet never dereferences `pv` — it invites a later `*pv` during a refactor. On x86/x64 a misaligned load usually just works (often slower); on stricter architectures (some ARM modes, SPARC, older MIPS/PowerPC) it can fault. The fix is to never materialize the typed pointer at all and read only through `std::memcpy` (or `std::bit_cast` under the same constraints):

```cpp
std::uint32_t decode(const void* p)
{
    std::uint32_t v;
    std::memcpy(&v, p, sizeof(v)); // safe regardless of p's alignment
    return v;
}
```

---
---

## Rules worth keeping in working memory

The central distinction in this chapter is between what the *allocation function* controls (where the bytes come from, how big a request is, what happens on failure) and what the *constructor/destructor* control (the object's lifetime). Overloading changes only the former. Carrying that split through every `new`/`delete` expression avoids most of the surprises above.

> A custom allocator changes who hands out bytes. It never changes when an object is considered alive.

### Core rules

1. **Overload `operator new`/`operator delete` for a reason you can measure** (allocation-pattern-specific performance, instrumentation, placement constraints) — not reflexively.
2. **Every placement allocation overload needs its placement deallocation twin**, even though the twin will almost only ever run during constructor unwinding.
3. **Give a base class a virtual destructor if any derived class may be `delete`d through a base pointer and either side overloads `operator delete`.**
4. **Never compare or hash a class object byte-for-byte**; padding bytes are not specified.

### Pitfalls at a glance

| Pitfall | What happens | Do instead |
|---|---|---|
| Overload one `operator new` form, forget the rest | Plain/array/nothrow `new` on that class stops compiling, or silently calls the wrong allocator | Overload (or explicitly re-expose) every form you need reachable |
| Assume placement delete runs on ordinary `delete p` | It never does; only non-placement `operator delete` runs there | Call the placement form explicitly if you ever need it outside constructor failure |
| Non-virtual destructor + `operator delete` through a base pointer | Wrong-sized/wrong-type deallocation, memory corruption | Make the destructor `virtual` whenever polymorphic deletion is possible |
| `memcmp`/byte-hash a struct with mixed member sizes | Comparison fails for logically equal objects because of padding | Compare member-wise, or use a packed/reflected representation designed for byte comparison |
| A hand-rolled pool's "free slot" marker collides with a legitimate value | Silent corruption: a live object is treated as free | Use an explicit free-list, never a magic data byte |

### Mental model summary

| Question | Language guarantee | Implementation detail | Application responsibility |
|---|---|---|---|
| Where do these bytes come from? | `operator new`/`operator delete` overload resolution rules | Actual allocator strategy (malloc, pool, arena, pmr resource) | Choose an allocation strategy that matches the object's lifetime/size profile |
| When does the object become/stop being alive? | Constructor runs after allocation succeeds; destructor runs before deallocation | None — this is fixed by the language | Never read/write through a pointer outside that window |
| How big is this object, really? | `sizeof`/`alignof` for the declared type | Padding layout is ABI/compiler-specific | Never serialize or compare raw object bytes across that boundary |

### Review checklist

- [ ] Does every placement `operator new` overload have a matching placement `operator delete` with the same extra parameters?
- [ ] If this class (or a base of it) overloads `operator delete`, is the destructor `virtual` wherever polymorphic deletion is possible?
- [ ] Does any code compare, hash, or serialize an object's raw bytes instead of its members?
- [ ] If a custom allocator (pool/arena) is used, is its "slot is free" bookkeeping separate from the object's own data, not encoded inside it?
- [ ] Is a `new_handler` (global or per-class via CRTP) actually required here, or would a plain `try`/`catch (std::bad_alloc&)` be clearer?

---

## Diagnostics, useful compiler settings and extensions

Static warnings catch some allocator-overload mistakes (mismatched placement pairs, a missing virtual destructor); most of the rest only shows up at runtime, under a sanitizer, or in production once fragmentation/exhaustion sets in.

### Warnings

| Compiler | Flags | What they reveal |
|---|---|---|
| GCC | `-Wall -Wextra -Wnon-virtual-dtor -Wdelete-non-virtual-dtor` | Base classes with non-virtual destructors that are unsafe to `delete` polymorphically |
| Clang | `-Wall -Wextra -Wnon-virtual-dtor -Wdelete-non-virtual-dtor` | Same, with Clang's diagnostic engine |
| MSVC | `/W4 /w44265` (non-virtual destructor) | Similar class-hierarchy destructor warnings |

### Sanitizers and runtime checks

| Tool | Setting | Best use |
|---|---|---|
| AddressSanitizer | `-fsanitize=address` | Use-after-free, double-free, and heap-buffer-overflow — invaluable when debugging a hand-rolled pool/arena |
| LeakSanitizer | `-fsanitize=leak` (often bundled with ASan) | Allocations that a custom allocator never returns to its own pool or to the system |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | Misaligned access, some pointer-arithmetic UB |
| Valgrind (memcheck) | `valgrind ./binary` | Deep allocator-bug diagnosis without recompiling, at a large speed cost |

### Semantics-changing options

| Option | Effect | Recommendation |
|---|---|---|
| `-fno-exceptions` (GCC/Clang) | Disables `throw`; `nothrow new` and manual null-checks become mandatory | Only for environments that already forbid exceptions globally |
| `/Zc:alignedNew` (MSVC) | Enables C++17 over-aligned `new`/`delete` support | Needed on MSVC to get standard-conforming over-aligned allocation |
| `-fsized-deallocation` (older Clang) | Enables C++14 sized `operator delete` where not default | Rarely needed on current compilers, where it is on by default in C++14 mode or later |

### Compiler extensions and tooling

| Tool | Purpose | When useful |
|---|---|---|
| `__STDCPP_DEFAULT_NEW_ALIGNMENT__` | Macro giving the default alignment guaranteed by plain `new` | Deciding whether a type actually needs the over-aligned `new` path |
| clang-tidy (`misc-new-delete-overloads`, `cppcoreguidelines-owning-memory`) | Static checks for mismatched/incomplete allocator overload sets | Reviewing any class that overloads `operator new`/`operator delete` |
| Compiler Explorer | Inspect generated allocation call sequences | Verifying which `operator new`/`operator delete` overload actually gets selected |

---

## Standards timeline

### C++98/03: the original model

`operator new`/`operator delete` (plain, array, placement, nothrow) and `std::set_new_handler` already existed essentially as used in `01_new_forms` and `02_operator_new_delete`. Placement new/delete pairing rules and the destructor/deallocation-function interaction for polymorphic deletion were already part of the core language model.

### C++11: alignment becomes a language-level concept

`alignof` and `alignas` were added as portable replacements for compiler-specific extensions (`__attribute__((aligned))`, `__declspec(align)`), covered in section 5. This standard also introduced move semantics, which changed allocation *patterns* (fewer copies, more transfers of ownership) without changing the `operator new`/`operator delete` contract itself.

### C++14: sized deallocation

C++14 added global sized `operator delete`/`operator delete[]` overloads taking an extra `std::size_t`:

```cpp
void operator delete(void* ptr, std::size_t size) noexcept;
```

When such a two-argument overload is provided and a single-argument one is not, the runtime passes the object's size to deallocation directly, avoiding a size lookup. `04_memory_pool/memory_pool.cpp`'s `memory_pool_item::operator delete(void*, size_t)` relies on this exact feature. Feature-test macro: `__cpp_sized_deallocation` (`201309L`).

### C++17: over-aligned allocation and polymorphic allocators

Two separate, relevant additions landed in C++17:

* **Over-aligned `new`-expressions.** When a type's alignment exceeds `__STDCPP_DEFAULT_NEW_ALIGNMENT__`, a `new`-expression now passes that alignment to the allocation function via the new `std::align_val_t` tag type, calling overloads like `operator new(std::size_t, std::align_val_t)`. Feature-test macro: `__cpp_aligned_new` (`201606L`).
* **`std::pmr` and `<memory_resource>`.** `std::pmr::memory_resource` is an abstract allocation interface, and `std::pmr::polymorphic_allocator<T>` wraps a runtime pointer to one — letting containers select an allocation *strategy* (a pool, an arena, the system heap) at runtime through a uniform type, rather than baking the allocator into the container's template parameter the way `std::allocator` does. The standard also ships ready-made resources (`std::pmr::monotonic_buffer_resource`, `synchronized_pool_resource`, `unsynchronized_pool_resource`) that solve, with library support, much of what `04_memory_pool`'s hand-rolled pool sketches by hand.

### C++20: destroying delete

C++20 added a form of class-specific `operator delete` that receives the object pointer *before* the destructor runs, and is responsible for calling the destructor itself:

```cpp
void operator delete(T* ptr, std::destroying_delete_t);
```

tagged by `std::destroying_delete_t`/`std::destroying_delete` in `<new>`. This matters for types that store their own size or type tag alongside their storage (variable-length objects, some custom-allocator-backed hierarchies): normal deletion destroys the object and then calls `operator delete`, by which point some of that bookkeeping information may already be gone; destroying delete lets the deallocation function read the object's state before destruction, then destroy and free it itself. Feature-test macros: `__cpp_impl_destroying_delete` / `__cpp_lib_destroying_delete` (`201806L`).

### C++23: `allocate_at_least`

`std::allocator<T>::allocate_at_least` (and the corresponding `std::allocator_traits<Alloc>::allocate_at_least`) lets an allocator return *more* storage than requested when that is cheaper or more convenient for the allocator to provide, instead of being forced to allocate exactly `n` elements. This is aimed at containers like `std::vector`/`std::basic_string`, which can grow their reported capacity to match what the allocator actually handed back instead of discarding the extra space. Feature-test macro: `__cpp_lib_allocate_at_least` (`202106L`). None of this repository's examples use it yet; it is listed here because it is the most directly relevant C++23 allocator feature for custom-allocator authors.

### C++26: nothing landed here that reshapes this chapter

Checked against the current C++26 feature list rather than assumed: C++26 removes the deprecated `std::allocator::is_always_equal` typedef (P2868R3), un-deprecates `std::pmr::polymorphic_allocator::destroy` (P2875R4), and adds `constexpr` support for some memory algorithms (P3508R0/P3369R0) plus freestanding-library coverage for `<memory>` among other headers (P2976R1). These are real but narrow — cleanups and constexpr-reach extensions, not a new allocation model. Nothing in C++26 changes how `operator new`/`operator delete` overloading, placement forms, or alignment work as covered in this chapter. Say so plainly rather than inventing a headline feature: there isn't one here.

---

## Migration note for Java / Python / C# developers

Java, Python, and C# never expose an "operator new" to override, because object lifetime in those languages is managed by a garbage collector: you request an object, and *when* (or whether) its storage is reclaimed is the runtime's decision, not yours. C++ gives you both halves of the problem — *and* the tools to intervene in the first half — because it targets domains where that control is the point, not an inconvenience.

### Why would you ever touch this, coming from a GC'd language

* **Arena/pool allocation** (section 4): when an application allocates and frees huge numbers of same-sized, short-lived objects, bypassing the general-purpose allocator for a dedicated pool is a measured performance and fragmentation win that a GC'd language's generational collector is already doing for you, transparently, as part of its design. In C++ you have to build or select that behavior explicitly.
* **Embedded and real-time constraints**: no GC pause is acceptable, and sometimes no heap exists at all — placement `new` (section 1) into a fixed hardware buffer, or a pool sized at startup (section 4), replaces dynamic allocation entirely.
* **Debugging/instrumentation**: overloading `operator new`/`operator delete` to log, count, or poison every allocation of a given type is how C++ programs build their own leak/corruption detectors, a role filled by the runtime's own tooling (a JVM profiler, Python's `tracemalloc`, the .NET GC's diagnostics) in a managed language.

### Where the concepts line up

| Habit from a managed language | C++ equivalent |
|---|---|
| The GC groups short-lived objects into a generation/arena for cheap bulk reclamation | A custom pool allocator (section 4) reclaiming same-sized slots in O(1) |
| Pluggable allocation strategy is mostly invisible/automatic | `std::pmr::polymorphic_allocator` (C++17) lets a container take a *runtime* choice of allocation strategy, explicitly |
| An `OutOfMemoryError`/`MemoryError` is thrown automatically | `std::bad_alloc`, or a `new_handler` you install yourself (section 1) — C++ makes the retry/abort/free-and-retry policy a decision you write |
| Object pooling frameworks (e.g. object pools in a GC runtime, used to reduce GC pressure) | The same idea, but here it *replaces* an allocator rather than sitting on top of one |

### Common wrong assumptions

Do not assume `delete p` "frees p's memory" the way a GC'd runtime reclaims an unreachable object — it runs a destructor and then calls a specific, resolvable-by-overload deallocation function; which function that is depends on `p`'s static/dynamic type and the virtual-destructor rules in section 2. Do not assume a custom `operator new`'s failure path is handled for you; unlike a managed runtime's uniform `OutOfMemoryError`, C++'s default behavior (`std::bad_alloc`) is itself overridable per-process (`std::set_new_handler`) or per-class (section 2's CRTP mixin), and a careless handler can spin forever instead of failing fast. Do not assume alignment is a detail the runtime "just handles" — C++ structs are not reordered or packed by a JIT the way some managed object layouts are; padding (section 5) is visible, and getting it wrong silently corrupts a byte-level comparison or a cast.

---

## Further reading

### Standard and language reference

* [memory/new/operator_new](https://en.cppreference.com/w/cpp/memory/new/operator_new)
* [memory/new/operator_delete](https://en.cppreference.com/w/cpp/memory/new/operator_delete)
* [language/new](https://en.cppreference.com/w/cpp/language/new)
* [language/delete](https://en.cppreference.com/w/cpp/language/delete)
* [memory/new/align_val_t](https://en.cppreference.com/w/cpp/memory/new/align_val_t)
* [memory/new/destroying_delete](https://en.cppreference.com/w/cpp/memory/new/destroying_delete)
* [memory/allocator/allocate_at_least](https://en.cppreference.com/w/cpp/memory/allocator/allocate_at_least)
* [memory/polymorphic_allocator](https://en.cppreference.com/w/cpp/memory/polymorphic_allocator)
* [header/memory_resource](https://en.cppreference.com/w/cpp/header/memory_resource)
* [language/alignas](https://en.cppreference.com/w/cpp/language/alignas)
* [language/alignof](https://en.cppreference.com/w/cpp/language/alignof)
* [memory/new/set_new_handler](https://en.cppreference.com/w/cpp/memory/new/set_new_handler)

### Production allocators and background reading

* [PJSIP pjlib memory pool documentation](http://www.pjsip.org/pjlib/docs/html/files.htm)
* [jemalloc: scalable memory allocation](https://www.facebook.com/notes/facebook-engineering/scalable-memory-allocation-using-jemalloc/480222803919)
* [Windows Low-Fragmentation Heap](https://learn.microsoft.com/en-us/windows/win32/memory/low-fragmentation-heap)

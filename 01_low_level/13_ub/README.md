# Undefined Behavior (UB) in C and C++ - Advanced Guide

This chapter is dedicated to Undefined Behavior (UB) - one of the most misunderstood,
dangerous, and performance‑critical aspects of C and C++.

UB means:
> *The C++ standard imposes no requirements on what happens.*

That includes:
- crashes
- seemingly "working" behavior
- silent data corruption
- optimizer removing your code entirely

---

## Why UB exists

UB is not a compiler bug. It exists to:
- allow aggressive optimizations
- avoid runtime checks
- support low‑level hardware models
- keep the language usable for kernels and runtimes

---

## UB vs Unspecified vs Implementation‑Defined

| Category | Meaning |
|--------|--------|
| Undefined behavior | No guarantees at all |
| Unspecified behavior | One of several valid behaviors |
| Implementation‑defined | Compiler documents the behavior |

---

## How compilers exploit UB

If the compiler proves UB *could* happen, it may assume it never happens.

```cpp
int f(int* p) {
    if (!p) return 0;
    return *p;
}
```

If `p` is ever null → UB → compiler may remove the check entirely.

---

## How to fight UB

- Compile with sanitizers: `-fsanitize=undefined,address`
- Enable warnings: `-Wall -Wextra -Wpedantic`
- Prefer standard abstractions
- Treat UB as security bugs

---

## Files

* `ub_examples.cpp` - isolated UB examples with detailed comments

---

## Rule

> If your program has UB, the compiler owes you nothing.


## UB cases explained (based on the original educational snippet)

Below is a short explanation for each UB pattern from your original file.  
Rule of thumb: the moment UB occurs, the whole program becomes unconstrained - even code "after" the UB may be optimized away.

### 1) `dereference_null()` - dereferencing a null pointer
What happens: `*ptr` is UB because a null pointer does not point to a valid object.  
Why it's UB: a pointer must be *dereferenceable*; `nullptr` never is.  
Typical outcomes: crash, "works" in debug, miscompilation in release.  
Fix: check before use; prefer references/RAII when ownership is clear.

### 2) `buffer_overflow()` - out-of-bounds write
What happens: `buffer[10] = 'A'` writes past a 5-byte array.  
Why it's UB: array indexing is only valid within bounds; out-of-bounds writes corrupt unrelated objects.  
Typical outcomes: silent corruption, security bugs, optimizer assuming it never happens.  
Fix: bounds checks; prefer `std::array`, `std::span`, `.at()` for checked access.

### 3) `signed_overflow()` - signed integer overflow
What happens: `max + 1` overflows `int`.  
Why it's UB: signed overflow is undefined (unlike unsigned, which wraps modulo 2^N).  
Typical outcomes: "impossible" branches removed, wrong loops/comparisons.  
Fix: widen types, check overflow, or use unsigned if wrap is intended.

### 4) `division_by_zero()` - integer divide by zero
What happens: `i / 0`.  
Why it's UB: division by zero has no defined result.  
Typical outcomes: crash (SIGFPE), trap, random result.  
Fix: validate denominators; define domain behavior (error/optional/exception).

### 5) `uninitialized_var()` - reading an uninitialized automatic variable
What happens: prints `i` without initializing it.  
Why it's UB: the value is indeterminate; reading it is undefined.  
Fix: initialize (`int i{};`) and enable warnings/sanitizers.

### 6) `array_out_of_bounds()` - out-of-bounds read
What happens: reads `array[5]` from `int array[5]`.  
Why it's UB: element 5 does not exist; even reads can violate object bounds assumptions.  
Fix: bounds checks; use `.at()` for teaching/debug.

### 7) `expired_pointer()` - dangling pointer to a dead local
What happens: pointer refers to `x`, then `x` goes out of scope.  
Why it's UB: object lifetime ended; dereference is invalid.  
Fix: never return/store pointers to locals; copy the value or allocate with ownership.

### 8) `type_punning_pointer()` - strict-aliasing violation via `reinterpret_cast`
What happens: reads an `int` object as `float`.  
Why it's UB: violates strict aliasing (and can violate alignment).  
Fix: `std::memcpy` (C++11+) or `std::bit_cast` (C++20) for bit reinterpretation.

### 9) `evaluation_order()` - using a variable in its own initializer
What happens: `int i = i * 0;` reads `i` before it's initialized.  
Why it's UB: direct uninitialized read (not merely "order of evaluation").  
Fix: initialize first (`int i = 0;`).

### 10) `deleted_pointer()` - use-after-delete
What happens: dereference after `delete`.  
Why it's UB: object lifetime ended; memory may be reused.  
Fix: avoid manual `new/delete`; use `std::unique_ptr`/`std::shared_ptr`.

### 11) `call_non_exist_virtual()` - virtual call after delete
What happens: calls `ptr->foo()` after `delete ptr`.  
Why it's UB: use-after-free; virtual dispatch relies on vptr/vtable that no longer exists.  
Fix: never use pointers after delete; use smart pointers and clear ownership rules.

### 12) `dangling_reference()` - reference to freed object
What happens: reference `ref` outlives the pointee.  
Why it's UB: references don't rebind; they dangle if the object dies.  
Fix: don't bind refs to owned dynamic objects unless lifetime is guaranteed.

### 13) `realloc_fails()` - mixing allocation families (`new` with `realloc`)
What happens: passing `new`-allocated memory to `realloc`.  
Why it's UB: `realloc` only works on pointers from `malloc/calloc/realloc`.  
Fix: use `std::vector`/`std::string` for resizing; or stick to `malloc/realloc/free` consistently.

### 14) `misalignment()` - misaligned access
What happens: cast `char*` to `int*` and dereference.  
Why it's UB: the pointer may not meet `alignof(int)`; misaligned dereference can trap.  
Fix: allocate as `int`, or use aligned allocation APIs; don't "invent" alignment by casting.

### 15) `break_strict_aliasing()` - strict aliasing again
Same issue as #8.  
Fix: `std::memcpy` or `std::bit_cast` (C++20).

### 16) `missed_return()` - falling off the end of a non-void function
What happens: not all control paths return a value.  
Why it's UB: caller expects an `int` value; none is produced.  
Fix: return on all paths; enable `-Wreturn-type`.

---

## Practical way to study UB
Run one case at a time with sanitizers:

```bash
clang++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer ub_examples.cpp -o ub
./ub
```

Sanitizers do not catch *all* UB (e.g., some strict-aliasing miscompilations), but they catch a lot.

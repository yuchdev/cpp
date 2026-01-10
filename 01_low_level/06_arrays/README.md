# C++ Arrays - Decay, Types, and Modern Alternatives

## Roadmap

### 01. Array decay, `sizeof`, and overloads

Key ideas:

* Arrays are objects with a real size, but in *most* expressions they **decay to a pointer to their first element**.
* The important non-decay cases you should remember:

  1. `sizeof(arr)` sees the whole array object
  2. `&arr` yields a **pointer-to-array** (`T (*)[N]`), not `T*`
  3. `decltype(arr)` for an unparenthesized id-expression preserves array type
  4. string literal initialization into `char[]` has special rules
  5. binding to a reference-to-array preserves type and size

What this example shows:

* `int a[5]; int* p = a;` works via decay.
* `int (*pa)[5] = &a;` is a pointer to the whole array.
* `p + 1` advances by `sizeof(int)` while `pa + 1` advances by `sizeof(a)`.
* Overload resolution prefers binding to `const int(&)[N]` over decaying to `const int*`.
* A function parameter `int x[]` is **exactly** `int* x`.
* The classic pitfall: `sizeof(a_param)` inside a function gives pointer size.

### 02. Templates, deduction of `N`, and array type traits


Key ideas:

* `T (&)[N]` is the canonical "array-preserving" parameter type.
* You can **deduce `N`** and keep array size at compile time.
* `auto&` preserves array-ness and constness when binding to an array.
* `decltype(a)` on a plain name `a` retains the array type (`int[4]`).

Type-trait tools covered:

* `std::extent<T>`: size of a given dimension
* `std::rank<T>`: number of dimensions
* `std::remove_extent<T>`: peel one dimension (e.g., `int[2][5] -> int[5]`)

### 03. Initialization rules and string literal facts

Key ideas:

* `int a[5] = {};` value-initializes all elements to zero.
* `int b[5] = {1,2};` zero-fills the tail.
* `int c[5];` is **uninitialized** for automatic storage (reading it is UB).
* Brace-initialization prevents narrowing in C++11+.

String literal specifics:

* A string literal like `"ABC"` is an **lvalue** of type `const char[4]` (includes the `\0`).
* `decltype("ABC")` is a reference type (`const char(&)[4]`) because `decltype(lvalue)` yields `T&`.
* `const char* p = "Hello";` stores a pointer (and `sizeof(p)` is pointer size).
* `char s1[] = "Hi";` creates an array sized to include the terminator.

### 04. Multi-dimensional arrays and pointer-to-array types

Key ideas:

* `int m[2][3]` is a *single contiguous object* laid out row-major.
* `&m[0][0]` is a plain `int*` view of contiguous storage.
* `m` decays to a pointer to its first row: `int (*)[3]`.
* `&m` is a pointer to the full matrix object: `int (*)[2][3]`.

Passing 2D arrays:

* The element type of `m` is `int[3]`, so the compiler must know the second dimension.
* You can accept a fixed shape (`const int (&)[2][3]`) or template on `(R, C)`.
* `int**` is a pointer-to-pointer and does **not** describe a contiguous 2D array.

### 05. Memory, `new[]/delete[]`, and typedef pitfalls

Key ideas:

* Arrays allocated with `new T[N]` must be freed with `delete[]`.
* Mixing `new[]` with `delete` is undefined behavior.

The subtle trap:

* `typedef int Week[7];` hides an array type.
* `new Week` allocates an array of 7 ints, but in many contexts you'll see it as `int*` due to decay.
* It becomes easier to accidentally:

  * use the wrong delete form
  * pass it around without size information

Also highlighted:

* Deleting an array through a base pointer (`Base* b = new Derived[2]; delete[] b;`) is a classic UB pattern.
  Even with virtual destructors, the array cookie/layout expectations can mismatch.

### 06. Modern helpers: raw arrays made safer, plus `std::array`

Key ideas:

* `std::begin(a)` / `std::end(a)` work for raw arrays in C++11+.
* Range-for iterates raw arrays **without decay**.
* `std::array<T, N>` is a thin wrapper around `T[N]`:

  * does not decay
  * carries its size
  * is copyable/assignable
  * provides `.data()` for C interop
  * supports `std::array<T, 0>`

---

## Quick reference: the rules you'll use daily

### When arrays decay

An array expression typically decays to `T*` (pointer to first element) **except** when:

* used with `sizeof(arr)`
* used with `&arr` (yields pointer-to-array `T (*)[N]`)
* bound to a reference-to-array (`T(&)[N]`)
* used in `decltype(arr)` as an unparenthesized id-expression

### Function parameters: the "array syntax lie"

```cpp
void f(int x[]);   // exactly the same as: void f(int* x);
void g(int x[10]); // still: void g(int* x);
```

If you need the size, take a reference (`T(&)[N]`), a `std::span` (C++20), or pass size explicitly.

### Pointer-to-array is real and useful

* `int a[5];` has type `int[5]`
* `a` decays to `int*`
* `&a` has type `int (*)[5]`

The address value may be numerically the same, but **arithmetic differs** (element stride vs whole-array stride).

---

## Suggested toolchain flags (high signal for this repo)

These warnings catch a lot of "array nonsense" early:

```bash
-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
```

And for learning/verification, sanitizers are extremely valuable:

```bash
-fsanitize=address,undefined -fno-omit-frame-pointer
```

---

## Connected C++ entities

If you want to extend the course beyond these files, the most useful next concepts are:

* **`std::span<T>` (C++20):** a non-owning view of contiguous data (perfect for "array + size").
* **`std::vector<T>`:** owning dynamic arrays with size.
* **RAII and smart pointers (`std::unique_ptr<T[]>`)** for dynamic arrays when you truly need them.
* **Iterator invalidation and lifetime rules** when bridging between modern containers and legacy APIs.

---

## Notes to developers migrating from other languages

If you come from Python/Java/C#, C++ arrays can feel "unfair" at first. The mental shift is:

* In many languages, an "array" is a **first-class runtime object** (size, bounds, metadata, often heap-based).
* In C++, a raw array `T[N]` is a **compile-time-sized object** with *no runtime metadata*. It's closer to "a struct containing N elements" than to a high-level collection.

What commonly surprises people:

* **The decay rule**: passing an array often silently turns it into a pointer, losing size information.
* **`int x[]` in parameters is not an array**: it's a pointer. This is one of the oldest compatibility features in C/C++.
* **`sizeof` is context-sensitive**: `sizeof(a)` is the full array size; `sizeof(param)` in a function is pointer size.
* **2D arrays are not `T**`**: `T[R][C]` is contiguous, while `T**` usually describes a "jagged" structure with separate allocations.
* **Manual memory is real**: if you use `new[]`, you must match it with `delete[]` (and in modern C++, you usually avoid `new` entirely).

Practical migration advice:

* Prefer **`std::array`** for fixed-size arrays and **`std::vector`** for dynamic size.
* For APIs that need "pointer + length", prefer **`std::span`** (C++20) or pass `(ptr, size)` explicitly.
* Treat raw arrays and pointer arithmetic as a **low-level interop/performance tool**, not your default data structure.

Once you internalize that raw arrays are *types with shape* (`T[N]`, `T[R][C]`) rather than "objects with metadata", the rules become consistent-and your C++ becomes both safer and more predictable.

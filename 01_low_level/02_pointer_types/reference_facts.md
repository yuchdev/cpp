### Reference Facts: C++ References (C++98 → C++17)

This note is a "facts + pitfalls" companion to `reference_facts.cpp`.  
Focus: lvalue references, rvalue references, how references are different from pointers, and obscure corner cases.

---

### 1. What is a reference?

A reference is an alias for an existing object.

- `T&` (lvalue reference) was introduced in C++98
- `T&&` (rvalue reference) was introduced in C++11

A reference is not meant to model "ownership" or "optional presence"; it models required access to an object.

---

### 2. Lvalue references (C++98)

### Core facts

- `T&` binds to lvalues (named objects)
- A reference is not reseatable
  - `rx = y;` assigns through the reference; it does not "rebind" the reference
- A reference cannot be null in well-formed code

### Weird-but-important constraints

- No arrays of references
- No pointers to references
- `sizeof(int&)` is ill-formed (but `sizeof(rx)` is `sizeof(T)`)

---

### 3. Const references and lifetime extension

### Binding to temporaries

- `const T&` can bind to a temporary (prvalue)

### Lifetime extension (the good case)

A temporary bound directly to a `const&` has its lifetime extended to the lifetime of that reference:

```cpp
const std::string& r = std::string("hello"); // OK, temporary lifetime extended
```

### Dangling pitfall (the bad case)

Lifetime extension does not safely "pass through" a function returning a reference:

```cpp
const std::string& dangerous_ref() {
  return std::string("dangling"); // returned reference dangles
}
```

---

### 4. Rvalue references (C++11)

### Why they exist

- enable move semantics
- enable perfect forwarding (in templates)

### Obscure fact: "named rvalue reference variables are lvalues"

```cpp
std::string&& rr = make_string();
std::string x = rr;           // copies (rr is an lvalue expression)
std::string y = std::move(rr); // moves
```

---

### 5. Overload resolution: `const T&` vs `T&&`

Typical idiom:

- `const T&` handles lvalues and const lvalues
- `T&&` handles rvalues (temporaries and moved-from objects)

---

### 6. Reference collapsing and forwarding references (C++11)

Reference collapsing rules:

- `U&  &  → U&`
- `U&  && → U&`
- `U&& &  → U&`
- `U&& && → U&&`

A parameter `T&&` is a forwarding reference when `T` is deduced.

This is the core mechanic behind `std::forward`.

---

### 7. `auto&&` and `decltype((x))` gotchas (C++11)

- `auto&& v = expr;` behaves like forwarding reference deduction:
  - if `expr` is lvalue → `v` becomes `T&`
  - if `expr` is rvalue → `v` becomes `T&&`

`decltype` preserves value category:

- `decltype(x)` is the declared type
- `decltype((x))` is a reference type because `(x)` is a lvalue expression

---

### 8. References vs pointers (technical + logical differences)

### References

- must bind to an object (no null)
- cannot be reseated
- no pointer arithmetic
- good for "required parameter" APIs

### Pointers

- can be null
- can be reseated
- allow pointer arithmetic
- used for optional return-values or parameters (pre-C++17), graph structures, and low-level work

Rule of thumb:

- reference: "you must give me an object"
- pointer: "you may give me an object"

---

### 9. Obscure pitfalls

- Reference members often delete copy assignment implicitly (because references cannot be reseated)
- Cannot bind non-const `T&` to a temporary
- Cannot bind references to bit-fields
- `std::vector<bool>` uses proxy references; `vb[0]` is not `bool&`

---

### 10. `std::reference_wrapper` (C++11)

A real reference cannot be stored in containers and is not reseatable.  
`std::reference_wrapper<T>` provides "reference-like" behavior that is copyable and reseatable:

- useful in containers
- useful for callbacks

---

### 11. Standards timeline (reference highlights)

- C++98: lvalue references, const refs
- C++11: rvalue refs, move, forward, collapsing, decltype/auto patterns
- C++14: generic lambdas; `decltype(auto)` helps preserve references in returns
- C++17: guaranteed copy elision; structured bindings can bind by reference (`auto&`, `auto&&`)

---

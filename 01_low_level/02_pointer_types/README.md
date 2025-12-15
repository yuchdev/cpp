## Pointer Facts

### 1. Boolean logic, control flow, and "pointer → bool"

* The logical operators `&&`, `||`, and `!` take operands of arithmetic and pointer types, convert them to `bool`, and return a `bool` result.
* The `&&` and `||` operators evaluate their second argument only if necessary, so they can be used to control evaluation order.
* The order of subexpressions evaluation within an expression is undefined.
  (So: don't rely on side-effect order unless the language guarantees sequencing)

### 2. Null pointers: `0`, `NULL`, and `nullptr`

* The standard conversions allow `0` to be used as a constant of pointer or pointer-to-member type.
* In C, `NULL` is typically `(void*.0`, which makes it illegal in C++.
* Prefer `nullptr`: it has its own type (`std::nullptr_t`., avoids overload ambiguity, and converts to any object pointer, function pointer, or pointer-to-member.

### 3. Pointer categories and (non-interchangeability)

* A pointer to any type of object can be assigned to a variable of type `void*`, but a pointer to member cannot.
* On some machines, an `int` and an `int*` do not occupy the same amount of space.
* Obscure but important: object pointers, function pointers, and pointer-to-member are distinct categories; they're not required to have the same size/representation, and they are not freely interconvertible.

### 4. Pointer arithmetic and address differences

* `size_t` holds sizes of objects, whereas `ptrdiff_t` holds differences of addresses within objects. (Pointer subtraction yields `ptrdiff_t`).
* Pointer subtraction is only defined within the same array object (including one-past-the-end.. Subtracting unrelated pointers is undefined behavior).
* One-past-the-end pointer values are valid to form and compare/subtract within the same array, but dereferencing one-past is undefined behavior.
* `ptrdiff_t` is closely tied to the machine pointer type, so is `size_t`.

### 5. Pointer comparisons

* Equality/inequality (`==`, `!=`. comparisons are always meaningful.
* Ordering comparisons (`<`, `>`, etc.. are only reliably specified in array/object-related cases; for unrelated pointers, don't assume raw `<` is meaningful.
* If you need a strict weak ordering of pointers (e.g., for ordered containers., prefer `std::less<T*>`).

### 6. Endianness, object representation, and "bytes"

* Sometimes, we want to treat an object as just "plain old data" (a contiguous sequence of bytes in memory).
* Obscure-but-real rule: inspecting an object's bytes through `unsigned char*` (or `std::byte*`. is a special, permitted aliasing case; other type-punning pointer casts can violate strict aliasing).
* "Little-endian" (common on x86 today. is a property of how multibyte values are stored, not a property of pointers).

### 7. Free store (`new`/`delete`. and lifetime pitfalls)

* Usually two or more words per allocation are used for free-store management.
* The main problems with free store are:

  * Leaked objects: People use `new` and then forget to `delete` the allocated object.
  * Premature deletion: People delete an object that they have some other pointer to and later use that other pointer.
  * Double deletion.
* Wherever possible, have that manager object being a scoped variable.
* Whenever possible, use standard-library facilities in preference to fiddling with pointers and bytes.
* Obscure fact: `new T[0]` is well-formed in standard C++; it returns a pointer value that must be released with `delete[]` (exactly once).

### 8. String literals and escape forms

* A string literal is statically allocated.
* The backslash convention exists for representing nongraphic characters.
* Raw string literals use the `R"(ccc)"` notation for a sequence of characters `ccc`.
* Unless you work with regular expressions, raw string literals are probably just a curiosity.
* Similarly, a string with the prefix `LR`, such as `LR"(str)"`, is a raw wide string.

### 9. `const` vs `constexpr`

* Basically, `constexpr`'s role is to enable and ensure compile-time evaluation, whereas `const`'s primary role is to specify immutability in interfaces.
* A constant expression is an expression that a compiler can evaluate.
* The address of a statically allocated object, such as a global variable, is a constant.

### 10. References, rvalue references, and move semantics

* To reflect the lvalue/rvalue and const/non-const distinctions, there are three kinds of references:

  * lvalue references: to refer to objects whose value we want to change
  * const references: to refer to objects whose value we do not want to change (a constant).
  * rvalue references: to refer to objects whose value we do not need to preserve after we have used it (a temporary).
* We use rvalue references to implement a "destructive read" for optimization.
* Since `move(x.` does not move `x` (it simply produces a rvalue reference to `x`, it would have been better if `move()` had been called `rval()`).
* Rvalue references can also be used to provide perfect forwarding.
* Reference to reference is just a reference. This is sometimes known as reference collapse.

### 11. Enums, bitwise operations, and switch/case constraints

* A plain enum can be implicitly converted to an integer type and used as an operand to bitwise logical operations.
* In general, prefer the enum classes.
* The underlying type must be one of the signed or unsigned integer types; the default is `int`. We could be explicit about that.
* `enum` is a user-defined type, so we can define the `|` and `&` operators.
* A plain enum can be unnamed.
* The expression in the case labels must be a constant expression of an integral or enumeration type.
* Symbolic names should be used systematically to avoid "magic numbers" in code.
* In many cases, plain enumerators are another alternative to consts.
* `int` size could be checked with `static_assert` so that work with masks.

### 12. Layout and representation details (classes, bit-fields, unions.

* Use of multiple access specifiers (that is, `public`, `private`, or `protected` can affect layout).
* Bit field saves data space, but the size of the code needed to manipulate these variables increases.
* The language doesn't keep track of which kind of value is held by a union.
* Union that holds current type info is often called a tagged union or a discriminated union.
* An anonymous union is an object, not a type, and its members can be accessed without mentioning an object name.

### 13. Other Pointer Facts

* Legit use of `intmax_t` is for formatting, and probably serialization.
* Obscure pitfall `std::addressof` vs overloaded `operator&`: `operator&` can be overloaded. If you need the real address of an object, use `std::addressof()`
* Treating arbitrary objects as "just bytes" is only reliably legal through `char*`, `unsigned char*`, or `std::byte*`. Other reinterpret-cast pointer reads can be undefined behavior.
* A pointer-to-member can be a "fat" representation (especially with multiple inheritance/virtual bases, it is, in fact, a structure). It may not fit in `void*` and is often not meaningful to print.
* Object pointers convert to `void*`; function pointers do not.
  `T* → void*` is a standard conversion for *object pointers*. A *function pointer* is a different category and is not guaranteed convertible to `void*` in standard C++. If you see code that casts a function pointer to `void*`, it's relying on a non-portable extension/ABI behavior.

---
---

## Reference Facts: C++ References (C++98 → C++17)

This note is a "facts + pitfalls" companion to `reference_facts.cpp`.  
Focus: lvalue references, rvalue references, how references are different from pointers, and obscure corner cases.

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

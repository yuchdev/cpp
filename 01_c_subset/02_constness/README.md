### Runtime/Compile-time constness

* `const` is not "compile-time constant": `const int x = f();` is still runtime-initialized; it's just read-only *after* initialization.
* `constexpr` is "can be constant-evaluated" -- not "will be": a `constexpr` function can run at runtime too. Constant evaluation happens when the context requires it.
* `const` affects a type system; "constant expression" affects evaluation: `const` participates in overload resolution and template matching; "constant expression" is about whether the compiler may/has-to evaluate now.
* `const` member functions are about the `this` type: in `T::foo() const`, `this` is `T const*`. But `mutable` fields ignore this rule and can be modified inside `const` functions.
* Top-level vs low-level const:
    * `const int* p` = pointer to const (low-level const)
    * `int* const p` = const pointer (top-level const)
    * template deduction treats these very differently.
* `constexpr` variables are implicitly `const`: `constexpr int k = 3;` is `const int` (and may be ODR-used issues apply if you take addresses etc., though C++17 inline vars eased this).
* `constinit` is *only* about initialization timing: it guarantees static initialization (no dynamic init / order fiasco), but the variable can still be mutable: `constinit int g = 42;` is allowed.
* `consteval` is "must be compile-time": calling a `consteval` function in a runtime-only context is ill-formed. Great for "enforced compile-time validation".
* `const` does not imply thread-safety: `const` only restricts mutation through that expression/type. The same object can be mutated elsewhere (non-const alias, `mutable`, atomics, `const_cast` UB if originally const).
* `volatile` is not "const but opposite": it's about observable side effects and preventing certain optimizations; it's not a concurrency primitive. `volatile const` exists and is occasionally meaningful for memory-mapped registers.

If you want, I can turn these into "flashcards" with tiny runnable examples (each fact in ~10–20 lines) so you can drop them into your Study C++ notes or a quiz script.

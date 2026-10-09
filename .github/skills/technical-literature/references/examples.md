# Technical Literature Examples

These examples calibrate the style. Short contrastive examples show local choices; the extended example shows how the style behaves across a continuous paragraph. Examples 4 and 5 show the chapter-specific habits: prose before code, and prose instead of a bullet list.

## Example 1 — Documentation versus literature

Raw meaning:

> Signed integer overflow is undefined behavior. This is important because compiler optimizations may assume that it never occurs.

Good:

> Signed integer overflow is undefined behavior in C++. That is more than a formal warning in the standard: the optimizer is allowed to reason as if such overflow never occurs. Code that appears to rely on wraparound can therefore behave very differently once optimization is enabled.

Too documentation-like:

> Do not overflow signed integers. Validate the input before performing arithmetic.

Why inadequate: the instruction may be useful, but it removes the conceptual explanation.

Too casual:

> Signed overflow is basically compiler chaos, so never do it.

Why bad: memorable, but technically imprecise and exaggerated.

## Example 2 — Controlled informality

Raw meaning:

> A pointer is not necessarily just an integer containing a memory address.

Good:

> It is tempting to think of a pointer as nothing more than an integer containing an address. On many machines that model works surprisingly well—until it doesn't. The C++ abstraction is deliberately stronger than that mental shortcut, and modern architectures make the distinction increasingly important.

Bad:

> A pointer constitutes an abstract semantic entity whose ontological status transcends its numerical representation.

Why bad: academic inflation obscures a straightforward technical point.

## Example 3 — Preserve qualification

Raw meaning:

> This optimization is usually cheap on modern desktop CPUs, but this is not guaranteed.

Good:

> On modern desktop CPUs, this optimization is usually cheap. Treat that as an engineering expectation, however, not a language-level guarantee.

Bad:

> This optimization is cheap on modern CPUs.

Why bad: removes the qualification and turns a practical observation into a guarantee.

## Example 4 — Prose before the example

Raw meaning:

> `const` on a pointer member does not make the pointee const.

Good (heading, explanation, code, interpretation):

> ### `const` is shallow
>
> A `const` object promises that its own subobjects will not be modified through it, and it is natural to read that promise as covering everything the object can reach. It does not. When a class holds a pointer, the pointer is a subobject and becomes `const`; the thing it points at is a different object, and the language has no reason to extend the qualifier to it. This is why a `const` member function can legitimately modify data owned through a pointer member, and why "logically const" has to be enforced by design rather than by the type system. The example below makes the distinction visible.
>
> ```cpp
> struct Holder { int* p; };
> static void touch(const Holder& h) { *h.p = 42; }  // compiles: p is `int* const`, not `const int*`
> ```
>
> Notice that the function takes a `const Holder&` and still writes an `int`. The pointer itself cannot be reseated, but the pointee can be modified freely. If the pointee should be protected too, the interface has to say so, for example by returning `const int&` from an accessor.

Too documentation-like (heading followed by code and a list):

> ### `const` is shallow
>
> ```cpp
> struct Holder { int* p; };
> static void touch(const Holder& h) { *h.p = 42; }
> ```
>
> * `const` applies to the pointer, not the pointee;
> * use `const int*` for a read-only pointee.

Why inadequate: the reader gets the verdict without the intuition that fails, the reason the rule exists, or what to look for in the code.

## Example 5 — Prose instead of a bullet list

Raw meaning (notes):

> * unsigned wraps
> * signed overflow is UB
> * mixed comparisons convert signed to unsigned

Good:

> The three integer behaviors that cause most production surprises are related but not alike. Unsigned arithmetic is defined to wrap modulo a power of two, so an unsigned overflow is a perfectly legal operation that may nevertheless be a logic error. Signed overflow, by contrast, is undefined behavior, which means the optimizer may assume it never happens. And when the two kinds meet in a comparison, the usual arithmetic conversions quietly turn the signed operand into an unsigned one, so a negative value becomes a very large positive one before the comparison is evaluated.

Why good: each fact keeps its qualification, and the connecting reasoning (what they share and where they differ) is stated rather than left to the reader.

## Extended example

A lock-free queue is not automatically faster than a queue protected by a mutex. Under light contention, a conventional lock may be extremely cheap, while a lock-free algorithm can pay for additional atomic operations, cache-line traffic, and retry loops on every access. The real advantage is usually about progress guarantees and behavior under contention rather than a universal reduction in latency. That distinction matters when benchmarking: if the workload never creates the conditions the algorithm was designed to handle, the benchmark may faithfully measure the wrong thing.

# Chapter 13. Undefined Behavior: The Boundary Between C++ Programs and Machine Code

*Closing chapter of Book I — The Low-Level Language*

The machine has no concept of an invalid C++ reference. A processor can load bytes from an address that once belonged to an object, calculate a sum that exceeds the range of a signed integer, or interpret an arbitrary sequence of bits as another type. Some of these operations produce a hardware exception; many execute quietly. It is tempting to call the result “whatever the machine does.”

That is not the C++ model.

C++ programs are defined against an **abstract machine**. The language specifies which operations are valid and which observable results a conforming implementation must preserve. Machine instructions are merely one way to implement that model. When source code crosses a boundary for which the C++ standard imposes no requirements, the compiler is not obliged to preserve the programmer's intuitive simulation of the underlying hardware.

This is **undefined behavior**, usually abbreviated *UB*. Its importance is difficult to overstate. Undefined behavior is simultaneously a source of security vulnerabilities, a constraint on every low-level optimization, and one of the most common sources of disagreement between experienced programmers who are looking at the same code through different mental models.

A dangerous expression may produce the expected result in a debug build and disappear in an optimized build. A pointer can retain its numeric address after the object at that address has ceased to exist. An array access can be invalid even if the memory happens to lie inside a larger allocation. A loop can race between threads even on a machine where the relevant aligned load is naturally atomic. None of these contradictions is resolved by inspecting the register values alone. The missing information is the source program's contract.

This closing chapter brings together the central ideas of Book I: fundamental and integer types, constness, floating-point behavior, pointer and reference semantics, arrays, enumerations and masks, function calls, linkage, namespaces, and C interoperability. The accompanying [`ub.cpp` demonstration](ub.cpp) contains sixteen intentionally hazardous examples. They are not intended as a test suite to run all at once: the source currently leaves every example uncalled from `main`. That is a sensible default for a collection of undefined-behavior demonstrations.

We will examine why those examples are invalid, what the optimizer is entitled to infer, which contemporary facilities make defined alternatives practical, and what a professional verification process can—and cannot—establish. We will end not with the claim that all C++ can be made safe through a compiler flag, but with the more useful engineering principle that **every low-level optimization depends on a precise model of the operations the program is allowed to perform**.

## 1. What undefined behavior means

The C++ standard distinguishes between programs and executions for which it specifies observable behavior and cases in which it imposes no requirements. An invalid operation can fall into the latter category even if it compiles, links, and appears to execute normally.

A particularly simple example is a null pointer dereference:

~~~cpp
int* p = nullptr;
// int x = *p; // undefined behavior if evaluated
~~~

The language does not require a particular exception, signal, crash, printed value, or recovery procedure. On a conventional operating system, an actual load from address zero will often fault. But that observed hardware outcome is not the C++ guarantee. The compiler may transform or eliminate the load, and the memory mapping policy itself is not a portable language property.

It is equally wrong to interpret UB as “the program is guaranteed to become chaotic.” A compiler can generate an ordinary machine load and the machine can produce a repeatable number. That observation does not establish defined behavior, because correctness is not determined by what happened in one run.

The standard's broad definition often encourages an exaggerated slogan: “Once UB exists, anything anywhere in the program can happen.” As a warning against relying on an invalid execution, it captures an important risk. As a model for debugging and optimization, it is too imprecise.

Consider:

~~~cpp
int value_or_zero(const int* p)
{
    if (p == nullptr)
        return 0;
    return *p;
}
~~~

On the null path, the function does not dereference the pointer. A compiler is not free to treat the mere *possibility* of a later null dereference as justification for deleting the prior guard from all defined executions. When `p` is non-null, the caller must still ensure that it points to an accessible live `int` object and that access is properly synchronized.

Undefined behavior matters on executions that violate relevant rules or preconditions. It is the *reachability of an invalid operation*, together with the compiler's knowledge of language constraints, that drives the subtle optimization effects.

## 2. Undefined, unspecified, and implementation-defined are different categories

Not every difference between compilers is undefined behavior. The standard deliberately leaves some choices to implementations or permits several results while retaining requirements.

| Category | Meaning | Engineering response |
|---|---|---|
| **Undefined behavior** | The standard imposes no requirements for the relevant invalid case | Remove the invalid operation or use an explicitly documented extension |
| **Unspecified behavior** | The implementation may choose among permitted outcomes without documenting which one | Do not rely on one choice unless all permitted outcomes are acceptable |
| **Implementation-defined behavior** | The implementation makes a choice that it must document | Consult and test the target toolchain contract |
| **Well-defined behavior** | The applicable language and library rules determine the required semantics | Verify preconditions and apply the specified result |

The exact evaluation order of some function arguments is a familiar example of unspecified ordering. Consider two independent side effects:

~~~cpp
int first();
int second();

consume(first(), second());
~~~

The order in which the argument expressions are evaluated is not simply guaranteed to be left-to-right. When the side effects are independent, different permitted orders need not create UB. The correct program should not depend on a particular order unless the language guarantees it for that construct.

For implementation-defined behavior, the precise implementation and standard version matter. Standard C++ has prescribed rules for many fundamental representations, but still leaves properties such as the widths of certain integer types to the implementation within specified constraints. A program that depends on `sizeof(long) == 8` needs an actual target contract; the C++ type name alone is not enough.

Some descriptions of UB also mention *ill-formed programs*, including ill-formed programs for which no diagnostic is required, or *unspecified values*. Those are separate standard categories. A malformed declaration rejected by the compiler is not usefully described as an executed runtime operation with UB.

A disciplined code review names the precise category. Calling every portability concern “undefined behavior” weakens rather than strengthens technical reasoning.

## 3. Why the language has undefined behavior

C++ is designed to map useful abstractions onto a wide range of hardware without requiring an expensive runtime check after every operation. Some rules are therefore expressed as preconditions: a pointer dereference requires a suitable object; an array access requires a valid index; a signed arithmetic operation must avoid results that the type cannot represent under ordinary language rules.

A compiler can optimize assuming that a *well-defined execution* respects those preconditions. It may conclude that code paths requiring signed overflow are unreachable, that pointers to live unrelated objects obey particular aliasing constraints, or that a non-null object pointer points to a valid object of an accessible type.

This design has historical reasons. C and early C++ were used for kernels, compilers, runtimes, device control, and numerical computing. Requiring every integer addition to check overflow or every pointer access to consult runtime metadata would have imposed costs and representational constraints that the languages deliberately avoided.

But UB should not be romanticized as a performance feature. A vulnerability is not made acceptable by being fast, and modern compilers can often produce excellent code from checked or strongly typed abstractions. The real engineering goal is to express a *precise, valid contract* that permits optimization without hiding invalid execution paths.

For instance, unsigned integers deliberately wrap modulo `2^N` for their width, while ordinary signed integer overflow is undefined. If a hash, counter, or mask genuinely requires modular arithmetic, an unsigned type is often the correct semantic choice. If a financial quantity must not exceed a limit, a checked operation or a wider representation is more appropriate. Choosing one behavior explicitly is better than depending on the machine's accidental response to an invalid operation.

## 4. How optimizers reason about forbidden paths

Suppose a programmer tries to detect signed overflow after performing the addition:

~~~cpp
bool increased(int x)
{
    return x + 1 > x;
}
~~~

For every input on which the addition is defined, the comparison is true. At the maximum representable `int`, the addition overflows and the program has UB. An optimizer can therefore compile this function as a constant `true` result when reasoning about defined executions.

The programmer may have imagined that two's-complement machine arithmetic wraps `INT_MAX` to a negative value and that the comparison detects the wrap. The source language does not supply that contract for signed addition.

The correct design tests *before* performing the invalid operation:

~~~cpp
#include <limits>
#include <optional>

std::optional<int> increment_checked(int x)
{
    if (x == std::numeric_limits<int>::max())
        return std::nullopt;
    return x + 1;
}
~~~

Here every executed addition is representable. A return value distinguishes success from overflow; the caller can choose how to handle failure.

### 4.1 A guard before an operation is different from a guard after it

The placement of a check matters:

~~~cpp
int guarded(const int* p)
{
    if (!p)
        return 0;
    return *p;
}
~~~

This is a valid null check, subject to the separate requirements for non-null pointers.

By contrast:

~~~cpp
int too_late(const int* p)
{
    const int result = *p;
    if (!p)
        return 0;
    return result;
}
~~~

The dereference occurs before the check. On defined executions, `p` cannot be null at that point, so an optimizer may treat the later condition as redundant. The check does not make the earlier access valid retroactively.

The same principle applies to bounds tests, allocation failure, integer ranges, and pointer lifetime. Validate *preconditions before the operation whose legality depends on them*.

### 4.2 Optimization is not the cause of the defect

A debug build can appear to work because fewer transformations are performed, registers are spilled, memory is initialized by unrelated activity, or addresses happen to remain mapped. An optimized build may expose the problem by moving operations, eliminating branches, vectorizing loops, or using stronger assumptions.

It is a mistake to diagnose the optimized behavior as a compiler bug without first proving that the source was well-defined. Compiler bugs exist, of course, but undefined source invalidates the argument “the debugger showed these exact bytes, therefore the optimizer must preserve my intended result.”

Compiler Explorer, disassembly, and controlled optimization comparisons are valuable tools. They explain the implementation's choices. They do not replace the standard's abstract-machine rules.

## 5. Null dereferences and invalid pointers

The demonstration [`dereference_null()`](ub.cpp) assigns a null pointer and then reads through it. The invalidity is immediate: `nullptr` points to no object.

~~~cpp
int* ptr = nullptr;
// int value = *ptr; // UB
~~~

The most obvious repair is to check for null before dereference. But null checks cover only one part of pointer validity. A non-null pointer can still refer to destroyed storage, an object of an incompatible type, an address with inadequate alignment, or memory that is concurrently modified without synchronization.

An API that accepts `T*` may choose to allow null as an explicit “no object” value. If null is not meaningful, a reference or a validated non-null wrapper can communicate a stronger interface contract. Even then, the programmer must ensure that the reference's target remains alive for the entire use.

### 5.1 Pointer validity is not address plausibility

A pointer printed in a debugger can look like a valid address even after its object has died. Comparing it to a known allocation range does not necessarily prove that a live object of the correct type exists there.

The object model gives meaning to accesses. The address is only one piece of information. This will recur in the examples involving deleted objects, misalignment, type punning, and container invalidation.

## 6. Arrays: an out-of-bounds access is not a harmless neighbor

The examples [`buffer_overflow()`](ub.cpp) and [`array_out_of_bounds()`](ub.cpp) perform writes and reads beyond built-in array bounds.

~~~cpp
char buffer[5]{};
// buffer[10] = 'A'; // UB
~~~

The valid indices are `0` through `4`. An access to index `10` does not become valid because the stack frame happens to contain another object at the corresponding machine address.

A read is not automatically safer:

~~~cpp
int values[5]{};
// int x = values[5]; // UB
~~~

An out-of-bounds read can access unrelated memory, disclose information, or be optimized under assumptions that make the programmer's intended control flow unreliable.

### 6.1 The one-past pointer is not a one-past object

C++ permits constructing or computing a pointer one past the end of an array in specified circumstances:

~~~cpp
int values[5]{};
int* end = values + 5; // permitted one-past pointer
~~~

That pointer can participate in certain comparisons and iterator-like operations. It does not point to an element that can be dereferenced:

~~~cpp
// int x = *end; // UB
~~~

This distinction underlies standard iterators and half-open ranges. A range's end iterator is a boundary marker, not an element. The language deliberately supports one-past positions so algorithms can express traversal without requiring an imaginary sentinel object in memory.

### 6.2 Safer views do not make bounds automatic

`std::array` offers fixed extent and checked `.at()`. `std::vector` owns a dynamically sized array and offers checked access through `.at()`. C++20 `std::span` represents a borrowed contiguous range including its extent.

~~~cpp
#include <array>
#include <span>

std::array<int, 5> values{};
std::span<int> view = values;

if (index < view.size())
    consume(view[index]);
~~~

A span carries size information but does not own its storage. It can dangle when its owner dies or reallocates, and unchecked indexing still requires a valid index. Modern abstractions move information into types and make validation convenient; they do not remove the need to reason about lifetimes and bounds.

## 7. Signed overflow and arithmetic preconditions

The [`signed_overflow()`](ub.cpp) demonstration evaluates one more than the maximum representable `int`:

~~~cpp
int max = std::numeric_limits<int>::max();
// int result = max + 1; // UB
~~~

Many real processors perform modular two's-complement addition at the instruction level. That observation does not make ordinary signed integer overflow defined in C++.

Unsigned arithmetic is different. For an unsigned integer with `N` value bits, arithmetic wraps modulo `2^N` under its rules:

~~~cpp
unsigned u = std::numeric_limits<unsigned>::max();
unsigned wrapped = u + 1u; // defined: zero
~~~

Use the latter when modular arithmetic is genuinely required, but remember that unsigned underflow can still be a logic defect even though it is not UB. A bounds check written as `index - 1 < size` can behave badly when `index` is zero, even though every unsigned operation is defined.

### 7.1 Overflow detection must precede the operation

For addition with a nonnegative increment:

~~~cpp
bool can_add(int x, int delta)
{
    if (delta < 0)
        return false;

    return x <= std::numeric_limits<int>::max() - delta;
}
~~~

The example explicitly restricts `delta` to nonnegative values. A general checked addition must account for both positive and negative operands without overflowing while performing the test.

A practical generic implementation can use compiler overflow intrinsics on supported targets, a carefully specified wider intermediate, or a standard checked arithmetic facility when available.

C++26 adds C-style checked integer arithmetic functions `ckd_add`, `ckd_sub`, and `ckd_mul` through `<stdckdint.h>` where implemented. Their intended result is an overflow indication along with the converted result, without first invoking ordinary signed-overflow UB. Availability and supported type requirements must be feature-tested rather than assumed from the compiler's language-mode flag.

### 7.2 Narrowing, shifts, and division have separate rules

Overflow is not the only arithmetic hazard. Integer division by zero is undefined. For signed division, the exceptional case of the minimum value divided by `-1` also matters when the quotient cannot be represented.

~~~cpp
int divide_checked(int a, int b)
{
    if (b == 0)
        throw std::domain_error("division by zero");

    if (a == std::numeric_limits<int>::min() && b == -1)
        throw std::overflow_error("unrepresentable quotient");

    return a / b;
}
~~~

This code defines an error policy instead of evaluating invalid division.

Shift operations require a valid shift count, and some signed-shift cases have version-specific rules. A left shift by a negative count or a count not less than the promoted left operand's width is undefined. Signed left shifting has additional value-range constraints that differ by edition and must not be casually replaced with “the CPU masks the count.”

For bit masks, prefer well-defined unsigned operations and explicit bounds checks. In low-level code, the precise width of the promoted operand is often more important than the width of the source expression's apparent type.

## 8. Division by zero and the difference between a trap and UB

The [`division_by_zero()`](ub.cpp) example divides one integer by zero. A machine instruction may trap or the operating system may deliver a signal commonly called `SIGFPE`, even though no floating-point calculation is involved.

The signal is a typical implementation consequence, not a C++ exception guarantee. The compiler is permitted to reason about the operation as invalid before choosing any machine instruction.

Floating-point division by zero is not governed by exactly the same simple rule. On implementations using IEC 60559/IEEE-style semantics, it can yield infinities or NaNs with relevant floating-point status flags. Compiler options, the floating-point environment, and exceptional-value support affect the contract.

Conflating floating-point exceptional values with signed integer UB leads to bad numerical APIs. Domain validation should match the actual type and intended mathematical meaning.

## 9. Uninitialized reads: data does not become valid by occupying storage

The demonstration [`uninitialized_var()`](ub.cpp) declares an automatic `int` without giving it a value and then reads it.

~~~cpp
int value;
// consume(value); // invalid use of an uninitialized value
~~~

The declaration itself is not automatically UB. The problem arises when the object is evaluated in a way not permitted by the language's uninitialized-value rules.

An unrelated example in the source is named `evaluation_order()`:

~~~cpp
// int i = i * 0;
~~~

Despite the name, the defect is not primarily a disagreement about operand evaluation order. It reads `i` while computing its own initializer, before `i` has been assigned a usable value. Multiplying the unknown value by zero does not rescue the expression.

A correct version initializes first:

~~~cpp
int i = 0;
i *= 0;
~~~

This illustrates a recurring principle: **a mathematical identity cannot make an invalid source operand valid**. An optimizer is entitled to use valid arithmetic identities, but that reasoning does not provide a legal value for an uninitialized object.

### 9.1 C++26's erroneous-value model

Historically, the standard's description of uninitialized automatic storage heavily emphasized *indeterminate values* and UB. C++26 introduces additional terminology for *erroneous values* and *erroneous behavior*, distinguishing some invalid evaluations from UB under the updated language rules.

That change matters to formal wording, diagnostics, and tools. It does not make reading an unwritten ordinary `int` a meaningful way to obtain random bits. The correct engineering rule remains: initialize the value or prove that every execution writes it before use.

There are narrowly defined exceptions for byte-oriented types and certain operations involving uninitialized-friendly values. Those exceptions support low-level storage handling and should not be generalized to arbitrary scalar reads.

### 9.2 Finding uninitialized uses

Compiler warnings such as `-Wuninitialized` and `-Wmaybe-uninitialized` can catch many cases. Their effectiveness depends on compiler and optimization level.

Clang's MemorySanitizer can identify numerous uses of uninitialized data on supported configurations, usually with the requirement that relevant dependencies also be instrumented. AddressSanitizer and UndefinedBehaviorSanitizer do not replace MemorySanitizer: they cover different categories.

The companion source contains a Clang-specific pragma suppressing uninitialized warnings. That is understandable in an intentionally invalid teaching example, but it would be undesirable in production source. The ability to hide a diagnostic is not evidence that the operation became defined.

## 10. Dangling pointers: storage and object lifetime are not synonymous

The [`expired_pointer()`](ub.cpp) demonstration stores the address of a local object and uses the pointer after the local's lifetime ends:

~~~cpp
int* p = nullptr;
{
    int x = 5;
    p = &x;
}
// int value = *p; // UB: x no longer exists
~~~

The stack memory may still contain the integer `5`, and the pointer may print the same address. Neither observation proves that an `int` object named `x` is still alive there.

The distinction between storage and lifetime is central to advanced C++. Storage can remain allocated while one object is destroyed and another is constructed in the same bytes. A container can retain capacity while invalidating iterators or pointers to erased elements. A placement-new operation can reuse an address without making all previously held pointers universally valid.

### 10.1 Container reallocation is an ordinary source of dangling

~~~cpp
std::vector<int> values{1, 2};
int* first = values.data();

values.push_back(3); // may reallocate
// *first may now refer to invalidated storage
~~~

The problem is not limited to explicit `delete`. Any operation that can move a container's elements may invalidate raw pointers, references, spans, or iterators.

Reserve strategies can reduce reallocations, but are not lifetime proofs after arbitrary mutations. Use indices or stable handles when identity across structural changes is required, and document invalidation rules in APIs that return views.

### 10.2 Smart pointers encode ownership, not universal access safety

`std::unique_ptr<T>` expresses exclusive ownership. `std::shared_ptr<T>` expresses shared ownership of an object through a control block. Both prevent many manual deletion errors.

They do not automatically make every borrowed `T*` valid. If code extracts a raw pointer from a unique owner and then destroys the owner, the raw pointer dangles. A shared pointer can keep an object alive while other threads still cause data races on its mutable fields.

Resource safety therefore requires both **ownership** and **usage protocols**. In synchronous code, a reference may be perfectly safe when its owner dominates the call. In asynchronous code, ownership transfer or a carefully managed weak-reference protocol may be needed.

## 11. Use-after-delete and stale references

The [`deleted_pointer()`](ub.cpp) example dereferences storage after `delete`. The [`dangling_reference()`](ub.cpp) example retains a C++ reference to an object whose allocation has been freed.

~~~cpp
int* p = new int(7);
int& ref = *p;

delete p;

// int a = *p;  // UB
// int b = ref; // UB
~~~

The second access is no safer because it uses a reference instead of pointer syntax. References do not own the object to which they refer, and they do not automatically rebind when that object disappears.

This is an important counterexample to the intuition that “references are safe pointers.” References impose useful binding and syntactic constraints, but the lifetime relation still belongs to the program.

Use-after-free is particularly dangerous in security-sensitive code because the freed allocation may be reused for another object. A stale reference can then access or overwrite data belonging to an unrelated owner. The memory allocator's reuse pattern can make the failure intermittent and difficult to reproduce.

AddressSanitizer is highly effective at catching many executed use-after-free cases, but its red zones and quarantine are diagnostic implementation techniques. A passing run is not a proof that every possible stale access has been exercised.

## 12. Virtual dispatch after destruction

The [`call_non_exist_virtual()`](ub.cpp) example creates a derived object, calls a virtual function, deletes the object, and then calls a virtual function through the stale base pointer.

~~~cpp
struct Base {
    virtual void run() {}
    virtual ~Base() = default;
};

struct Derived : Base {
    void run() override {}
};

Base* p = new Derived;
p->run();
delete p;

// p->run(); // UB
~~~

The obvious cause is use-after-delete. On many ABIs, the invalid call also tries to use virtual-dispatch metadata associated with an object that no longer exists.

It is inaccurate, however, to reduce the language rule to “the vtable is missing.” The standard defines object lifetime and virtual dispatch semantics, not a universal mandatory vtable representation. A particular ABI may store a virtual-function pointer in an object's memory; another implementation strategy remains possible.

A further hazard appears *during* construction and destruction, when virtual calls follow special rules concerning the current construction/destruction stage. Code must not assume that an object being constructed or torn down behaves as a fully alive most-derived object for every dispatch and cast.

The fix is ownership discipline: do not invoke members through stale pointers, and use a virtual destructor when deleting a derived object through a base-class pointer under the relevant rules.

## 13. Type punning, aliasing, and object existence

The demonstrations [`type_punning_pointer()`](ub.cpp) and [`break_strict_aliasing()`](ub.cpp) reinterpret an `int` object's address as `float*` and read through that pointer.

~~~cpp
int bits = 1;
float* p = reinterpret_cast<float*>(&bits);
// float value = *p; // not a valid general type-punning technique
~~~

A cast can produce a pointer value, but it does not automatically create a live `float` object in the same storage. Accessing an object through an incompatible glvalue type can violate the type-access rules that support alias analysis.

This is distinct from alignment. An address can be correctly aligned for both types and still fail the object-lifetime and type-access rules.

### 13.1 Why aliasing rules matter to optimization

An optimizer may reason that accesses through pointers to unrelated types do not refer to the same object, except where the standard explicitly permits aliasing. That assumption allows it to retain values in registers, reorder loads, and vectorize loops.

If the programmer secretly makes an `int*` and a `float*` refer to the same object through invalid casts, the compiler can produce results different from a byte-by-byte machine simulation. The problem is not that alias analysis was “too aggressive.” The program asserted an invalid memory-access relationship.

Character and byte-oriented access have special representation-inspection rules. Objects can generally be examined through appropriate character or `std::byte` views, but that permission does not mean that arbitrary bytes constitute a live object of every possible type.

### 13.2 Defined alternatives: `std::memcpy` and `std::bit_cast`

For copying a trivially copyable object's representation into an equal-sized trivially copyable destination, C++20 `std::bit_cast` is often concise:

~~~cpp
#include <bit>
#include <cstdint>

std::uint32_t bits_of_float(float value)
{
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    return std::bit_cast<std::uint32_t>(value);
}
~~~

This illustration assumes the source and destination representations meet the operation's requirements. The operation is not guaranteed to produce meaningful values for arbitrary target types with invalid representations.

`std::memcpy` is another useful tool for moving object representations and for parsing binary data into suitable trivially copyable objects under the applicable lifetime rules:

~~~cpp
float value = 0.0f;
std::uint32_t bits{};
static_assert(sizeof(value) == sizeof(bits));
std::memcpy(&bits, &value, sizeof(bits));
~~~

Neither facility automatically performs network byte-order conversion, validates protocol fields, or removes the need to reason about padding and representations.

### 13.3 `reinterpret_cast` is not an object factory

A pointer conversion does not by itself allocate storage or start the lifetime of an arbitrary object:

~~~cpp
alignas(double) unsigned char storage[sizeof(double)]{};

auto* p = reinterpret_cast<double*>(storage);
// This cast alone does not establish a live double object.
~~~

C++20 and C++23 refined implicit object creation and lifetime-management facilities for certain implicit-lifetime types, including `std::start_lifetime_as` in C++23. Those facilities have exact preconditions and should not be treated as universal permission to cast arbitrary packets into C++ objects.

When parsing external bytes, prefer explicit decoding or well-defined construction. The lifetime of the target object matters as much as the address and alignment.

## 14. Alignment is a semantic requirement

The [`misalignment()`](ub.cpp) example allocates a `char` object, converts its address to `int*`, and dereferences it.

~~~cpp
char* raw = new char;
int* p = reinterpret_cast<int*>(raw);
// int value = *p; // invalid object access; alignment may also fail
delete raw;
~~~

This example has more than one problem. A single `char` allocation does not provide a live `int` object, and its storage may not satisfy the alignment and size required for an `int` on the target.

Alignment rules are not merely performance advice. Accessing an object through an improperly aligned pointer can violate language requirements even on a CPU that tolerates unaligned machine loads.

### 14.1 Aligned storage is not enough on its own

A legitimate low-level allocator must provide suitable storage, start the required object's lifetime through a legal mechanism, and eventually end that lifetime and release storage appropriately.

~~~cpp
alignas(int) std::byte storage[sizeof(int)];
int* p = ::new (static_cast<void*>(storage)) int(42);

consume(*p);
p->~int();
~~~

This placement-new example is illustrative for manual lifetime management. It creates an `int` in suitably aligned storage; merely casting `storage` to `int*` would not express the same construction operation.

Most application code should prefer ordinary objects, `std::array`, `std::vector`, smart pointers, or allocator-aware containers. Manual storage management belongs where the abstraction truly requires it and where lifetime proofs can be maintained.

## 15. Allocation families cannot be mixed

The [`realloc_fails()`](ub.cpp) demonstration passes memory allocated with `new` to C's `realloc`.

~~~cpp
int* p = new int(7);
// p = static_cast<int*>(std::realloc(p, 100)); // UB
delete p;
~~~

`realloc` is designed for pointers from the compatible C allocation family under its preconditions. It does not understand C++ constructors, object destruction, class-specific allocation functions, or arbitrary memory managers.

Even when an implementation routes `new` internally through `malloc`, the programmer is not entitled to assume those families are interchangeable. The source-language and library contracts, not a particular allocator's implementation, decide validity.

The normal C++ solution for resizable owned sequences is a container:

~~~cpp
std::vector<int> values{7};
values.resize(100);
~~~

This properly manages element lifetimes, capacity, and deallocation according to the container's rules. It may still invalidate pointers and references when it reallocates, so ownership safety and view validity must both be considered.

At C/C++ interoperability boundaries, provide matching exported `create`/`destroy` or `allocate`/`release` functions rather than asking the caller to guess how an object was obtained.

## 16. A missing return is a control-flow error, not a random integer source

A non-`void`, non-`main` function that reaches its closing brace without returning a value ordinarily has undefined behavior when that path is taken.

~~~cpp
int classify(int x)
{
    if (x > 0)
        return 1;

    // Missing return when x <= 0
}
~~~

Compilers commonly diagnose this with `-Wreturn-type` or an equivalent warning. A caller must not treat the absent return as producing a predictable register value.

The companion [`missed_return()`](ub.cpp) function contains an important subtlety: its local variable is initialized to `1`, and the condition `i == 1` leads directly to a return. The path that falls off the end is therefore unreachable in that particular function as written. The *general rule* about falling off a non-void function remains important, but we must not claim the existing demonstration necessarily executes UB merely because the function text contains a possible-looking missing return.

To demonstrate the actual failure, use an externally supplied argument or a path whose condition may be false, and run it under a controlled diagnostic configuration.

This is a reminder that code reviews should distinguish **potentially invalid syntax or control flow** from **an invalid path actually reachable in the program**.

## 17. Sequencing rules changed, but side effects still require care

Older C++ discussions frequently used expressions such as `i++ + i++` to illustrate unsequenced modifications. C++11 introduced a more explicit sequencing model, and C++17 strengthened sequencing guarantees for several expressions and operators.

The exact edition matters. An expression that was undefined under older evaluation-order rules can have well-defined sequencing under later rules, while other expressions remain unsequenced or have unspecified evaluation order.

A frequently misunderstood example is:

~~~cpp
int i = 0;
// int result = i++ + i++; // unsequenced conflicting modifications
~~~

This expression is not made safe merely by enabling C++20. The source still contains conflicting side effects without the required sequencing.

By contrast, the language specifies sequencing for the built-in comma operator:

~~~cpp
int i = 0;
int first = (i++, i); // the increment is sequenced before reading i
~~~

That code has defined sequencing, though it is rarely the clearest way to express the intent.

A normal function call with two independent arguments can be well-defined even if their evaluation order is unspecified. The engineering task is not to insist on one universal left-to-right rule; it is to separate required sequencing from acceptable variability and actual UB.

Where side effects interact, prefer separate statements:

~~~cpp
const int a = compute_a();
const int b = compute_b();
consume(a, b);
~~~

The sequence is visible, auditable, and independent of subtle expression rules.

## 18. Data races are undefined behavior even without invalid pointers

Concurrency introduces a form of UB that may look perfectly legal when each thread's code is read in isolation.

~~~cpp
int counter = 0;

// Thread A:
// ++counter;

// Thread B:
// ++counter;
~~~

Without synchronization, conflicting accesses to the same memory location from different threads can create a **data race** under the C++ memory model. At least one access is a modification, the operations are not appropriately atomic, and there is no required happens-before ordering.

The fact that an aligned machine load or store happens to be indivisible does not make a compound increment atomic. Even two simple machine accesses are not automatically ordered under the C++ language model.

### 18.1 Use a synchronization contract

For an independently maintained count:

~~~cpp
std::atomic<unsigned> counter{0};

void increment()
{
    counter.fetch_add(1, std::memory_order_relaxed);
}
~~~

Relaxed ordering can be sufficient for a counter whose value is not used to publish unrelated data. It is not a universal replacement for release/acquire or mutex synchronization.

For example, publishing an initialized object through a flag often requires a happens-before relationship:

~~~cpp
int payload = 0;
std::atomic<bool> ready{false};

// Producer:
payload = 42;
ready.store(true, std::memory_order_release);

// Consumer:
// if (ready.load(std::memory_order_acquire))
//     consume(payload);
~~~

This illustrates the synchronization principle in a simplified two-thread protocol where publication happens once and the payload is not concurrently modified after publication. In real code, the full lifecycle, repeatability, and shutdown conditions require explicit design.

### 18.2 `volatile` does not solve thread synchronization

A volatile-qualified variable does not provide atomic increments or establish a general inter-thread happens-before relationship in C++. Using `volatile bool stop` as a universal communication flag is therefore unsound.

Volatile may have roles in memory-mapped I/O under platform-specific constraints, but those concerns require separate hardware memory-ordering and device semantics. Treat ordinary concurrent software communication with atomics, locks, or other well-defined synchronization mechanisms.

### 18.3 Atomic pointers do not solve reclamation

An atomic pointer can safely publish a pointer value under an appropriate protocol, yet a reader may still dereference the object after another thread deletes it.

~~~cpp
std::atomic<Node*> head{nullptr};
// Atomic publication alone does not protect Node lifetime.
~~~

Lock-free algorithms must handle reclamation. Hazard pointers, epoch-based reclamation, RCU, reference counting, and locks solve different aspects of the problem. ABA and memory reuse complicate reasoning further.

C++26 standard hazard-pointer and RCU library facilities extend the available vocabulary where implemented, but they do not retroactively make arbitrary raw pointer sharing safe.

The key question is not only whether the pointer value is read atomically; it is whether the pointed-to object remains alive and accessible for the whole operation.

## 19. The object model is more demanding than the memory map

Several previous examples share a theme: a pointer seems to address real allocated bytes, yet the access is invalid. Alignment, lifetime, and type-access requirements can all matter independently.

Consider memory recycled by an arena allocator. It can remain mapped and writable while the original object is destroyed and another object is constructed in the same storage. A stale pointer can compare equal to the new object's address, but that does not automatically entitle it to access the new object under every circumstance.

C++ has detailed rules about transparent replacement of objects, pointer reuse, placement construction, and `std::launder` in situations where lifetime or identity changes. These should not be reduced to “same address means same object.”

For performance-sensitive data structures, it is often better to represent stable identity with an explicit handle, generation number, index, or owner-managed reference rather than assuming address identity is permanent.

The core object model is not a cosmetic layer over machine memory. It is the reason the optimizer can make precise assumptions and the programmer can reason about object lifetime without depending on accidental allocation layouts.

## 20. Floating-point behavior belongs to a different numerical contract

Book I's floating-point chapter distinguishes mathematical real numbers from finite machine representations. That distinction becomes especially important when discussing UB.

An ordinary floating-point overflow may produce infinity on an implementation supporting IEEE/IEC 60559-style semantics. Invalid floating operations may yield NaNs, and comparisons involving NaNs have specified peculiarities. These are not the same language rules as signed integer overflow.

Compiler flags such as `-ffast-math` can change floating-point optimization assumptions and permitted transformations under a toolchain-specific contract. A program that depends on NaN propagation, signed zero, or particular rounding behavior should not casually enable such options.

Floating-point-to-integer conversions also need care. A conversion whose truncated value cannot be represented in the destination integer type can have undefined behavior, even when the source expression is a perfectly valid floating-point value.

~~~cpp
double large = 1e100;
// int value = static_cast<int>(large); // UB if out of range
~~~

A numerical API should check representable ranges and define its handling of NaNs, infinities, and rounding before conversion.

The general principle is to avoid importing the rules of one arithmetic domain into another. Signed integers, unsigned integers, floating-point values, and bit-level representations have different contracts.

## 21. Library preconditions are part of defined behavior

Not all UB originates in a built-in operator. Standard library facilities can impose preconditions on their callers. Violating such a precondition may invoke undefined behavior even if the machine operation itself appears innocuous.

Calling `vector::operator[]` with an out-of-range index is a familiar example in ordinary unchecked container use. Dereferencing an end iterator, using an invalidated iterator, accessing an empty `optional` through an unchecked operation, or supplying an invalid comparator to an algorithm can similarly violate required contracts.

A particularly subtle example is a comparator that is not a strict weak ordering:

~~~cpp
auto invalid_less = [](int a, int b) {
    return a <= b;
};
~~~

A sorting algorithm requiring a strict weak ordering cannot use this predicate as a valid `less` relation, because `invalid_less(a, a)` is true. The code might still compile, and small tests might yield plausible results; the algorithm's requirements are not satisfied.

### 21.1 Preconditions should be expressed in APIs

A function that requires a nonempty span or sorted input should make the requirement apparent through types, validation, documentation, or a checked interface.

For untrusted input, do not replace validation with an unchecked assertion that the input must be valid. For an internal invariant established earlier in a carefully controlled system, an assertion or contract may be appropriate, provided it does not conceal a recoverable error case.

This distinction becomes especially significant with C++23 `std::unreachable` and C++26 contracts.

## 22. `std::unreachable` and optimizer assumptions

C++23 added `std::unreachable()` in `<utility>`. It declares that control flow cannot reach the call in a well-defined program. Reaching it is undefined behavior.

~~~cpp
#include <utility>

// C++23, only if the state is provably impossible:
// std::unreachable();
~~~

This is not a checked assertion that throws or reliably terminates. It is an unchecked promise to the implementation. If the programmer's invariant is false, the compiler can use that promise to eliminate code or perform transformations that would otherwise be inappropriate.

Never use `std::unreachable` as a generic response to untrusted input or an unexpected enum value. A defensive default case should report an error or fail through a defined policy unless every possible value is already provably excluded.

The same caution applies to compiler extensions such as `__builtin_unreachable`, `__assume`, and vendor “assume” attributes. They can improve optimization under genuine invariants, but an invalid assumption destroys the program's basis for defined execution.

## 23. Contracts in C++26

C++26 introduces standard language support for contract assertions with preconditions, postconditions, and assertion statements, subject to the selected semantics and toolchain support. The details of how a violation is handled depend on the contract evaluation configuration and the language model.

Contracts are not a universal replacement for input validation. A contract that expresses “this index must be in range” can document or enforce a relationship in some build modes, but the program must still decide what to do with untrusted out-of-range input when recovery is required.

Nor does writing a contract make an invalid pointer dereference defined. If an unchecked operation's precondition is violated, an annotation does not retroactively create a valid object or change the aliasing rules.

A sound design separates *validation at trust boundaries* from *invariants established inside the implementation*. It also treats build-time contract mode as part of the deployment specification.

## 24. A systematic method for studying undefined behavior

The educational [`ub.cpp`](ub.cpp) contains intentionally dangerous functions but leaves `main` returning zero without calling them. That makes it possible to compile the examples without automatically executing a collection of invalid operations.

The correct way to study one is to isolate a single case, understand which operation violates the language rule, and then use compiler diagnostics and sanitizers to observe an implementation's behavior.

A typical Clang or GCC-style command for a suitable isolated test is:

~~~sh
c++ -std=c++20 -O1 -g \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    isolated_case.cpp -o isolated_case
./isolated_case
~~~

The actual file in the repository is named `ub.cpp`, not `ub_examples.cpp` as an earlier README example suggested. Since its `main` does not call the hazardous functions, compiling and running it unchanged will generally not exercise those defects. To test one, build a temporary harness or separate copy that invokes exactly the intended function.

Do not run all examples sequentially and expect meaningful results after the first invalid operation. A sanitizer may abort immediately; the process may crash; or the implementation may continue in an unconstrained state.

### 24.1 Sanitizers are complementary, not interchangeable

| Tool | Particularly useful for | Important limitation |
|---|---|---|
| **AddressSanitizer (ASan)** | Many buffer overflows, use-after-free, and stack lifetime errors | Cannot prove correct object semantics on unexecuted paths |
| **UndefinedBehaviorSanitizer (UBSan)** | Selected invalid arithmetic, shifts, type/alignment and other UB operations | Does not cover every UB category; instrumentation differs |
| **MemorySanitizer (MSan)** | Use of uninitialized data | Limited platform/toolchain support; benefits from fully instrumented dependencies |
| **ThreadSanitizer (TSan)** | Many executed data races | Cannot prove race freedom; synchronization support and availability vary |
| **Static analysis** | Suspicious paths, missing returns, lifetime and aliasing hazards | Can miss complex cases or report false positives |

ASan, UBSan, MSan, and TSan do not all work together in one universal invocation. Some combinations are unsupported or unhelpful; choose the tool matching the suspected defect and supported target.

Warnings such as `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wreturn-type` are valuable. GCC's `-Wmaybe-uninitialized` and Clang's uninitialized-related warnings can catch common errors. MSVC offers `/W4 /permissive- /analyze`, with separate sanitizer support on appropriate targets.

A serious investigation also needs assertions, property tests, fuzzing, and code review. Dynamic instrumentation reports only executed cases, and a clean run is not proof of absence of UB.

### 24.2 Compare optimization modes carefully

If an invalid program behaves differently with `-O0` and `-O3`, comparing generated assembly can show what transformation changed the outcome. That is useful evidence about how the compiler reasons.

However, turning optimization off is not a correctness fix. The source remains invalid on executions that violate the relevant rules. Likewise, `-fno-strict-aliasing` may disable some alias-based optimizations without creating missing object lifetimes or repairing alignment. `-fwrapv` may establish a compiler-specific signed-wrap contract while leaving ISO C++ portability unresolved.

Use toolchain flags as explicitly documented modes, not as substitutes for valid source semantics.

## 25. Designing interfaces that make UB harder to express

The best low-level abstractions do not merely check errors after they occur. They make correct usage easy and invalid operations more visible.

Instead of exposing a pointer without its length, accept `std::span<T>` or pointer-plus-length when crossing a C ABI. Instead of asking callers to remember how a resource was allocated, return a type with a matching deleter. Instead of handing out references to container elements with unknown invalidation rules, provide stable handles or document borrowing precisely.

Instead of relying on signed overflow to signal a limit, make the range check explicit. Instead of type punning through unrelated pointer types, use a representation-copying or object-lifetime facility with defined preconditions. Instead of updating a shared counter through an ordinary reference, use an atomic or mutex under a documented memory-ordering protocol.

Every one of these choices has performance implications. Some add checks, some add metadata, some change ownership. But the correct comparison is not between “unvalidated raw code” and “slow abstraction” as slogans. It is between implementations of the *same well-defined operation* under representative workloads.

A compiler can optimize a clear span-based loop aggressively when the bounds, aliasing, and lifetimes are understood. An unchecked pointer loop may be faster in one benchmark, but a benchmark that silently invokes UB is not measuring a valid alternative implementation of the same contract.

### 25.1 Trust boundaries and internal invariants

A useful architectural distinction is between **external data**, which may be malformed, and **internal invariants**, which the implementation establishes and preserves.

At an external boundary, validate lengths, conversions, enum values, structure versions, and pointer contracts. Inside a component, use types and invariants to avoid repeating every check where they are already proven. When an invariant is performance-critical, document the proof and test the construction paths that establish it.

Unchecked assumptions belong only where the invariant is genuinely guaranteed. If a failure remains possible due to network input, disk corruption, race conditions, or caller misuse, replacing validation with `std::unreachable` may turn a recoverable condition into UB.

## 26. Security implications: exploitability is not required for a bug to matter

Buffer overruns, use-after-free, double deletion, type confusion, and data races form a major class of security vulnerabilities in native code. Attackers can sometimes influence allocation patterns, input lengths, or object lifetimes to make a latent invalid access exploitable.

Yet an operation need not be demonstrably exploitable to be a correctness problem. UB can corrupt a financial calculation, break a lock-free data structure under rare scheduling, or produce a wrong checksum in an embedded controller without allowing code execution.

Security-oriented reviews therefore focus on the contract and reachability, not merely on whether a proof-of-concept crash exists.

Fuzzing with sanitizers is particularly useful for parser and protocol boundaries, where malformed inputs can exercise unusual lengths, nesting structures, integer conversions, and resource states. Concurrency tests require additional synchronization reasoning because scheduling variability makes race coverage inherently incomplete.

When handling untrusted data, prioritize explicit parsing, checked arithmetic, bounded views, validated formats, and resource ownership. These techniques protect both security and reliability.

## 27. ABI and cross-language boundaries do not excuse UB

Chapters 10–12 explored linkage, namespaces, and C/C++ interoperability. Those mechanisms make independently compiled components connectable, but they do not relax the object model.

An `extern "C"` function can still receive an invalid pointer, a mismatched callback calling convention, or a structure whose layout differs between producer and consumer. A dynamically loaded plugin can still call a function after its code is unloaded. A shared library can still destroy an object with an incompatible allocator.

A boundary that compiles and links is only the beginning of correctness. The interface must specify scalar widths, alignment, object lifetime, allocation family, error propagation, synchronization, and versioning.

This is particularly important in cross-language systems where other runtimes have different safety expectations. JNI, Python extension APIs, and C# native interoperability each add their own rules for managed-object lifetime and thread attachment. A stable C ABI can reduce coupling, but it does not automatically provide valid memory ownership or exception propagation across every runtime.

The practical design is to keep the binary boundary small and explicit, with rich C++ abstractions on the implementation side and a carefully tested handle or buffer protocol at the interface.

## 28. What changed between C++98 and C++26

**C++98/03.** The familiar abstract-machine model already included undefined behavior for invalid pointers, out-of-bounds access, signed overflow, and broken object lifetimes. The C++ type system offered references, classes, constructors, and destructors, but programmers still frequently managed arrays and dynamic storage manually.

**C++11.** Move semantics and smart pointers made ownership easier to encode in types. The standardized memory model established precise data-race and synchronization requirements. Thread-safe initialization of function-local statics removed an important initialization race.

**C++14.** Relaxed constexpr restrictions allowed more computations to be checked in constant-evaluation contexts, while the fundamental runtime UB rules remained.

**C++17.** Several expression sequencing guarantees were strengthened. New library vocabulary and guaranteed copy-elision cases refined object lifetime and construction reasoning, but did not make dangling references or invalid casts safe.

**C++20.** `std::span` offered a standard non-owning range view; `std::bit_cast` supplied a defined representation-conversion facility for suitable types; concepts and improved constant evaluation made more preconditions expressible in generic APIs. The language also refined implicit object creation for certain implicit-lifetime types.

**C++23.** `std::unreachable` made unchecked impossibility assumptions explicit. `std::start_lifetime_as` and related facilities offered more precise lifetime tools under stated preconditions. These are advanced mechanisms, not general-purpose safety switches.

**C++26.** The model of uninitialized values adds erroneous-value terminology; checked integer arithmetic, contracts, and standardized hazard-pointer/RCU facilities provide additional vocabulary for writing and analyzing low-level code. Feature availability and final toolchain support must be tested, and none of these developments abolishes UB.

Over nearly three decades, C++ has accumulated more ways to encode valid operations and more tools for diagnosing invalid ones. The language has not attempted to replace its entire low-level model with mandatory runtime checks.

## 29. For developers coming from Java, Python, and C#

Managed runtimes often define exceptions for array bounds violations, invalid casts, or null accesses. Garbage collection may also ensure that an object remains allocated while it is reachable through managed references.

It is natural to carry those expectations into C++ and assume that a bad pointer will reliably throw, an array access will always be checked, or a reference keeps its referent alive. Those assumptions are wrong.

C++ exposes lower-level operations whose preconditions are the programmer's responsibility. A native pointer is not a garbage-collected strong reference. A `std::span` is a borrowed view, not an owning array. A `reinterpret_cast` does not perform the runtime compatibility checks of Java's class casts or C#'s managed type system. An invalid arithmetic operation may have undefined behavior rather than a language exception.

The right translation is not to abandon familiar high-level design principles. Use value types, RAII, owning containers, variants, optional results, checked boundaries, and explicit concurrency protocols. The language offers many of the same architecture-level benefits when the appropriate abstraction is chosen.

The key difference is that C++ gives more direct control over the machine and consequently requires the program to establish more of its own safety contract.

## 30. The final Book I lesson: machine knowledge is necessary but insufficient

The first twelve chapters developed a vocabulary for reasoning close to the machine. They explained how fundamental types are represented; how const qualification affects access; how integer widths, promotions, and conversions interact; how floating-point behavior differs from real arithmetic; how pointers, references, arrays, enums, and bit masks operate; how functions and callable objects depend on signatures and lifetimes; and how separate compilation, linkage, namespaces, and C compatibility organize complete systems.

Undefined behavior is not an isolated thirteenth topic. It is the boundary condition underneath all of them.

A signed integer is meaningful only within the arithmetic model chosen by the language. A pointer is valid only with the required provenance, lifetime, type-access, and alignment conditions. A reference is safe only while its target exists. A function call is legitimate only when its declaration, ABI, and argument contracts agree. A lock-free algorithm is correct only when atomic ordering and memory reclamation form one coherent protocol. A binary interface is reliable only when both sides agree on the representation and ownership of what crosses it.

Understanding the assembly instructions does not remove these requirements. Indeed, deep machine knowledge can sometimes be misleading when it tempts the programmer to replace abstract-machine rules with “what this processor usually does.” A compiler optimizes *C++ semantics*, not a promise to preserve every plausible hand-written machine trace.

This is also why the solution is not to abandon performance. The solution is to pursue performance from a defined program. Measure complete workloads. Use carefully selected integer types. Maintain stable object lifetimes. Make aliasing relationships explicit. Validate input at trust boundaries. Prefer abstractions that put important preconditions into types. Use sanitizers, warnings, static analysis, fuzzing, and multi-compiler testing as complementary tools.

No individual tool proves complete correctness. But each can reduce the gap between what the programmer believes the program means and what the C++ abstract machine actually requires.

A final low-level engineering rule is worth remembering:

> **The most dangerous optimization is not the one that removes a check. It is the one that relies on a precondition the program never actually established.**

Book I ends here because this principle underlies every advanced use of C++ that follows: concurrency, allocators, SIMD, object representation, high-performance containers, systems interfaces, and the design of zero-overhead abstractions. The goal of advanced C++ is not merely to make the machine do something quickly. It is to make the *specified program* do the right thing quickly, across compilers, architectures, and time.

## Further reading

The language references for [undefined behavior](https://en.cppreference.com/w/cpp/language/ub), [object lifetime](https://en.cppreference.com/w/cpp/language/lifetime), [object representation and type aliasing](https://en.cppreference.com/w/cpp/language/reinterpret_cast), [evaluation order](https://en.cppreference.com/w/cpp/language/eval_order), [constant expressions](https://en.cppreference.com/w/cpp/language/constant_expression), and the [C++ memory model](https://en.cppreference.com/w/cpp/language/memory_model) provide the exact rules that matter when a case appears ambiguous.

The library references for [`std::bit_cast`](https://en.cppreference.com/w/cpp/numeric/bit_cast), [`std::span`](https://en.cppreference.com/w/cpp/container/span), [`std::unreachable`](https://en.cppreference.com/w/cpp/utility/unreachable), and [`std::start_lifetime_as`](https://en.cppreference.com/w/cpp/memory/start_lifetime_as) document facilities that make some difficult operations expressible without UB. Verify edition and implementation support before using the newest ones.

The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) offer practical recommendations for interfaces, bounds, lifetimes, type safety, and resource management. [Compiler Explorer](https://godbolt.org/) helps investigate optimizer assumptions and generated code. LLVM's [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html), [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html), [MemorySanitizer](https://clang.llvm.org/docs/MemorySanitizer.html), and [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) documentation explains the capabilities and limitations of specific instrumentation.

The companion [`ub.cpp`](ub.cpp) should be studied **one invalid example at a time**, preferably by extracting a scenario into an isolated test rather than modifying the repository's safe, non-invoking `main`. The strongest conclusion from any demonstration is not that a particular compiler crashes or optimizes unexpectedly, but that we can state exactly which source-level precondition was violated and how to express the intended operation in defined C++.

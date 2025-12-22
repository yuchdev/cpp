/*
================================================================================
01_function_identity_abi_callconv.cpp

Advanced topic: function identity at the ABI level.

A function is not only "R(Args...)" — ABI properties are part of its *call contract*:
  - calling convention (Windows 32-bit: __cdecl/__stdcall/etc.)
  - varargs vs non-varargs (affects register usage / promotions)
  - linkage / symbol decoration (extern "C", visibility/export)
  - platform ABI (SysV AMD64 vs Windows x64 vs AArch64, etc.)

This example focuses on *preventing ABI UB* by encoding ABI intent in types.

================================================================================
*/

#include <iostream>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

//------------------------------------------------------------------------------
// Section A: Calling conventions (Windows).
// On 32-bit Windows, calling convention mismatches are a common real-world crash.
// On 64-bit Windows, the user-level calling convention is mostly unified, but
// the annotations still matter for interop and type safety.
//------------------------------------------------------------------------------
#if defined(_MSC_VER) || defined(_WIN32)

void __cdecl   cdecl_func(int x)   { println("__cdecl   cdecl_func(", x, ")"); }
void __stdcall stdcall_func(int x) { println("__stdcall stdcall_func(", x, ")"); }

using CdeclPtr   = void(__cdecl*)(int);
using StdcallPtr = void(__stdcall*)(int);

static void demo_calling_conventions() {
    println("== Windows calling conventions ==");

    // Correctly typed pointers:
    CdeclPtr pc = &cdecl_func;
    StdcallPtr ps = &stdcall_func;

    pc(1);
    ps(2);

    // If you try to assign mismatched convention, correct typing stops you:
    //
    // CdeclPtr wrong = &stdcall_func; // ❌ ill-formed

    // The dangerous part: forcing the type with casts can compile but becomes UB.
    // Typical symptom on 32-bit: stack imbalance / corrupted return address.
    //
    // auto forced = reinterpret_cast<CdeclPtr>(&stdcall_func);
    // forced(3); // ❌ UB (do not do this)
}

#else

static void demo_calling_conventions() {
    println("== Non-Windows typical ABIs ==");

    println("On SysV AMD64/AArch64, calling convention is fixed for normal user code.");
    println("But ABI issues still exist via:");
    println("  - varargs (different argument passing rules)");
    println("  - extern \"C\" boundaries");
    println("  - structure layout/packing/align flags");
    println("  - vector/SIMD ABI differences");
}

#endif

//------------------------------------------------------------------------------
// Section B: Varargs vs non-varargs is also an ABI contract.
// On some ABIs, passing floating-point args differs for variadic calls.
//------------------------------------------------------------------------------
static int sum2(int a, int b) { return a + b; }

// A toy variadic function. Not a real sum: just demonstrates type distinction.
static int sumv(int n, ...) { return n; }

static void demo_varargs_distinction() {
    println("\n== Variadic vs non-variadic function types ==");

    using Sum2Ptr = int(*)(int,int);
    using SumVPtr = int(*)(int, ...);

    Sum2Ptr p2 = &sum2;
    SumVPtr pv = &sumv;

    println("p2(1,2) = ", p2(1,2));
    println("pv(123, ...) = ", pv(123, 1,2,3));

    // Types are distinct; assigning is ill-formed:
    //
    // Sum2Ptr bad = pv; // ❌ ill-formed
    //
    // Forcing casts => UB territory.
    println("Takeaway: varargs is part of the function type/ABI contract.");
}

//------------------------------------------------------------------------------
// Summary / lecture notes
//------------------------------------------------------------------------------
static void notes() {
    println("\n== Notes ==");
    println("* Encode ABI details in types (pointer typedefs).");
    println("* Avoid casts that 'force' ABI compatibility — they can compile but be UB.");
    println("* Prefer typed alternatives over varargs in modern C++.");
}

int main() {
    demo_calling_conventions();
    demo_varargs_distinction();
    notes();
    return 0;
}

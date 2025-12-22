#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace cpp
{
class test_me
{
  public:
    void point_to_me() {}
};

void point_to_me_static() {}

// A string literal has static storage duration; safe to return pointer to its first element.
constexpr const char* error_message(int /*i*/)
{
    return "range error";
}

// constexpr enables compile-time evaluation when context requires it.
constexpr int f(int i) { return i + 1; }

// "perfect swap" (almost)
template <class T>
void swap(T& a, T& b)
{
    T tmp{std::move(a)};
    a = std::move(b);
    b = std::move(tmp);
}
} // namespace cpp

// ============================================================================
// Pointer facts and pitfalls (obscure / interesting)
// ============================================================================

static void show_bytes_of_object()
{
    std::puts("\n== Bytes of an object (endianness + representation) ==");

    // NOTE: object representation can be inspected via unsigned char / std::byte.
    // This is one of the few "legal aliasing" escape hatches.
    long example = 1L;

    auto* bytes = reinterpret_cast<const unsigned char*>(&example);

    std::cout << "long example = " << example << ", sizeof(long) = " << sizeof(long) << "\n";
    std::cout << "Bytes: ";
    for (std::size_t i = 0; i < sizeof(example); ++i)
    {
        // Print the byte value, not the pointer address.
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned>(bytes[i]) << ' ';
    }
    std::cout << std::dec << "\n";

    std::puts("Fact: byte order (endianness) is a property of representation, not of pointers.");
    std::puts("Fact: reading object bytes via unsigned char is explicitly permitted.");
}

static void pointer_sizes_and_alignment()
{
    std::puts("\n== Pointer size & alignment facts ==");

    std::cout << "sizeof(void*)  = " << sizeof(void*) << "\n";
    std::cout << "alignof(void*) = " << alignof(void*) << "\n";
    std::cout << "sizeof(int*)   = " << sizeof(int*) << "\n";
    std::cout << "sizeof(char*)  = " << sizeof(char*) << "\n";

    // Obscure: function pointers are not required to have the same size/representation as object pointers.
    std::cout << "sizeof(void(*)()) = " << sizeof(void (*)()) << "\n";

    // Obscure: pointer-to-member can be larger than a regular pointer (multiple inheritance / virtual bases).
    std::cout << "sizeof(void (cpp::test_me::*)()) = " << sizeof(void (cpp::test_me::*)()) << "\n";

    std::puts("Fact: object pointers, function pointers, and member pointers are distinct categories.");
    std::puts("They are NOT required to be interchangeable or same-sized.");
}

static void address_and_addressof()
{
    std::puts("\n== Every object has an address; std::addressof defeats operator& tricks ==");

    struct Weird
    {
        int x{42};
        void* operator&() { return reinterpret_cast<void*>(0xDEADBEEF); } // don't do this in real code
    };

    Weird w;
    // &w calls overloaded operator&
    std::cout << "Overloaded &: " << &w << "\n";

    // std::addressof gets the real address.
    Weird* real = std::addressof(w);
    std::cout << "std::addressof: " << real << "\n";

    std::puts("Obscure fact: operator& can be overloaded; std::addressof is the escape hatch.");
}

static void constness_on_pointer_vs_pointee()
{
    std::puts("\n== constness: pointer vs pointee ==");

    int a{};
    int*       p  = &a;     (void)p;
    int* const p1 = &a;     (void)p1;  // const pointer
    const int* p2 = &a;     (void)p2;  // pointer to const
    int const* p3 = &a;     (void)p3;  // same as p2
    const int* const p4 = &a; (void)p4;

    std::puts("Rule of thumb: const next to * applies to pointee; const at the end applies to pointer.");
}

static void pointer_arithmetic_rules()
{
    std::puts("\n== Pointer arithmetic rules (where UB hides) ==");

    int arr[10] = {0,1,2,3,4,5,6,7,8,9};

    int* first = arr;
    int* fifth = first + 5; // OK: points into the array

    // One-past-the-end is allowed for forming a pointer, but not for dereferencing.
    int* one_past = arr + 10; // OK to compute, compare, subtract within same array
    std::cout << "one_past = " << one_past << " (do not dereference)\n";

    // Safe subtraction within same array (including one-past):
    std::ptrdiff_t dist = one_past - first;
    std::cout << "one_past - first = " << dist << "\n";

    // UB examples (left as comments intentionally):
    // int* before = arr - 1;     // forming pointer before array is UB in modern rules
    // int  x = *one_past;        // dereference one-past is UB
    // std::ptrdiff_t bad = &arr[0] - &some_other_array[0]; // subtract unrelated pointers => UB

    std::puts("Fact: pointer subtraction is only defined when both pointers refer into the same array object");
    std::puts("(including one-past). Otherwise: undefined behavior.");
}

static void void_pointer_facts()
{
    std::puts("\n== void* facts ==");

    int x = 7;
    void* vp = &x;

    // You cannot do arithmetic on void* in standard C++ (GNU C extension allows it).
    // ++vp; // ill-formed in standard C++

    // You must cast back to a complete type to dereference.
    int* ip = static_cast<int*>(vp);
    std::cout << "*static_cast<int*>(vp) = " << *ip << "\n";

    std::puts("Fact: void* is a generic object pointer, but it is not an iterator.");
}

static void nullptr_and_nullptr_t()
{
    std::puts("\n== nullptr and std::nullptr_t ==");

    // nullptr converts to any object pointer and any function pointer, and any pointer-to-member.
    int* ip = nullptr;
    void* vp = nullptr;
    (void)ip; (void)vp;

    void (*fp)() = nullptr;
    void (cpp::test_me::*mp)() = nullptr;

    // Overload resolution: nullptr is not an integer (unlike NULL/0).
    auto overload = [](auto) {};
    (void)overload;

    std::cout << "decltype(nullptr) is std::nullptr_t? "
              << std::boolalpha
              << std::is_same_v<decltype(nullptr), std::nullptr_t>
              << "\n";

    std::puts("Obscure fact: you cannot take the address of nullptr (it's a pure literal).");
    // auto pnull = &nullptr; // ill-formed

    std::puts("Practical fact: prefer nullptr over 0/NULL to avoid overload ambiguity.");
}

static void function_pointer_vs_object_pointer()
{
    std::puts("\n== Function pointers vs object pointers ==");

    void (*fp)() = &cpp::point_to_me_static;

    std::cout << "function pointer fp = " << reinterpret_cast<void*>(fp) << " (printed via cast)\n";

    // Standard C++ does NOT guarantee that you can convert function pointer <-> void*.
    // Many platforms allow it, but it is not portable.
    // void* vp = (void*)fp; // non-portable / ill-formed without explicit cast; still not guaranteed

    std::puts("Fact: object pointers convert to void*; function pointers generally do not.");
}

static void strict_aliasing_escape_hatches()
{
    std::puts("\n== Strict aliasing: what pointer casts DO and DO NOT allow ==");

    struct S { int a; float b; } s{1, 2.0f};

    // UB: reading through an incompatible type pointer violates strict aliasing:
    // int* pi = reinterpret_cast<int*>(&s.b);
    // std::cout << *pi << "\n";

    // Legal: access object representation via unsigned char.
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(&s);
    std::cout << "First byte of S object representation: "
              << static_cast<unsigned>(raw[0]) << "\n";

    std::puts("Obscure fact: only char/unsigned char/std::byte may alias arbitrary object storage safely.");
}

static void new_delete_and_zero_length_arrays()
{
    std::puts("\n== new/delete and zero-length dynamic arrays ==");

    // Standard allows new T[0]; returns non-null unique pointer (commonly),
    // but you must delete[] it exactly once.
    int* p = new int[0];
    std::cout << "new int[0] returned: " << p << "\n";
    delete[] p;

    // NOTE: delete on memory allocated with new[] is UB; must match forms.
    // delete p; // UB

    std::puts("Fact: new T[0] is well-formed; it returns a pointer you must pass to delete[].");
}

static void pointer_comparisons()
{
    std::puts("\n== Pointer comparisons: what is and isn't meaningful ==");

    int a = 1, b = 2;
    int* pa = &a;
    int* pb = &b;

    std::cout << "pa=" << pa << ", pb=" << pb << "\n";

    // Equality/inequality comparisons are always well-formed:
    std::cout << "pa == pb ? " << (pa == pb) << "\n";

    // Relational comparisons (<, >) for unrelated objects are not generally meaningful.
    // They are allowed syntactically, but ordering is only specified within arrays / certain cases.
    // Prefer std::less<> if you truly need a strict weak ordering of pointers for associative containers.
    std::less<int*> less;
    std::cout << "std::less(pa,pb) = " << less(pa, pb) << "\n";

    std::puts("Fact: use std::less<> for ordering pointers; raw < may be unspecified for unrelated objects.");
}

static void pointer_integer_roundtrip()
{
    std::puts("\n== Pointer <-> integer round-trip (intptr_t/uintptr_t) ==");

#ifdef INTPTR_MAX
    int x = 123;
    void* p = &x;

    std::uintptr_t u = reinterpret_cast<std::uintptr_t>(p);
    void* p2 = reinterpret_cast<void*>(u);

    std::cout << "p=" << p << " -> uintptr_t=0x" << std::hex << u << std::dec << " -> p2=" << p2 << "\n";
    std::cout << "round-trip preserved? " << std::boolalpha << (p == p2) << "\n";

    std::puts("Caveat: even if uintptr_t exists, some platforms restrict meaningful pointer-int round-trips.");
#else
    std::puts("uintptr_t not available on this target (optional typedef).");
#endif
}

static void pointer_to_member_obscurity()
{
    std::puts("\n== Pointer-to-member obscurities ==");

    void (cpp::test_me::*mp)() = &cpp::test_me::point_to_me;
    cpp::test_me obj;

    // Call member function pointer:
    (obj.*mp)();

    cpp::test_me* pobj = &obj;
    (pobj->*mp)();

    std::puts("Fact: pointer-to-member is not an address; it can be a fat descriptor.");
    std::puts("It may not fit into void* and often cannot be meaningfully printed.");
}

static void pointer_provenance_note()
{
    std::puts("\n== Pointer provenance (advanced / optimizer-facing) ==");

    // This is mostly a *theory* / optimizer rule: compilers track which allocation an address came from.
    // Even if you manufacture an integer equal to some address and cast it to a pointer,
    // the optimizer may assume it's not a valid pointer to a live object (depending on model).
    //
    // In practice: avoid "inventing" pointers from integers unless you truly interact with OS/hardware.
    std::puts("Obscure fact: compilers may assume pointers originate from valid objects (provenance model).");
    std::puts("Practical: avoid constructing pointers from integers except in low-level OS/hardware code.");
}

void nullptr_type()
{
    std::puts("\n== nullptr examples (object/function/member pointers) ==");

    int* inull{nullptr};
    void* vnull{nullptr};
    (void)inull;
    (void)vnull;

    void (*static_func_ptr)() = &cpp::point_to_me_static;
    void (cpp::test_me::*method_ptr)() = &cpp::test_me::point_to_me;

    std::cout << "Function pointer static_func_ptr = " << reinterpret_cast<void*>(static_func_ptr) << "\n";
    std::puts("Member function pointer printing is not portable; skipping direct print.");

    static_func_ptr = nullptr;
    method_ptr = nullptr;

    nullptr_and_nullptr_t();
    function_pointer_vs_object_pointer();
}

void pointer_conversions()
{
    std::puts("\n== Multi-level pointer cv-qualification pitfalls ==");

    std::cout << "C++ version: " << __cplusplus << '\n';

    char** p = nullptr;

    // const char** p1 = p; // ERROR: would allow writing const char* into char*
    const char* const* p2 = p; (void)p2;        // OK
    volatile char* const* p3 = p; (void)p3;     // OK
    volatile const char* const* p4 = p2; (void)p4; // OK

    double* a[2][3] = {};
    // double const * const (*ap)[3] = a; // ERROR pre-C++20 (cv qualification rules)
#if __cplusplus >= 202002L
    // C++20 relaxed some of these array-pointer conversions.
    double* const (*ap1)[3] = a;
    (void)ap1;
#endif

    std::puts("Fact: the 'const char**' trap prevents writing a const pointer into a non-const slot.");
}

void show_const_pointers()
{
    std::puts("\n== Const pointers and dynamic allocation ==");

    const int* pp1 = new const int(3);

    // delete expects a pointer to non-const? No: delete works fine with const pointee type.
    delete pp1;

    // new const int; // would default-initialize const int (ill-formed / useless depending on form)
    // new const int[100]; // const arrays are almost always a design smell

    int* pp5 = new int[0];
    delete[] pp5;
}

void show_ptrs_refs()
{
    std::puts("\n== Pointers vs references vs arrays ==");

    void (*static_func_ptr)() = &cpp::point_to_me_static;
    void (cpp::test_me::*method_ptr)() = &cpp::test_me::point_to_me;

    // void* v1 = static_func_ptr; // not portable / generally ill-formed
    // void* v2 = method_ptr;      // ill-formed

    static_func_ptr = nullptr;
    method_ptr = nullptr;

    std::vector<int> vv1{1, 2, 3};
    std::vector<int> vv2{4, 5, 6};
    cpp::swap(vv1, vv2);

    int arr[] = {7, 2, 5, 9, 5, 3, 5, 8, 0, 3};
    std::sort(std::begin(arr), std::end(arr));

    std::puts("Fact: built-in arrays decay to pointers in most expressions, but not in sizeof/decltype/&.");
}

void show_string_literals()
{
    std::puts("\n== String literal facts ==");

    const char* qs = R"(quoted string)";
    const char* complicated = R"("('(?:[^\\']|\\.)*'|\"(?:[^\\\"]|\\.)*\")|")";
    const char* with_returns = R"(atatat
tatatat)";

    std::cout << qs << "\n" << complicated << "\n" << with_returns << "\n";

    const wchar_t* long_complicated = LR"("('(?:[^\\']|\\.)*'|\"(?:[^\\\"]|\\.)*\")|")";
    std::wcout << long_complicated << L"\n";

    std::puts("Fact: string literals have static storage duration and are immutable in C++.");
    std::puts("Modifying them through a non-const pointer is undefined behavior.");
}

namespace cpp
{
class tagged_union
{
  public:
    struct bad_tag {};

    void set_integer(int i)
    {
        tag_ = tag::integer;
        i_ = i;
    }
    void set_pointer(int* pi)
    {
        tag_ = tag::pointer;
        pi_ = pi;
    }

    int integer() const
    {
        if (tag_ != tag::integer) throw bad_tag();
        return i_;
    }
    int* pointer() const
    {
        if (tag_ != tag::pointer) throw bad_tag();
        return pi_;
    }

  private:
    enum class tag { integer, pointer };
    tag tag_{tag::integer};

    union
    {
        int  i_{0};
        int* pi_;
    };
};
} // namespace cpp

void show_union_tags()
{
    std::puts("\n== Tagged union demo ==");

    cpp::tagged_union tu;
    int i = 0;
    tu.set_integer(1);
    tu.set_pointer(&i);

    // Reading inactive union member is UB; tagged-union is a manual discipline.
    std::cout << "tagged_union pointer() = " << tu.pointer() << "\n";
}

int main()
{
    pointers_facts();
    nullptr_type();
    pointer_conversions();
    show_const_pointers();
    show_ptrs_refs();
    show_string_literals();
    show_union_tags();
    return 0;
}

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <version>

namespace cpp
{
class test_me
{
public:
    int value{42};

    void point_to_me() {}
};

void point_to_me_static() {}

// A string literal has static storage duration; returning a pointer to its first element is safe.
constexpr const char* error_message(int /*i*/)
{
    return "range error";
}

constexpr int f(int i) { return i + 1; }

// Educational move-based swap. Production code should normally use std::swap.
template <class T>
void swap(T& a, T& b)
{
    T tmp{std::move(a)};
    a = std::move(b);
    b = std::move(tmp);
}
} // namespace cpp

// ============================================================================
// Pointer facts and pitfalls (C++20 baseline, selected C++23 facilities gated)
// ============================================================================

static void show_bytes_of_object()
{
    std::puts("\n== Object representation: std::span + std::as_bytes (C++20) ==");

    long example = 1L;

    // C++ permits inspection of object representation through char, unsigned char,
    // and std::byte. std::as_bytes makes the intent explicit without inventing
    // an unrelated typed pointer.
    std::span<const long> object{&example, 1};
    const auto bytes = std::as_bytes(object);

    std::cout << "long example = " << example << ", sizeof(long) = " << sizeof(long) << "\n";
    std::cout << "Bytes: ";
    for (std::byte b : bytes)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << std::to_integer<unsigned>(b) << ' ';
    }
    std::cout << std::dec << "\n";

    // std::bit_cast is representation conversion, not numeric conversion.
    const auto copied_bytes =
        std::bit_cast<std::array<std::byte, sizeof(long)>>(example);
    static_assert(std::tuple_size_v<decltype(copied_bytes)> == sizeof(long));

    std::puts("Fact: byte order is a representation property, not a property of pointers.");
    std::puts("Fact: std::as_bytes/std::bit_cast are preferable to unrelated typed pointer punning.");
}

static void pointer_sizes_and_alignment()
{
    std::puts("\n== Pointer categories, size, and alignment ==");

    std::cout << "sizeof(void*)  = " << sizeof(void*) << "\n";
    std::cout << "alignof(void*) = " << alignof(void*) << "\n";
    std::cout << "sizeof(int*)   = " << sizeof(int*) << "\n";
    std::cout << "sizeof(char*)  = " << sizeof(char*) << "\n";
    std::cout << "sizeof(void(*)()) = " << sizeof(void (*)()) << "\n";
    std::cout << "sizeof(int cpp::test_me::*) = "
              << sizeof(int cpp::test_me::*) << "\n";
    std::cout << "sizeof(void (cpp::test_me::*)()) = "
              << sizeof(void (cpp::test_me::*)()) << "\n";

    std::puts("Object pointers, function pointers, and pointers-to-member are distinct categories.");
    std::puts("The language does not require them to have the same representation or size.");
}

static void address_and_addressof()
{
    std::puts("\n== std::addressof defeats overloaded operator& ==");

    struct Weird
    {
        int x{42};

        // Legal, but intentionally hostile API design for demonstration.
        Weird* operator&() { return nullptr; }
    };

    Weird w;

    std::cout << "overloaded &w is null? " << std::boolalpha << (&w == nullptr) << "\n";

    Weird* real = std::addressof(w);
    std::cout << "std::addressof(w) = " << static_cast<void*>(real) << "\n";

    std::puts("std::addressof obtains the real address even when operator& is overloaded.");
}

static void constness_on_pointer_vs_pointee()
{
    std::puts("\n== constness: pointer vs pointee ==");

    int a{};
    int* p = &a;                    // mutable pointer, mutable pointee
    int* const p1 = &a;             // const pointer, mutable pointee
    const int* p2 = &a;             // mutable pointer, const pointee
    int const* p3 = &a;             // same type as p2
    const int* const p4 = &a;       // const pointer, const pointee

    static_assert(std::is_same_v<decltype(p2), decltype(p3)>);
    (void)p;
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;

    std::puts("Read declarations from the identifier outward: const may qualify the pointer or pointee.");
}

static void pointer_arithmetic_rules()
{
    std::puts("\n== Pointer arithmetic: the array-object boundary ==");

    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    int* first = arr;
    int* fifth = first + 5;
    int* one_past = arr + 10;

    std::cout << "*fifth = " << *fifth << "\n";
    std::cout << "one_past = " << static_cast<void*>(one_past)
              << " (valid value, not dereferenceable)\n";

    const std::ptrdiff_t dist = one_past - first;
    std::cout << "one_past - first = " << dist << "\n";

    // Undefined behavior examples, deliberately not executed:
    // int* before = arr - 1;               // forms an out-of-range pointer
    // int value = *one_past;               // dereferences one-past
    // int other[1]{};
    // auto bad_distance = &arr[0] - &other[0]; // unrelated arrays

    std::puts("Pointer arithmetic is defined in terms of an array object, not arbitrary address arithmetic.");
}

static void span_and_to_address()
{
    std::puts("\n== std::span, contiguous iterators, and std::to_address (C++20) ==");

    int storage[] = {10, 20, 30, 40};
    std::span<int> view{storage};

    static_assert(std::contiguous_iterator<decltype(view.begin())>);

    // span::iterator is a contiguous iterator, but code should not assume its concrete
    // type is int*. std::to_address obtains the represented raw address.
    int* raw = std::to_address(view.begin());

    std::cout << "view.size() = " << view.size()
              << ", *std::to_address(view.begin()) = " << *raw << "\n";

    view.subspan(1, 2)[0] = 99;
    std::cout << "storage[1] after subspan write = " << storage[1] << "\n";

    std::puts("std::span carries pointer + extent semantics without owning the elements.");
}

static void void_pointer_facts()
{
    std::puts("\n== void* facts ==");

    int x = 7;
    void* vp = &x;

    // Standard C++ has no arithmetic on void*.
    // ++vp; // ill-formed

    int* ip = static_cast<int*>(vp);
    std::cout << "*static_cast<int*>(vp) = " << *ip << "\n";

    std::puts("void* is a generic object pointer. It is not a byte iterator and cannot be dereferenced directly.");
}

static void nullptr_and_nullptr_t()
{
    std::puts("\n== nullptr and std::nullptr_t ==");

    int* ip = nullptr;
    void* vp = nullptr;
    void (*fp)() = nullptr;
    void (cpp::test_me::*mp)() = nullptr;

    static_assert(std::is_same_v<decltype(nullptr), std::nullptr_t>);
    static_assert(!std::is_integral_v<std::nullptr_t>);

    std::cout << "int* null? " << std::boolalpha << (ip == nullptr) << "\n";
    std::cout << "void* null? " << (vp == nullptr) << "\n";
    std::cout << "function pointer null? " << (fp == nullptr) << "\n";
    std::cout << "member pointer null? " << (mp == nullptr) << "\n";

    // auto pnull = &nullptr; // ill-formed: nullptr is a literal, not an object

    std::puts("Prefer nullptr over 0/NULL: it participates in pointer overload resolution as a pointer-like null literal.");
}

static void function_pointer_vs_object_pointer()
{
    std::puts("\n== Function pointers vs object pointers ==");

    void (*fp)() = &cpp::point_to_me_static;
    fp();

    std::cout << "function pointer is non-null? " << std::boolalpha << (fp != nullptr) << "\n";

    // Object pointer -> void* is a standard conversion.
    int object = 1;
    void* object_address = &object;
    (void)object_address;

    // There is no corresponding implicit standard conversion from function pointer
    // to void*. Some ABIs provide explicit/conditionally-supported conversions, but
    // portable C++ should not use void* as generic storage for function pointers.
    // void* not_portable = fp; // ill-formed

    std::puts("Use the correct pointer category; do not assume void* can store every kind of pointer.");
}

static void strict_aliasing_escape_hatches()
{
    std::puts("\n== Aliasing, representation, and std::bit_cast ==");

    struct S
    {
        int a;
        float b;
    } s{1, 2.0f};

    // Undefined behavior if executed: an int lvalue is not a permitted way to read a float object.
    // int* pi = reinterpret_cast<int*>(&s.b);
    // std::cout << *pi << "\n";

    const auto raw = std::as_bytes(std::span<const S>{&s, 1});
    std::cout << "First representation byte of S: "
              << std::to_integer<unsigned>(raw.front()) << "\n";

    const auto float_bytes =
        std::bit_cast<std::array<std::byte, sizeof(float)>>(s.b);
    std::cout << "float object representation contains "
              << float_bytes.size() << " C++ bytes\n";

    std::puts("reinterpret_cast can change a pointer type; it does not grant permission to access an object through that type.");
}

static void explicit_lifetime_management()
{
    std::puts("\n== Storage is not object lifetime: construct_at/destroy_at (C++20) ==");

    struct Point
    {
        int x;
        int y;
    };

    alignas(Point) std::byte storage[sizeof(Point)];

    Point* p = std::construct_at(
        reinterpret_cast<Point*>(storage),
        Point{3, 4});

    std::cout << "constructed Point = {" << p->x << ", " << p->y << "}\n";

    std::destroy_at(p);

    // p still contains an address value, but there is no live Point there now.
    // std::cout << p->x; // UB: lifetime has ended

    std::puts("An addressable region of storage and a live T object are different concepts.");
}

#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
static void start_lifetime_as_demo()
{
    std::puts("\n== std::start_lifetime_as (C++23) ==");

    struct Header
    {
        std::uint32_t magic;
        std::uint16_t version;
        std::uint16_t flags;
    };

    static_assert(std::is_trivially_copyable_v<Header>);

    Header source{0x12345678u, 3u, 7u};
    alignas(Header) std::array<std::byte, sizeof(Header)> storage{};

    std::memcpy(storage.data(), &source, sizeof source);

    // C++23 can explicitly begin the lifetime of an implicit-lifetime type in
    // suitable storage while retaining the existing object representation.
    Header* restored = std::start_lifetime_as<Header>(storage.data());

    std::cout << "restored.version = " << restored->version
              << ", restored.flags = " << restored->flags << "\n";
}
#endif

static void new_delete_and_zero_length_arrays()
{
    std::puts("\n== new/delete, zero-length arrays, and ownership ==");

    // new T[0] is well-formed. The result must still be matched with delete[].
    int* p = new int[0];
    std::cout << "new int[0] returned: " << static_cast<void*>(p) << "\n";
    delete[] p;

    // delete p; // wrong form for storage obtained by new[] => undefined behavior

    auto owned_array = std::make_unique<int[]>(4);
    owned_array[0] = 10;

#if defined(__cpp_lib_shared_ptr_arrays) && __cpp_lib_shared_ptr_arrays >= 201707L
    // Array overloads of make_shared are available in C++20.
    auto shared_array = std::make_shared<int[]>(4);
    shared_array[0] = 20;
    std::cout << "unique array[0] = " << owned_array[0]
              << ", shared array[0] = " << shared_array[0] << "\n";
#else
    std::cout << "unique array[0] = " << owned_array[0] << "\n";
#endif

    std::puts("Raw new/delete demonstrate lifetime mechanics; ownership should normally be represented by RAII types.");
}

static void pointer_comparisons()
{
    std::puts("\n== Pointer comparison ==");

    int a = 1;
    int b = 2;
    int* pa = &a;
    int* pb = &b;

    std::cout << "pa == pb ? " << std::boolalpha << (pa == pb) << "\n";

    // Built-in relational ordering of unrelated object pointers may have an unspecified
    // result. std::less provides the implementation-defined strict total pointer order.
    const bool total_order = std::less<>{}(pa, pb);
    const bool ranges_total_order = std::ranges::less{}(pa, pb);

    std::cout << "std::less<>(pa, pb) = " << total_order << "\n";
    std::cout << "std::ranges::less{}(pa, pb) = " << ranges_total_order << "\n";
}

static void pointer_integer_roundtrip()
{
    std::puts("\n== Pointer <-> integer round-trip ==");

#ifdef UINTPTR_MAX
    int x = 123;
    int* p = &x;

    // uintptr_t is optional. Where it exists it is an unsigned integer type capable
    // of holding a converted void pointer. reinterpret_cast back to the same pointer
    // type after a sufficiently-wide integer conversion preserves the pointer value.
    const std::uintptr_t bits = reinterpret_cast<std::uintptr_t>(p);
    int* p2 = reinterpret_cast<int*>(bits);

    std::cout << "pointer = " << static_cast<void*>(p)
              << " -> integer = 0x" << std::hex << bits << std::dec
              << " -> pointer = " << static_cast<void*>(p2) << "\n";
    std::cout << "round-trip preserved? " << std::boolalpha << (p == p2) << "\n";

    std::puts("Do not treat the integer representation as a portable linear address or serialize it as identity.");
#else
    std::puts("std::uintptr_t is optional and is not available on this target.");
#endif
}

static void pointer_to_member_obscurity()
{
    std::puts("\n== Pointer-to-member is not an ordinary address ==");

    int cpp::test_me::*data_member = &cpp::test_me::value;
    void (cpp::test_me::*member_function)() = &cpp::test_me::point_to_me;

    cpp::test_me obj;
    cpp::test_me* pobj = &obj;

    std::cout << "obj.*data_member = " << obj.*data_member << "\n";
    std::cout << "pobj->*data_member = " << pobj->*data_member << "\n";

    (obj.*member_function)();
    (pobj->*member_function)();

    std::puts("A pointer-to-member can encode adjustment information; it is not required to fit in void*.");
}

struct c_resource
{
    int value;
};

static int c_create_resource(c_resource** out)
{
    if (out == nullptr)
    {
        return -1;
    }

    *out = new c_resource{73};
    return 0;
}

static void c_destroy_resource(c_resource* p)
{
    delete p;
}

#if defined(__cpp_lib_out_ptr) && __cpp_lib_out_ptr >= 202106L
static void out_ptr_demo()
{
    std::puts("\n== std::out_ptr: smart-pointer/C-API interop (C++23) ==");

    using resource_ptr =
        std::unique_ptr<c_resource, decltype(&c_destroy_resource)>;

    resource_ptr resource{nullptr, &c_destroy_resource};

    if (c_create_resource(std::out_ptr(resource)) == 0)
    {
        std::cout << "resource->value = " << resource->value << "\n";
    }

    std::puts("std::out_ptr adapts pointer-to-pointer output APIs without manually release/reset choreography.");
}
#endif

static void observer_vs_owner()
{
    std::puts("\n== Raw pointer as observer; smart pointer as owner ==");

    auto owner = std::make_unique<int>(42);
    int* observer = owner.get();

    std::cout << "*observer while owner lives = " << *observer << "\n";

    owner.reset();
    // observer now dangles. Even if a later allocation reuses the same numeric address,
    // that does not revive this pointer as a pointer to the old object.
    observer = nullptr;

    std::puts("Ownership and addressability are different properties; raw pointers do not encode ownership.");
}

void show_ptrs_refs()
{
    std::puts("\n== Arrays, decay, and ranges ==");

    std::vector<int> vv1{1, 2, 3};
    std::vector<int> vv2{4, 5, 6};
    cpp::swap(vv1, vv2);

    int arr[] = {7, 2, 5, 9, 5, 3, 5, 8, 0, 3};

    static_assert(std::is_array_v<decltype(arr)>);
    static_assert(std::size(arr) == 10);

    std::sort(std::begin(arr), std::end(arr));

    int* decayed = arr;
    (void)decayed;

    std::puts("Built-in arrays decay in many expressions, but std::span/std::size preserve extent at interfaces.");
}

void show_string_literals()
{
    std::puts("\n== String literal pointer facts ==");

    const char* qs = R"(quoted string)";
    const char* complicated = R"("('(?:[^\\']|\\.)*'|\"(?:[^\\\"]|\\.)*\")|")";
    const char* with_returns = R"(atatat
tatatat)";

    std::cout << qs << "\n" << complicated << "\n" << with_returns << "\n";

    const wchar_t* wide = LR"(wide raw string)";
    std::wcout << wide << L"\n";

    // Since C++20, u8 string literals are arrays of const char8_t, not const char.
    std::u8string_view utf8 = u8"UTF-8";
    std::cout << "u8 literal code-unit count = " << utf8.size() << "\n";

    std::puts("String literals have static storage duration and their elements are not modifiable.");
}

int main()
{
    show_bytes_of_object();
    pointer_sizes_and_alignment();
    address_and_addressof();
    constness_on_pointer_vs_pointee();
    pointer_arithmetic_rules();
    span_and_to_address();
    void_pointer_facts();
    nullptr_and_nullptr_t();
    function_pointer_vs_object_pointer();
    strict_aliasing_escape_hatches();
    explicit_lifetime_management();
#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
    start_lifetime_as_demo();
#endif
    new_delete_and_zero_length_arrays();
    pointer_comparisons();
    pointer_integer_roundtrip();
    pointer_to_member_obscurity();
#if defined(__cpp_lib_out_ptr) && __cpp_lib_out_ptr >= 202106L
    out_ptr_demo();
#endif
    observer_vs_owner();
    show_ptrs_refs();
    show_string_literals();
    return 0;
}

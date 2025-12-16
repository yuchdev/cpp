// ReSharper disable All
#include <iostream>
#include <type_traits>
#include <utility>
#include <cassert>
#include <vector>

/// Fundamental C++ type: void
/// void has an empty set of values; objects of type void are disallowed.
/// https://en.cppreference.com/w/cpp/language/types
///
/// Templates:
/// - `void` is commonly used as a "no meaningful value" marker in generic code.
/// - SFINAE/detection idioms often use `void` (via `void_t`) to test if expressions/types exist.
void void_type()
{
    // No objects of type void:
    // void v;       // ill-formed
    // sizeof(void); // ill-formed

    int n = 123;
    int* p_int = &n;

    // Object pointer -> void* conversion is implicit and preserves the address value
    void* p_void = p_int;

    // Reverse conversion requires an explicit cast
    int* p_int2 = static_cast<int*>(p_void);
    assert(p_int == p_int2);

    std::cout << "*p_int2 = " << *p_int2 << '\n';

    // Standard C++ forbids pointer arithmetic on void*
    // ++p_void; // ill-formed (GNU extension allows this; non-portable)

    // Function pointers are NOT guaranteed convertible to void*
    // void (*fp)() = []{};
    // void* bad = reinterpret_cast<void*>(fp); // non-portable, not guaranteed by standard
}

// -----------------------------------------------------------------------------
// Templates + void: specializations and SFINAE/detection (C++11..C++17)
// -----------------------------------------------------------------------------

// 1) Classic "void as a tag" specialization
template <typename T>
struct storage_category
{
    static constexpr const char* name = "value-producing type";
};

template <>
struct storage_category<void>
{
    static constexpr const char* name = "void (no value)";
};

// 2) A simple `void_t` (C++17 has std::void_t; we define our own to be explicit)
template <typename...>
using void_t = void;

// 3) Detection idiom: detect if T has a member type `value_type`
template <typename T, typename = void>
struct has_value_type : std::false_type {};

template <typename T>
struct has_value_type<T, void_t<typename T::value_type>> : std::true_type {};

// 4) Detection idiom: detect if an expression `std::declval<T&>().size()` is valid
template <typename T, typename = void>
struct has_size_method : std::false_type {};

template <typename T>
struct has_size_method<T, void_t<decltype(std::declval<T&>().size())>> : std::true_type {};

static void void_in_templates()
{
    std::puts("\n== void in templates ==");

    std::cout << "storage_category<int>  : " << storage_category<int>::name << '\n';
    std::cout << "storage_category<void> : " << storage_category<void>::name << '\n';

    std::cout << "has_value_type<int>                 = " << has_value_type<int>::value << '\n';
    std::cout << "has_value_type<std::vector<int>>    = " << has_value_type<std::vector<int>>::value << '\n';

    std::cout << "has_size_method<int>                = " << has_size_method<int>::value << '\n';
    std::cout << "has_size_method<std::vector<int>>   = " << has_size_method<std::vector<int>>::value << '\n';

    std::puts("Fact: SFINAE works by substituting template arguments; if a substitution fails,");
    std::puts("it removes that candidate (instead of hard error). Using void_t is a convenient pattern.");
}

int main(int argc, char* argv[])
{
    void_type();
    void_in_templates();
    return 0;
}

#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

// arrays_decay_and_overloads.cpp
// Focus: decay rules, sizeof pitfalls, overload resolution, and parameter "array syntax".

namespace cpp {

// Arrays decay to pointers in most expressions, but NOT in:
// 1) sizeof(arr)
// 2) &arr (address-of) -> pointer to array
// 3) decltype(arr) when arr is an id-expression
// 4) initialization from string literal into char[] (special rules)
// 5) when bound to reference to array

template <typename T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept { return N; }

template <typename T>
constexpr bool is_pointer_v = std::is_pointer<T>::value;

static void decay_basics()
{
    int a[5] = {1,2,3,4,5};

    // sizeof sees the full array object, no decay:
    static_assert(sizeof(a) == sizeof(int) * 5, "sizeof(array) is full storage");

    // Here array decays:
    int* p = a;
    assert(p[0] == 1);

    // &a is pointer-to-array, different type:
    int (*pa)[5] = &a;
    static_assert(std::is_same<decltype(pa), int (*)[5]>::value, "pointer to array type");

    // They point to the same address numerically, but pointer arithmetic differs:
    assert(reinterpret_cast<void*>(p) == reinterpret_cast<void*>(pa));

    // +1 differs:
    // p + 1 moves by one int
    // pa + 1 moves by one *whole array* (5 ints)
    assert(reinterpret_cast<char*>(p + 1) == reinterpret_cast<char*>(p) + sizeof(int));
    assert(reinterpret_cast<char*>(pa + 1) == reinterpret_cast<char*>(pa) + sizeof(a));
}

// Overloads to show what you actually passed.
static void takes_ptr(const int*) { std::cout << "takes_ptr(const int*)\n"; }

template <std::size_t N>
static void takes_arr_ref(const int (&)[N]) { std::cout << "takes_arr_ref(const int(&)[" << N << "])\n"; }

static void overload_resolution()
{
    int a[3] = {1,2,3};

    // a in expression context can match both:
    // - decays to const int* for takes_ptr
    // - binds directly to reference for takes_arr_ref (better match)
    takes_arr_ref(a); // chosen explicitly
    takes_ptr(a);     // works via decay

    // In function parameters, "int x[]" is exactly "int* x".
    auto param_is_pointer = [](int x[]) {
        (void)x;
        static_assert(std::is_same<decltype(x), int*>::value, "function param array -> pointer");
    };
    param_is_pointer(a);
}

// Common pitfall: expecting sizeof in function to give full array size.
static void sizeof_pitfall(int a_param[]) // actually int*
{
    // sizeof(a_param) is sizeof(int*), not array size
    std::cout << "sizeof(a_param) = " << sizeof(a_param) << " (pointer size)\n";
}

static void show_sizeof_pitfall()
{
    int a[10] = {};
    std::cout << "sizeof(a) = " << sizeof(a) << " (array storage)\n";
    sizeof_pitfall(a);
}

} // namespace cpp

int main()
{
    cpp::decay_basics();
    cpp::overload_resolution();
    cpp::show_sizeof_pitfall();
    std::cout << "arrays_decay_and_overloads.cpp: OK\n";
    return 0;
}

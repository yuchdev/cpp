#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

// arrays_initialization_and_literals.cpp
// Focus: initialization, aggregate rules, literal array types, and C++20 char8_t.

namespace cpp {

static void init_rules()
{
    int zeroed[5] = {};
    int partial[5] = {1, 2};
    int uninitialized[5];
    (void)uninitialized; // do not read before initialization

    assert(zeroed[4] == 0);
    assert(partial[0] == 1);
    assert(partial[2] == 0);

    // An omitted bound is deduced from the initializer.
    int deduced[] = {1, 2, 3};
    static_assert(std::extent_v<decltype(deduced)> == 3);

    // Brace initialization rejects narrowing.
    // int narrowed[1] = {3.14}; // ill-formed
}

static void string_literal_facts()
{
    // A string literal is an lvalue array whose bound includes the terminator.
    static_assert(std::is_same_v<
        decltype("ABC"),
        const char (&)[4]>);

    using LiteralArray =
        std::remove_reference_t<decltype("ABC")>;

    static_assert(std::is_same_v<
        LiteralArray,
        const char[4]>);

    const char* p = "Hello";
    std::cout << "sizeof(p) = " << sizeof(p) << " (pointer)\n";

    char local[] = "Hi";
    static_assert(sizeof(local) == 3);
    assert(local[2] == '\0');

    // The local array is a writable copy; the literal itself is not writable.
    local[0] = 'B';
    assert(local[0] == 'B');
}

static void utf8_literal_cpp20()
{
#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    // Since C++20, a u8 literal has element type const char8_t rather than
    // const char. Code written for C++17 may therefore need an explicit UTF-8
    // code-unit interface instead of accepting const char*.
    static_assert(std::is_same_v<
        decltype(u8"ABC"),
        const char8_t (&)[4]>);

    constexpr char8_t text[] = u8"ABC";
    static_assert(std::extent_v<decltype(text)> == 4);
    static_assert(text[3] == u8'\0');
#endif
}

static void multidimensional_initialization()
{
    int matrix[2][3] = {
        {1, 2, 3},
        {4}
    };

    assert(matrix[0][2] == 3);
    assert(matrix[1][0] == 4);

    // Missing elements are value-initialized recursively.
    assert(matrix[1][1] == 0);
    assert(matrix[1][2] == 0);
}

} // namespace cpp

int main()
{
    cpp::init_rules();
    cpp::string_literal_facts();
    cpp::utf8_literal_cpp20();
    cpp::multidimensional_initialization();

    std::cout << "arrays_initialization_and_literals.cpp: OK\n";
    return 0;
}

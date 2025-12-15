#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

// arrays_initialization_and_literals.cpp
// Focus: initialization rules, zero/value init, brace-init narrowing (C++11+),
// string literal types, '\0' terminator, and common "string literal is array" facts.

namespace cpp {

static void init_rules()
{
    int a[5] = {};           // value-initialized => all zero
    int b[5] = {1,2};        // remaining elements zero
    int c[5];                // uninitialized (indeterminate) for automatic storage
    (void)c;                 // do not read without init

    assert(a[4] == 0);
    assert(b[2] == 0);

    // C++11+: brace-init prevents narrowing
    // int d[1] = { 3.14 }; // ERROR (narrowing)
}

static void string_literal_facts()
{
    // "ABC" is an lvalue of type const char[4] (includes '\0').
    // decltype(lvalue_expression) produces T&, so we get const char(&)[4]
    static_assert(std::is_same<decltype("ABC"), const char(&)[4]>::value, "");

    // If you want to assert the underlying array type:
    static_assert(std::is_same<std::remove_reference_t<decltype("ABC")>, const char[4]>::value, "");

    const char* p = "Hello";
    std::cout << "sizeof(p) = " << sizeof(p) << " (pointer)\n";

    char s1[] = "Hi"; // size 3: 'H','i','\0'
    static_assert(sizeof(s1) == 3, "");
    assert(s1[2] == '\0');
}


} // namespace cpp

int main()
{
    cpp::init_rules();
    cpp::string_literal_facts();
    std::cout << "arrays_initialization_and_literals.cpp: OK\n";
    return 0;
}

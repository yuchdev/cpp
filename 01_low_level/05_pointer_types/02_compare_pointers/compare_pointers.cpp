#include <cassert>
#include <compare>
#include <functional>
#include <iostream>
#include <ranges>

static void comparison_inside_one_array()
{
    std::cout << "\n== Pointer ordering inside one array ==\n";

    int values[] = {10, 20, 30, 40};

    int* p0 = &values[0];
    int* p2 = &values[2];
    int* end = values + 4;

    // Built-in relational comparison is meaningful for elements of the same array
    // (and its one-past pointer): higher subscript => greater pointer.
    assert(p0 < p2);
    assert(p2 < end);
    assert(end - p0 == 4);

    std::cout << "p0 < p2 < end follows array element order.\n";
}

static void comparison_of_unrelated_objects()
{
    std::cout << "\n== Unrelated object pointers ==\n";

    int a = 10;
    int b = 20;
    int* pa = &a;
    int* pb = &b;

    // Equality is meaningful: these pointers either represent the same pointer
    // value or they do not.
    std::cout << std::boolalpha << "pa == pb: " << (pa == pb) << '\n';

    // A built-in relational comparison such as pa < pb is syntactically valid,
    // but for unrelated complete objects the language does not give it the
    // portable address-ordering meaning programmers often assume. The result
    // may be unspecified.
    //
    // bool raw_order = pa < pb; // legal expression, poor portable ordering policy

    // std::less provides the library's strict total order over pointers, even
    // where built-in relational operators do not provide a useful total order.
    std::cout << "std::less{}(pa, pb): "
              << std::less<>{}(pa, pb) << '\n';

    // C++20 ranges comparison objects provide the corresponding strict total
    // order for pointers.
    std::cout << "std::ranges::less{}(pa, pb): "
              << std::ranges::less{}(pa, pb) << '\n';
}

static void three_way_comparison()
{
    std::cout << "\n== C++20 three-way comparison ==\n";

    int values[] = {1, 2, 3};
    int* first = &values[0];
    int* last = &values[2];

    // Pointer <=> follows the built-in pointer composite-type and ordering rules.
    // It does not turn unrelated pointers into a portable numeric-address order.
    const auto order = first <=> last;

    assert(order < 0);
    std::cout << "first <=> last is less for elements of the same array.\n";
}

int main()
{
    comparison_inside_one_array();
    comparison_of_unrelated_objects();
    three_way_comparison();
}

#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

// arrays_new_delete_and_typedef_pitfalls.cpp
// Focus: new[]/delete[] correctness, hidden array types via typedef/using,
// and "static type must match delete form".

namespace cpp {

static void new_delete_basics()
{
    int* p = new int[5]{};    // value-init => zeros
    assert(p[4] == 0);

    delete[] p;               // must use delete[] for arrays
}

// Typedef hides array-ness => easy to accidentally delete wrong or store wrong type.
static void typedef_array_pitfall()
{
    typedef int Week[7];

    // This allocates an array of 7 ints and returns int* (decayed)
    int* p = new Week;

    // Correct:
    delete[] p;

    // Another classic issue (do not do):
    // Base* b = new Derived[2];
    // delete[] b; // UB because delete[] expects the same dynamic/static array type layout.
    //
    // (This is especially nasty with virtual destructors or different sizes.)
}

} // namespace cpp

int main()
{
    cpp::new_delete_basics();
    cpp::typedef_array_pitfall();
    std::cout << "arrays_new_delete_and_typedef_pitfalls.cpp: OK\n";
    return 0;
}

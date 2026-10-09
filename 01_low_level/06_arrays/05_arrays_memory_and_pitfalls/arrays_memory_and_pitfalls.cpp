#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>

namespace cpp {

static void new_delete_basics()
{
    int* p = new int[5]{};
    assert(p[4] == 0);

    delete[] p;
}

static void typedef_array_pitfall()
{
    using Week = int[7];

    // The new-expression allocates one Week object, which is an array of seven
    // ints. The result undergoes the array-new rules and is observed as int*.
    int* p = new Week;
    delete[] p;

    // Hiding the array type behind an alias makes ownership/deallocation rules
    // harder to read. Prefer an owning vocabulary type in new code.
}

static void unique_ptr_array()
{
    auto values = std::make_unique<int[]>(5);

    values[0] = 10;
    values[4] = 50;

    assert(values[0] == 10);
    assert(values[4] == 50);

    // unique_ptr<T[]> selects delete[] automatically and exposes operator[].
}

static void cxx20_array_factories()
{
#if defined(__cpp_lib_shared_ptr_arrays) && __cpp_lib_shared_ptr_arrays >= 201707L
    auto shared = std::make_shared<int[]>(4);
    shared[0] = 7;
    shared[3] = 11;

    assert(shared[0] == 7);
    assert(shared[3] == 11);
#endif

#if defined(__cpp_lib_smart_ptr_for_overwrite) && __cpp_lib_smart_ptr_for_overwrite >= 202002L
    // for_overwrite avoids value-initializing scalar elements when every element
    // will immediately be overwritten. Reading before writing would be invalid.
    auto scratch = std::make_unique_for_overwrite<int[]>(4);

    for (std::size_t i = 0; i < 4; ++i)
        scratch[i] = static_cast<int>(i * i);

    assert(scratch[3] == 9);
#endif
}

static void polymorphic_array_warning()
{
    struct Base
    {
        virtual ~Base() = default;
    };

    struct Derived : Base
    {
        int extra = 42;
    };

    // Base* bad = new Derived[2];
    // delete[] bad; // undefined behavior
    //
    // Array deletion must recover and destroy the array using the allocation's
    // element type/layout. A virtual destructor does not make arrays covariant.
}

} // namespace cpp

int main()
{
    cpp::new_delete_basics();
    cpp::typedef_array_pitfall();
    cpp::unique_ptr_array();
    cpp::cxx20_array_factories();
    cpp::polymorphic_array_warning();

    std::cout << "arrays_memory_and_pitfalls.cpp: OK\n";
    return 0;
}

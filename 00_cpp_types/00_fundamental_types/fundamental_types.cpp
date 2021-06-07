#include <iostream>
#include <bitwise.h>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types

// void - type with an empty set of values
// There are no arrays of void, nor references to void. However, pointers to void and functions returning type void.
// Type void have use in template metaprogramming
void boolean_type()
{
    // The value of sizeof(bool) is implementation defined and might differ from 1
    // Bitwise representation is implementation-defined
    // Naming of bool often express its boolean nature, starting from "is"
    bool is_root = true;
    std::cout 
        << "sizeof(bool) = " << sizeof(is_root)
        << "; "
        << "bitwise(bool) = " << bitwise(is_root) << '\n';

    // Operator sizeof() yelds size in bytes of the object representation of type or expression
    // sizeof() cannot be used with function types, incomplete types, or bit-field glvalues
}

// TODO: nullptr_t, void

// TODO: move to appropriate lesson
void initialization_order()
{
    // example of UB due to bad initialzation
    int i = i * 0;

    // no defined ordef of initilization within one expression
}


// 1.
template<typename Key, typename Val>
void print_map(const std::map<Key, Val>& m)
{
    /*
    1.This function does not have 3rd map template param, 
    if it's provided, funcion dos not work\
    Solution is to pass additional args
    1.1
    void print_map(const std::map<
        Key, Val, 
        class Compare = std::less<Key>, 
        class Allocator = std::allocator<std::pair<const Key, T>
    >& m)
    1.2
    void print_map(const std::map<Key, Val, AdditionalArgs...>& m)
    1.3
    Use 1.2 and concept
    */

    // 2.Const pair is not the same as pair of const!
    // The example will produce a lot or redundant copies
    for (const std::pair<Key, Val>& p: m){
        std::cout << p.first << '=' << p.second;
    }

    std::cout << typeid(m);
    // TODO: print pair type
    //std::cout << typeid(std::map<Key, Val>);

    // Better solution is to use auto
    // or C++17 structure binding
    for (const auto& [key, val]: m){
        std::cout << key << ' ' << val;
    }
    // https://google.github.io/styleguide/cppguide.html#auto
    // https://channel9.msdn.com/posts/Scott-Meyers-Andrei-Alexandrescu-and-Herb-Sutter-C-and-Beyond


}

// 2. Use C++20 auto!
void print_pair(const auto& p)
{
    // Check out other way to iterate 
    std::cout << p.first << ' ' << p.second;

    // first and second are considered bad design in general
    // If you work with other pair than std::pair, it may not work
    // Right implementation
    std::cout << std::get<0>(p) << ' ' << std::get<1>(p);

    // or use concept
    // requires std::tuple_size<Pair>::value == 2;
}

// 3.Use C++20 auto!
void zero_init_all(auto& container)
{
    for (auto& val: container){
        val = {};
    }

    // second auto should be auto&& for perfect forwarding
    // It will not work for vector<bool>, where you need by-val semantic
}

// 4.
struct Pet{};

struct Dog : public Pet{};

template <typename T>
struct PrintMe
{
    static void print()
    {
        std::cout << "PrintMe::print()\n";
    }
};

template <>
struct PrintMe<Pet>
{
    static void print()
    {
        std::cout << "PrintMe<Pet>::print()\n";
    }
};

void test_pet()
{
    // Would print "PrintMe::print()\n"
    // Specialization knows nothing about inheritance
    PrintMe<Dog>::print();
}

// 5.
class HoldRaii
{
    static size_t object_count = 0;
public:
    HoldRaii() = default;
    HoldRaii(const HoldRaii& other)
    {
        // increment a static counter
        object_count++;
    }
    ~HoldRaii()
    {
        object_count--;
    }
};

int main()
{
    // TODO: insert any function call
    return 0;
}

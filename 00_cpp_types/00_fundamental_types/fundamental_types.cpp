#include <iostream>
#include <vector>
#include <string>
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
// TODO: move to appropriate lesson
void initialization_order()
{
    // example of UB due to bad initialization
    int i = i * 0;

    // no defined order of initialization within one expression
}
#pragma clang diagnostic pop


// 1.
template<typename Key, typename Val>
void print_map(const std::map<Key, Val>& m)
{
    /*
    1.This function does not have 3rd map template param, 
    if it's provided, function dos not work
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

// 5. What problem here (dangling ref, inefficiency, potential leak, compile error)
class HoldRaii
{
    static size_t object_count;
public:
    HoldRaii() = default;
    HoldRaii(const HoldRaii& other)
    {
        // increment a static counter
        ++object_count;
    }
    ~HoldRaii()
    {
        --object_count;
    }

    // No move operation declared, which means copy-operation will be called
    // e.g. push_back() operations
    // See Rule of Zero
    // https://arne-mertz.de/2015/02/the-rule-of-zero-revisited-the-rule-of-all-or-nothing/

    // Let's implement move c-tor
    // What's wrong with it?
    HoldRaii(HoldRaii&& other)
    {
        // increment a static counter
        // (object left in empty state, but not destructed yet)
        ++object_count;
    }
    // Answer: miss noexcept
    // Other noexcept c-tor won't accept it
    // E.g. vector c-tor not move, it copies
    // see:
    // https://en.cppreference.com/w/cpp/utility/move_if_noexcept
    // https://stackoverflow.com/questions/28627348/noexcept-and-copy-move-constructors
    // TODO: implement example
};

// static
size_t HoldRaii::object_count = 0;

// 8. Rule if you want to copy, pass by value
std::vector<std::string> strings;

void save_to_list(std::string s)
{
    strings.push_back(std::move(s));
    // Herb Sutter: reasons to pass std::vector and std::string by const & are largely gone
    // So as rule "if you want to copy, pass by value"
    // It could be used while we don't save string in a caller, just use const&
    // If we use and copy, we should use string&& and std::move(s)
    // Use std::forward for perfect forwarding
    // See Howard_Hinnant_Accu_2014.pdf
}

// Right way is overload
void save_to_list(std::string&& s)
{
    strings.push_back(s);
}

void save_to_list(const std::string& s)
{
    strings.push_back(s);
}

template <typename T> requires
std::convertible_to<T, std::string>
void save_to_list(const std::string& s)
{
    strings.push_back(std::forward<T>(s));
}

// 9. C-tor should be explicit
// Person p = "John";

// 10. Smart ptr class
class smart_ptr
{
public:
    smart_ptr(T* p) : ptr(p) {}
    ~smart_ptr() { delete ptr; }

    // Answer: code break logical const
    // We want to use <const T> rather than const smart_ptr<T>
    // const iterators are also potentially part of this problem!
    T& get() const { return ptr; }
    void set(T t) { *ptr = t; }

private:
    T* ptr;
};

// 11.
// Facebook pre-C++11 example
// Usage example:
// const std::string s = get_or_default(m, s1, s2);
template <class Map, typename Key>
const typename Map::mapped_type& get_or_default(
        const Map& map,
        const Key& key,
        const typename Map::mapped_type& defaultVal)
{
    auto pos = map.find(key);
    return (pos != map.end() ? pos->second : defailtVal);
}

// Answer: possible dangling reference on return type
// rvalue defaultVal would be dangling
// Usage example:
// const std::string& s = get_or_default(m, "apple", "orange");
// Solution 1: return by value 'const typename Map::mapped_type'
// Solution 2: don't allow rvalue 'typename Map::mapped_type&& defaultVal'
// and delete its implementation
// Solution 3: overload for rvalue 'typename Map::mapped_type&& defaultVal'
// and return by value
//



int main()
{
    boolean_type();
    return 0;
}

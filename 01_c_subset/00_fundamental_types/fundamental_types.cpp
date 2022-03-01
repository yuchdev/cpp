#include <iostream>
#include <vector>
#include <type_traits>
#include <bitwise.h>
#include <map>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types

void boolean_type()
{
    // The value of sizeof(bool) is implementation defined and might differ from 1
    // Bitwise representation is implementation-defined
    // Naming of bool variables and function often express its boolean nature, starting from "is"
    bool is_root = true;
    std::cout 
        << "sizeof(bool) = " << sizeof(is_root)
        << "; "
        << "bitwise(bool) = " << bitwise(is_root) << '\n';

    // Operator sizeof() yields size in bytes of the object representation of type or expression
    // sizeof() cannot be used with function types, incomplete types, or bit-field l-values
}

// Prior to C++11 standard null pointer was served by C-macro NULL
// C++11 standard null pointer is defined as nullptr
// The problem with NULL is that when we pass integer 0, we can't say if its a pointer or integer
// It potentially could be a problem during function overloading over types void* and int,
// and template metaprogramming
void accept_null(int)
{
    std::cout << "accept_null(int)\n";
}

void accept_null(void*)
{
    std::cout << "accept_null(void*)\n";
}

void accept_null(int*)
{
    std::cout << "accept_intptr(int*)\n";
}

void accept_null(std::nullptr_t)
{
    std::cout << "accept_null(std::nullptr_t)\n";
}

void nullptr_type()
{
    // Trying to call with NULL
    // error: call to 'accept_null' is ambiguous
    // accept_null(NULL);
    // accept_null(0);

    accept_null(0);

    // Since C++11, we have a null pointer literal, nullptr, of std::nullptr_t type
    // If two or more overloads accept different pointer types (void* and int* in our case),
    // an overload for std::nullptr_t is necessary to accept a null pointer argument.
    // Otherwise, we got an error: call to 'accept_null' is ambiguous
    int* ptr1 = nullptr;
    void* ptr2 = nullptr;
    accept_null(nullptr);
    accept_null(ptr1);
    accept_null(ptr2);

    // nullptr is a constant expression, equal to size of void*
    assert(sizeof(std::nullptr_t) == sizeof(void *));

    // Since C++14, we have a template struct is_null_pointer()
    std::cout
        << "std::is_null_pointer<int*>::value"
        << std::is_null_pointer<int*>::value << '\n';
}

// void - type with an empty set of values
// It is an incomplete type that cannot be completed
// (consequently, objects of type void are disallowed)
// There are no arrays of void, nor references to void.
// However, pointers to void and functions returning type void.
// Type void have use in template metaprogramming
// to show that a type is not known at compile time
void void_type()
{
    // Pointer of any type can be implicitly converted to pointer to void, without changing its value
    // The reverse conversion requires static_cast
    int n = 0;
    int* p_int = &n;
    void* p_void = p_int;
    int* p_int2 = static_cast<int*>(p_void);
}

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

    std::cout << typeid(m).name() << '\n';
    // TODO: print pair type
    //std::cout << typeid(std::map<Key, Val>);

    // Better solution is to use auto with C++17 structure binding
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

// TODO: sort out std::convertible_to usage
#if __cplusplus >= 20 and false

template <typename T> requires
std::convertible_to<T, std::string>
void save_to_list(const std::string& s)
{
    strings.push_back(std::forward<T>(s));
}

#endif

// 9. C-tor should be explicit
// Person p = "John";

// 10. Smart ptr class
template <typename T>
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
    return (pos != map.end() ? pos->second : defaultVal);
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
// Note: ASAN sanitizer finds the problem


// TODO: watch
[[maybe_unused]]
void unique_dangling_ref()
{
    // warning: object backing the pointer will be destroyed at the end of the full-expression
    auto& ref = *std::make_unique<int>(42);
    std::cout << ref << '\n';
}

// 12.
struct Person
{
    std::string name_;
    Person(const std::string& n) : name_(n) {}
    const std::string& name() { return name_; }
};

// What's wrong with iterations?
void show_iterative()
{
    // [a]
    for(char c: std::string{"hello"}) {
        // do something with c
    }
    // [b]
    for(const char& c: std::string{"hello"}) {
        // do something with c
    }
    // [c]
    for (char c: Person{"John"}.name()) {
        // do something with c
    }
}

// Answer: [c] using dangling reference
// Temporary object's Person{"John"} lifetime is not extended for the loop scope

// 13.

class Base {};

class Derived;

void foo(const Base& d) {}

void foo1(const Derived& d)
{
    // This is runtime bad casting and UB
    // However, the code is compiling
    // Never use C-style casting!
    // Use static_cast or dynamic_cast
    foo((const Base&) d);
}

// 14.
void add_distance(double dist)
{

}

void show_type_aliases()
{
    using meters = double;
    meters distance = 7.5;
    add_distance(distance);
}

// Answer: design is wrong, no type enforcement
// Distance could be in other units
// Use strong types and type literals
// Chrono is a great example for type literals:
// Examples of good measurement naming types
// https://www.boost.org/doc/libs/1_75_0/doc/html/boost_units.html
// https://github.com/joboccara/NamedType

// Wrong Type is Actually Crashing
// https://hownot2code.com/2016/09/02/a-space-error-370-million-for-an-integer-overflow/

// ...don’t just wrap it with a struct
// This is NOT a Strong Type
// struct Meters { double m; }
// Rectangle r(Meters(10), Meters(12));
// but then this would also work:
// Rectangle r({10}, {12});
// Note also that this is against the encapsulation rule
// Such structs turn to grow into fully functioning classes with public members

// 15.

void show_int_ub()
{
    int x = INT_MAX;
    int y = x + 1;
    if (x < y) {
        std::cout << "x is smaller";
    } else {
        std::cout << "y is smaller or equal";
    }
}
// Answer: this is UB, integer overflow
// Check out UB analyzer
// https://tsnippet.trust-in-soft.com/#

// Examples of int/uint UB
// https://stackoverflow.com/questions/18195715/why-is-unsigned-integer-overflow-defined-behavior-but-signed-integer-overflow-is
// https://stackoverflow.com/questions/7488837/why-is-int-rather-than-unsigned-int-used-for-c-and-c-for-loops
// https://stackoverflow.com/questions/199333/how-do-i-detect-unsigned-integer-multiply-overflow
// https://stackoverflow.com/questions/7488837/why-is-int-rather-than-unsigned-int-used-for-c-and-c-for-loops
// https://stackoverflow.com/questions/22587451/c-c-use-of-int-or-unsigned-int
// TODO: watch
// https://www.youtube.com/watch?v=93Cjg42bGEw&t=91s
// https://www.youtube.com/watch?v=sBtAGxBh-XI
// https://www.boost.org/doc/libs/1_70_0/libs/numeric/conversion/doc/html/index.html
// https://www.boost.org/doc/libs/develop/libs/safe_numerics/doc/html/index.html
// https://us-cert.cisa.gov/bsi/articles/knowledge/coding-practices/safe-integer-operations
// http://blog.llvm.org/2011/05/what-every-c-programmer-should-know.html

int main()
{
    boolean_type();
    return 0;
}

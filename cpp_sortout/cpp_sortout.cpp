#include <utilities/elapsed.h>

#include <iostream>
#include <cstdint>
#include <vector>
#include <map>

/// Radix sort of integer vector
void radix_sort(const std::vector<int>& v)
{
    std::vector<int> result;
    std::vector<int> buckets[10];

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < v.size(); ++j)
        {
            int bucket_index = (v[j] / static_cast<int>(std::pow(10, i))) % 10;
            buckets[bucket_index].push_back(v[j]);
        }

        for (int j = 0; j < 10; ++j)
        {
            result.insert(result.end(), buckets[j].begin(), buckets[j].end());
            buckets[j].clear();
        }
    }
}

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

// This is a question from one interview. The question is 
// "What value boo(-1) function call return?"
// Got it?
// Answer: The else actually matches with the second if (C++ Standard 6.4.1/1)
// Tabs arranged to make the answer not that obvious
// (especially for Python programmers)
// Clang even shows warning on this situation, which gives this compiler additional points
int boo(int x){
    if (x > 0)
        if (x > 100)
            return 1;
    else
        return 2;
    return 0;
}

void naming_conventions()
{
    // Give variables with large scope long and clear names
    static int test_counter = a;

    // It's okay to give short names to variables with small scope
    for (int i = 0; i < test_counter; i++){
        ::g_global += test_counter;
        cout << ::g_global << endl;
    }

    // access global variable
    ::g_global = 0;

    // global variable shadowing
    int g_global = 0;

    // There's no way to access 
    // a shadowed local variable
}

// Why rand() is a very slow function. It calls Enter/LeavCriticalSection 
// and locks data bus, which may kill your application performance, 
// especially if the architecture relying on multiple CPU.
// Here's some example
void show_rand()
{
    volatile int x;
    int n = 10000000;
    MeasureTime m;

    std::cout << "Start random number generation with rand()\n";
    for (int i = 0; i < n; ++i) {
        x = rand();
    } 
    std::cout << "Random number generation with rand() took " <<  m.elapsed_mcsec() << '\n';
}


// Alignment
struct S { char c; int i; };

// Wrong compare
bool is_equal(S a, S b) { return 0 == memcmp(&a, &b, sizeof(S)); }

int main()
{
    return 0;
}



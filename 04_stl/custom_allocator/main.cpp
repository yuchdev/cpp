#include <iostream>
#include <list>
#include <set>
#include <vector>
#include <cstdlib>
#include <iterator>
#include <memory>
#include "custom_allocator.h"


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

void show_custom_allocator()
{
    // objects with different allocators should not be mixed in general
    // For example, list::splice with different allocators will work
    // not by list merging as usual, but by element-by-element insertion
    typedef std::vector<int, std::custom_allocator<int> > custom_vector;
    custom_vector v;
    v.push_back(1);
    v.push_back(2);
    v.pop_back();

    // STL includes functions for low-level work with 
    // user containers and algorithms
    // they offer a strong exception guarantee

    // initialization of RAM by value
    int arr[10] = {};
    uninitialized_fill(arr, arr + sizeof(arr) / sizeof(int), 1);

    // initialize n elements by a value
    uninitialized_fill_n(arr, 5, 2);

    // copying the interval into RAM
    int arr2[10] = {};
    uninitialized_copy(arr, arr + sizeof(arr) / sizeof(int), arr2);

    // there is `raw_storage_iterator` iterator for iterating uninitialized memory
    // for short-term memory usage:
    pair<int*, ptrdiff_t>p = get_temporary_buffer<int>(10);
    // it returns a pair, pointer to the beginning and the size

    // deallocation has no check for 0
    if (p.first != 0)
        return_temporary_buffer(p.first);
}

void show_c_memory()
{

    // C functions to manage "raw" memory

    // allocate 100 bytes
    void* m = malloc(100);

    // allocates and zero-initialize memory for 100 int
    void* c = calloc(100, sizeof(int));

    // resize to 200
    void* m1 = realloc(m, 200);
    if (m1 != NULL) {
        m = m1;
    }
    else {
        free(m);
        puts("Error (re)allocating memory");
    }
    free(m);
    free(c);
    // in addition 
    // memcpy() - copy without overlapping
    // memmove() - moving with overlapping
    // memchr() - bit search by value
    // memcmp(), memset()
}

int main()
{
    return 0;
}

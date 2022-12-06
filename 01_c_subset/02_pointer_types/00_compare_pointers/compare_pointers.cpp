#include <iostream>

// How to compare pointers
void compare_pointers_1()
{
    // Detailed explanation: there are two ways to compare pointers in C++:
    // * equality operators, which is != and ==
    // * more-less operators, < <= > >=

    int a { 10 }, b { 20 };
    int* pa = &a;
    int* pb = &b;
    if (a < b) {
        std::cout << "Really, what do you expect here?\n";
    }
    // technically may be correct, but according to the Standard it's wrong
}

// C++17 Standard 8.9 says:
// If two pointers point to different elements of the same array, or to subobjects thereof, 
// the pointer to the element with the higher subscript compares greater.

// It other words, first operators allowed in any case
// Second make sense ONLY if two pointers belong to the same address space
// Otherwise we meet Unspecified Behavior (Not Undefined!)

// Right way to compare pointers is std::less() function, which fives guarantees for correctness
// https://en.cppreference.com/w/cpp/utility/functional/less
// We can even create template function for pointers only
template<typename T>
bool less_ptr(T* a, T* b)
{
    return std::less<T*>()(a, b);
}

void compare_pointers_2()
{
    int a { 10 }, b { 20 };
    int* pa = &a;
    int* pb = &b;
    if (less_ptr(pa, pb)) {
        std::cout << "Now I feel safe comparing my pointers!\n";
    }
    // Example where this approach may be necessary, is segment memory model
    // It used to be model for 16-bit CPU architectures, where memory was a pair of 
    // [selector + offset]
    // Let's imagine, we have some embedded architecture with the same memory model
    // In this case, plain compare (pa < pb) between different segments make no sense!
}

int main()
{
    // TODO: insert any function call
    return 0;
}

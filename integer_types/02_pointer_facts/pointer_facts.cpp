#include <iostream>
#include <cmath>
#include <bitset>
#include <ctime>
#include <cstddef>

/// Returns bitset with bitwise representation of 'value'
template <typename T>
std::bitset<sizeof(T) * 8> bitwise(T value)
{
    constexpr size_t bits_size = sizeof(T) * 8;
    return std::bitset<bits_size>(value);
}

// TODO: alignment
// https://stackoverflow.com/questions/5435841/memory-alignment-in-c-structs
// https://en.cppreference.com/w/c/language/object

// Important information
// Agreements about types size in C++:
// 1 = sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long)
// 1 <= sizeof(bool) <= sizeof(long)
// sizeof(char) <= sizeof(wchar_t) <= sizeof(long)
// sizeof(float) <= sizeof(double) <= sizeof(long double)
// sizeof(N) <= sizeof(signed N) <= sizeof(unsigned N)

// TODO: make 1

// 2. What is the size of enum
// TODO: find in Standard

enum enum1{ dark, light };		// 0:1
enum enum2{ a = 3, b = 9 };		// 0:15
enum enum3{ c = 3, d = 900 };	// 0:1024
enum enum4{ e = 3, f = 5000000000 };	// out of range 0:2^32

void enum_size()
{
    enum1 e1 = enum1(0);
    enum1 e2 = enum1(1);

    // UB
    enum1 e3 = enum1(20); 

    // enum size is platform-dependent
    size_t sz = sizeof(enum1);
    sz = sizeof(enum2);
    sz = sizeof(enum3);

    // out if range
    enum4 e4 = enum4::f;
    sz = sizeof(e4);
    long long l = e4;
}

// 3. How to compare pointers
void compare_pointers_1()
{
    int a{ 10 }, b{ 20 };
    int* pa = &a;
    int* pb = &b;
    if (a < b) {
        std::cout << "Really, what do you expect here?\n";
    }
    // technically may be correct, but according to the Standard it's wrong
}

// Detailed explanation: there are two ways to compare pointers in C++:
// * equality operators, which is != and ==
// * more-less operators, < <= > >=

// TODO: Standard chapter
// First operators allowed in any case
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
    int a{ 10 }, b{ 20 };
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

// 4. Facts about pointers
void pointers_facts(){

    // x86 of today use little - endian storage for all types of data(integer, floating point, BCD)


    // Every object has an address
    // Guaranteed that there are no objects with NULL address
    int a{};
    int* ptr_a = &a;

    // Standard output prints pointers as hexadecimal
    std::cout << "ptr_a = " << ptr_a << '\n';

    // Always take notice about constant semantic of POINTER or OBJECT,
    // which pointer points on

    // pointer is constant, integer does not
    int* const ptr1 = &a;

    // Integer is constant, pointer does not
    const int* ptr2 = &a;
    // .. basically the same
    int const* ptr3 = &a;

    // Both pointer and integer are constant
    const int* const ptr4 = &a;

    // Pointer arithmetic
    int some_array[10] = {0,1,2,3,4,5,6,7,8,9};
    int* ptr_to_first = some_array;


    // 1.pointer + integer: moves forward from the pointer
    int* fifth_element = ptr_to_first + 5;

    // 2.pointer - integer: moves backward from the pointer
    int* second_element = ptr_to_first -3;

    // 3.pointer - pointer: is important and means distance between pointers
    // Pointer distance has a special type, defined in <cstddef> include file
    ptrdiff_t some_distance = fifth_element - second_element;

    // 4.++ or -- moves to the pointee size!
    ++ptr_to_first; // moves 4 bytes forward!
    ptrdiff_t other_distance = ptr_to_first - some_array;
    
    // Exception from this rule is void*
    void* vp = reinterpret_cast<void*>(some_array);
    // ++vp; there's no increment/decrement for void*


    // 4.pointer + pointer: why do we even need this?

    // A pointer to a non-array dynamic object behaves the same as a pointer to the
    // first element of an array of length one
    int* p1 = new int(1);
    int* p2 = new int(2);
    bool p1_less_p2 = less_ptr(p1, p2);
    delete p1;
    delete p2;
}

// 5. Facts about arrays
void array_facts()
{
    // A bit obsolete typedef for array
    typedef int Vector3D[10];
    Vector3D numbers = { 1, 2, 3 };

    // string literal is an array
    const char* str = "Some string";
    std::cout << "Size of the string '" << str << "' is " << sizeof(str);

    // string literal is constant by default
    char* const_string = "Other string";
    // p[0] = 'o' - can't touch this!

    // If you want to operate non-const string literal, use array instead
    char non_const_string[] = "this string is non-constant";
    non_const_string[0] = 'T';
    std::cout << non_const_string << '\n';

    // Interesting notation of arrays
    int x = 2;
    std::cout << x["ABCDEF"] << '\n';
    std::cout << 2["ABCDEF"] << '\n';

    // Is this code correct?
    // Yes! // It is correct!

    // Sincs plain ANSI C times, for array x[1] is equal  to 1[x] is equal to *(1 + x)
    // ISO / IEC 9899:TC2 : 6.5.2.1 Array subscripting
    // Thus, &*E is equivalent to E (even if E is a null pointer), and &(E1[E2]) to ((E1)+(E2))

    // Simply says, 1[array] == array[1]
    // 1 + *pointer == *pointer + 1

    // However, don't try to change it, since l-value specifies constant object
    //x["ABCDEF"] = 'Z'; error C2166
}

// 5. Array and pointer are not the same!
// Some developers think if array could be implicitly casted to pointer,
// it means they array address and array itself basically the same. Right?..
// Wrong! 
// Expressions 'array' and '&array' have different types and could not even be compared

// Let's use this static type
template <typename T, std::size_t N>
inline std::size_t arraysize(T(&arr)[N]) {
    return N;
}

void array_pointer_and_array(){

    // Step 1. Let's create array of integers and pointer to integer
    // Pointer let us assign the array address (or the first element's of array address)
    // without any problems
    int integer_array[42] = {};
    for (size_t s = 0; s < 42; ++s){
        integer_array[s] = s;
    }

    int* integer_ptr = integer_array;

    // Step 2. Now, what if we try to get array's address? Isn't it pointer to integer t00?
    // int* ii31 = &array;
    // cannot convert from 'int (*)[42]' to 'int *'
    
    // That's how looks pointer to array type
    int(*array_ptr)[42] = &integer_array;

    // Looks like the same pointer
    ptrdiff_t diff1 = reinterpret_cast<int*>(array_ptr) - integer_ptr;
    std::cout << std::hex << array_ptr << '-' << integer_ptr << '=' << std::dec << diff1 << '\n';

    // Step 3. Why not to add 1 to out array (and to the pointer to array)
    int* int_array_plus = 1 + integer_array;
    int(*ii6)[42] = 1 + &integer_array;

    ptrdiff_t diff2 = reinterpret_cast<int*>(ii6)-int_array_plus;
    std::cout << std::hex << ii6 << '-' << int_array_plus << '=' << std::dec << diff2 << '\n';
    // diff2 равен 41. It means (&array + 1) points to the LAST array item + offset 1
    // In other words, pointer to array points to the "whole array", 
    // and +1 pointer would be out of array bounds


    // Step 4. How can we use it? For example, here is funny trick how to get the array size
    // &array+1 points to the next "after array" element
    // &array[0] - points to the first element
    ptrdiff_t diff3 = (1[&integer_array] - &integer_array[0]);

    // Compiler would arrange the following operations priority
    // 1( [&array] ) - &( array[0] ) == ( 1 + &array ) - &(0 + array)
    std::cout << std::hex << 1[&integer_array] << '-' << &integer_array[0] << '=' << std::dec << diff3 << '\n';

    // 5. However, the trick is quite vague, and really not necessary,
    // since array size is included statically in every array type
    std::cout << arraysize(integer_array) << '\n';

    // 6. Pointer to multi-dimensional array
    int m_arr[10][20] = {};
    int(*m_dim_ptr)[10][20] = &m_arr;
}


// 6. Facts about references
// TODO
void show_references(){

    int ii = 10;

    // Reference must be initialized, and could not be re-initialized
    int& rr = ii;

    // When we meet function T& ref = boo(),
    // we expect that object lives long enough not to create "dead" reference
}

// 7. Unexpected type cast

// Be careful with unsigned integers, I mean, really
void comparing_unsigned()
{
    unsigned int i1 = 1;
    int j1 = -1;
    
    // i1 == 1, j1 == -1, arithmetically it's bigger, right?
    // In Soviet C++ unsigned type cast you!
    if (j1 < i1) {
        std::cout << "Unreachable;";
    }

    // Unreachable section will never be reached, 
    // because unsigned(-1) being casted to maxint == 2147483647 == 0xffffffff
}

// This cast was really unexpected
// TODO: some explanation
void show_shift(){
    
    // Let's have, for example, b10(1075) == b2(10000110011)
    // And move it right as many times how many bits in the integer type

    // In our case it's going to be 64
    long long l = 0;

    // 1st shift it's going to be b2(100001100110)
    // 2nd b2(1000011001100)
    // 3rd b2(10000110011000)
    // Let's see how it works...

    std::cout << "Wanna see some type magic?\n";
    for (size_t i = 0; i < sizeof(long long)* 8; ++i){
        l = 1075 << i;
        std::cout << "Step " << i << " magic =  " << bitwise(l) << '\n';
    }

    l = 0;
    std::cout << "\nNo-no, David Blaine!\n";
    for (size_t i = 0; i < sizeof(long long) * 8; ++i) {
        l = 1075LL << i;
        std::cout << "Step " << i << " magic =  " << bitwise(l) << '\n';
    }
}

int main()
{
    return 0;
}

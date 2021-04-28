#include <iostream>

// Facts about arrays
void array_facts()
{
    // A bit obsolete typedef for array
    typedef int Vector3D[10];
    Vector3D numbers = { 1, 2, 3 };

    // string literal is an array
    const char* str = "Some string";
    std::cout << "Size of the string '" << str << "' is " << sizeof(str);

    // string literal is constant by default
    // Even if you skip 'const' specifier
    const char* const_string = "Other string";
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
    // Yes! It is correct!

    // Sincs plain ANSI C times, for array x[1] is equal  to 1[x] is equal to *(1 + x)
    // ISO / IEC 9899:TC2 : 6.5.2.1 Array subscripting
    // Thus, &*E is equivalent to E (even if E is a null pointer), and &(E1[E2]) to ((E1)+(E2))

    // Simply says, 1[array] == array[1]
    // 1 + *pointer == *pointer + 1

    // However, don't try to change it, since l-value specifies constant object
    //x["ABCDEF"] = 'Z'; error C2166
}

// However, array and pointer are not the same!
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
    for (int s = 0; s < 42; ++s){
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
    // diff2 == 41. It means (&array + 1) points to the LAST array item + offset 1
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


int main()
{
    // TODO: insert any function call
    return 0;
}

#include <iostream>

#pragma clang diagnostic ignored "-Wuninitialized"

void dereference_null()
{
    int* ptr = nullptr;
    int value = *ptr; // UB
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

void buffer_overflow()
{
    char buffer[5];
    buffer[10] = 'A'; // UB
    std::cout << "Value of this variable is undefined: buffer[10] = " << buffer[10] << std::endl;
}

void signed_overflow()
{
    int max = std::numeric_limits<int>::max();
    int overflowed = max + 1; // UB
    std::cout << "Value of this variable is undefined: overflowed = " << overflowed << std::endl;
}

void division_by_zero()
{
    int i = 1;
    int j = 0;
    int result = i / j; // UB
    std::cout << "Value of this variable is undefined: result = " << result << std::endl;
}

void uninitialized_var()
{
    int i; // UB
    std::cout << "Value of this variable is undefined: i = " << i << std::endl;
}

void array_out_of_bounds()
{
    int array[5];
    int i = 5;
    int value = array[i]; // UB
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

void expired_pointer()
{
    int* ptr;
    {
        int x = 5;
        ptr = &x;
    }
    int value = *ptr; // UB (ptr points to a destroyed variable)
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

void type_punning_pointer()
{
    int i = 1;
    float* ptr = reinterpret_cast<float*>(&i);
    float value = *ptr; // UB
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

void evaluation_order()
{
    // example of UB due to bad initialization
    int i = i * 0;

    // no defined order of initialization within one expression

    // Suppress optimization
    std::cout << "Value of this variable is undefined: i = " << i << std::endl;
}

void deleted_pointer()
{
    int* ptr = new int;
    delete ptr;
    int value = *ptr; // UB
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

void call_non_exist_virtual()
{
    struct A
    {
        virtual void foo() {}
    };

    struct B : A
    {
        void foo() override {}
    };

    A* ptr = new B;
    ptr->foo();
    delete ptr;
    ptr->foo(); // UB
}

void dangling_reference()
{
    int* ptr = new int;
    int& ref = *ptr;
    delete ptr;
    int value = ref; // UB
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

void realloc_fails()
{
    int* ptr = new int;
    int* new_ptr = new int;
    delete ptr;
    new_ptr = static_cast<int*>(realloc(new_ptr, 100)); // UB
    std::cout << "Value of this variable is undefined: new_ptr = " << new_ptr << std::endl;
}

void misalignment()
{
    char* ptr = new char;
    int* int_ptr = reinterpret_cast<int*>(ptr);
    int value = *int_ptr; // UB
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

void break_strict_aliasing()
{
    int i = 1;
    float* ptr = reinterpret_cast<float*>(&i);
    float value = *ptr; // UB
    std::cout << "Value of this variable is undefined: value = " << value << std::endl;
}

int missed_return()
{
    int i = 1;
    if (i == 1)
    {
        return 1;
    }
    // UB
}

int main()
{
    return 0;
}

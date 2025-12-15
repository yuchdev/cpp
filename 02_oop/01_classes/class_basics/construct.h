#pragma once

// We consider in this class basic problems of construction and destruction
// The class could be created and destructed in the number of methods
// 1. Named object in the automatic memory (in the stack)
// 2. Object in a free dynamic memory (new/delete).
// 3. Non-static class member.
// 4. Object-element of array.
// 5. Local static object.
// 6. Global object. // TODO: difference global and static?
// 7. Temporary object
// 8. Object created by a user-defined allocator.
// 9. Member of union

// Unlike method, c-tor can't be virtual, static, and does not return any value

/** @brief
That's a simple class
*/
struct SimpleStruct
{
    int a;
    SimpleStruct();
    SimpleStruct(int);
    ~SimpleStruct();
};

class A;

/** @brief
That's a class, containing other member objects
*/
class SimpleClass
{
public:
    // Default c-tor always call default c-tors of aggregated classes
    SimpleClass();

    // C-tor with default members may work like a default
    SimpleClass(int i/* = 0*/);

    // Note: The compiler does not generate const operator=(const A&) if non-const is defined
    SimpleClass(const SimpleClass& c);

    // Copy c-tor defines how object behave if returned by value
    // Move c-tor defines, how object pass its state to another object
    ~SimpleClass();

    // In order to access a static class member, you should define a static method
    static void out_static_array();

    A get_a(A a);

private:

    // C-tor of this class is being called in default c-tor
    SimpleStruct intrnal;
    int b {};

    // You can define a C-array member, however, 
    // on practice it may cause problem with initialization
    int arr[5];

    // Such array can only call default c-tors for its members ()
    // For VC++ it cause warning C4351.

    // You can't define suck array size by another class member, even constant:
    // int arr1[sz];

    // Such restrictions render C-array members useless
    // You can use C++ std::array, std::vector
    // However, static C-arrays were in common use for defining settings,
    // common for the class in general
    static const char ch_array[];
    static const int int_array[];

    // Integer static class member could be used as an object counter
    // It is inherited by all the class children!
    static int _counter;
};

/** @brief
Placing copy c-tor and assignment in private section was the method to deny class copying before C++11
*/
class NonCopyClass
{
private:
    int d;
    NonCopyClass(const NonCopyClass& n);
    NonCopyClass& operator=(const NonCopyClass& n);
public:
    NonCopyClass() : d(5) {  /*d = 5; - better not to do so*/ }
};

/** @brief
The static method for initializing static class members
*/
void create_static(int i);

/** @brief
Meyers' singleton - create and return static member
*/
int& return_static();

/** @brief
This class contains a C-pointer. The pointer could be initialized as any other class member
*/
class ContainsPointer
{
    SimpleClass* _pc = nullptr;

    // The class can contain a pointer to itself
    // However, be careful with recursion here!
    ContainsPointer* _p_cp;

    // You can initialize the pointer in declaration, in constructor,
    // or as a default value of constructor
    ContainsPointer(SimpleClass* pc = new SimpleClass) : _pc(pc) {}
    ~ContainsPointer() { delete _pc; }
};


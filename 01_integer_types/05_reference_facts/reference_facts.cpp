#include <iostream>

// Facts about references

void lvalue_reference()
{

    // * Aliasing is when more than one lvalue refers to the same memory location 
    // (when you hear lvalue, think of things (variables) that can be on the left-hand side of assignments), 
    // i.e. that are modifiable. As an example:

    int anint = 0;
    int *intptr=&anint;

    // * Reference effectively an alias

    // * Reference must be initialized, and could not be re-initialized
    // It always refers to one object (being an alias of that object)
    int val = 10;
    int& val_ref = val;


    // * Underneath, reference is a const pointer which can't be nullptr, 
    // and de-reference automatically

    // * Reference to reference is just a reference. This is sometimes known as reference collapse
    // We cannot create the pointer to a reference either

    // * When we meet function T& ref = boo();
    // we expect that object lives long enough not to create "dead" reference

    // * Reference-returning methods are a convenient way to create function calls chain
    // object.method1().method2();
    // Including overloaded operator calls
    // stream << value1 << value2;
    // a += b += c; // (a, b, c are objects with overloaded operator+=())
    // And even
    // a, b, c; // (a, b, c are objects with overloaded operator,())

    // * The most common problem with a reference is overliving a referent
    // After that it refers to some garbage in memory
    // You may artificially create this situation, far example, like this:
    int* p = nullptr;
    int& r = *p;
    // Nope, don't create it, it's illegal according to  C++ 2014 section 8.3.2 (UB)

    // * keyword this has to be a reference: always points to a referent, 
    // can't be zero, can't overlive it.
    // However, keyword this was introduced before references were added to C++

    // * Use references when you can, and pointers when you have to
}

void rvalue_reference()
{
    // TODO:
}

void function_reference()
{
    // TODO:
}

int main()
{
    // TODO: insert any function call
    return 0;
}

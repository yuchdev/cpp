#include <iostream>

// Facts about references
// TODO:
// * Const ref makes rvalue object live the same time as ref
// * Non-const pointer is assigned to const (not otherwise!)
// * Rvalue references (enabling move semantics)

void lvalue_reference()
{
    // * Aliasing is when more than one lvalue refers to the same memory location
    // (when you hear lvalue, think of things (variables) that can be on the left-hand side of assignments), 
    // i.e. that are modifiable. As an example:

    int anint = 0;
    int* intptr = &anint;

    // * Reference effectively an alias

    // * Reference must be initialized
    // TODO: can it be re-initialized?
    int val1 = 10;
    int val2 = 20;
    int& val_ref = val1;
    std::cout << val_ref << '\n'; // prints '10'
    val_ref = val2;
    std::cout << val_ref << '\n'; // prints '20'

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

    // * You can't save references in STL containers, 
    //   due to its inability to be initialized with the default value
}

// "perfect swap" (almost)
template<class T>
void swap(T& a, T& b)
{
    // Since move(x) does not move x (it simply produces an rvalue reference to x),
    // it would have been better if move() had been called rval(),
    // but by now move() has been used for years

    // move from a
    T tmp { std::move(a) };

    //move from b
    a = std::move(b);

    //move from tmp
    b = std::move(tmp);
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
    lvalue_reference();
    return 0;
}

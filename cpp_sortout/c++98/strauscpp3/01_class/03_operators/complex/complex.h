#pragma once
#include <vector>
#include <cmath>
#include <cstddef>

// The class covers operator overloading techniques
// Operator overloading could be used both to work with concrete types, 
// and designing generic and abstract interfaces (e.g. operators -> [] ())

/*
Could be overloaded
+	-	*	/	=
%	^	&|	~
==	<	>	!=	<=	>=
+=	-=	*=	/=
%=	^=	&=	|=
<<	>>	>>=	<<=
!	&&	||
++	--
->	->*	[]	() ,
new	new[]	delete	delete[]
operator {type}()
*/

/*
Can't be overloaded
::	.	.*	?:
*/

/** @brief
The class complex_t implements the arithmetic of a complex number
and demonstrates the principles of overloading operations in C++
*/
class complex_t
{
public:
    // Default constructor, and the same time constructor for real and imaginary parts
    // Designing classes, assume that there are no reasonable default values for most functions
    complex_t(double r = 0., double i = 0.);

    // Constructor for implicit conversion from int
    complex_t(int r);

    // Accessors
    inline double real() const;
    inline double imag() const;

    // OPERATORS OVERLOAD

    // Operators which are better to be overloaded by a method
    // have to return a reference to this, as they change their state
    // Is is done in odeer to provide the ability of "chaining" them (`a += b += c ...`)
    // and prevent accidental creation of unnamed objects
    complex_t& operator+=(const complex_t& a);
    complex_t& operator-=(const complex_t& a);
    complex_t& operator*=(const complex_t& a);
    complex_t& operator/=(const complex_t& a);

    // These operators are constant, since they do not change the state of an object
    bool operator==(const complex_t& a) const;
    bool operator!=(const complex_t& a) const;

    // Increment and decrement
    // prefix version, returns a reference
    complex_t& operator++();
    complex_t& operator--();

    // postfix version, returns value on stack, which is worse for performance
    // accept a dummy parameter of type int
    complex_t operator++(int);
    complex_t operator--(int);

    // Operators that must be overloaded as class methods
    // `operator=` is not needed in this case. It returns a reference to itself, since it modifies the state, and needs a check for self-assignment
    // `operator->` for smart pointers and implementation of indirect access (iterators)
    // `operator[]` for range checking (must be constant and non-constant)
    // `operator()` in functors, substring operators (substr) and for multidimensional matrices

    // Type conversion operators have no return value in declaration
    operator int();
    operator double();

    // It could be used for converting state of an object into a value
    // For example in the `while( cin >> x )`
    // `cin >> x` returns `istream&` and is converted to int

    // Binary addition operators can be defined by either method or function.
    // It's more convenient to do it with a function, because it provides commutativity of the operation,
    // nnd it works in cases where we want to pass the first parameter other than `this`
        // In other words, if you implement `operator+` as a method you can do like this `a + 5`, but can not like this `5 + a`
    // It makes sense to declare both class and friend operators in a common namespace,
        // effectively declaring a unified interface.
    // There is no precedence for members over non-members when browsing operators

    // Operators overloaded by friend functions, since all parameters may type complex_t&. params of types int or double
    // can be implicitly transformed, which gives more flexibility and commutativity of operations.
    // As on option, it is possible instead of declaring them as friends, 
        // organize an interface to the private part of the class.
    // Declaration of friend functions could may coincide with implementation
    // Note, in binary operators it is make sense to return by const value,
    // so that it was not possible to use it like `(a+b) = c`
    friend const complex_t operator+(const complex_t& a, const complex_t& b)
    {
        complex_t tmp(a);
        return tmp += b;
    }

    friend const complex_t operator-(const complex_t& a, const complex_t& b)
    {
        complex_t tmp(a);
        return tmp -= b;
    }

    friend const complex_t operator*(const complex_t& a, const complex_t& b)
    {
        complex_t tmp(a);
        return tmp *= b;
    }

    friend const complex_t operator/(const complex_t& a, const complex_t& b)
    {
        complex_t tmp(a);
        return tmp /= b;
    }
    // In high-performance environment it is worth abandoning implicit conversions in operators sometimes
    // in order to improve performance.
    // Such operators could be explicitly overloaded like `operator+( const complex_t& a, int b )`
    // or better implemented as template functions.

// radius-vector of a complex number
    friend double r_vector(const complex_t& c)
    {
        return sqrt((c.re * c.re) + (c.im * c.im));
    }

private:
    double re, im;
};

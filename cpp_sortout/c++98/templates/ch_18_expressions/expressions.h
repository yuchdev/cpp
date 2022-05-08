#pragma once

// Calculation properties of an expression template - transfer either by value or by reference
template <typename T> class A_Scalar;

// Basic template - default transfer by reference (for vectors)
template <typename T>
struct A_Traits
{
    typedef T const& ExprRef;
};


// Specialization for a scalar, transfer by value
template <typename T>
struct A_Traits< A_Scalar<T> >
{
    typedef A_Scalar<T> ExprRef;
};

// Scalar object, as an "adapter" for template expressions
// so that the templates see it as an array as well
template <typename T>
class A_Scalar
{
private:
    T const& val;
public:
    A_Scalar(T const& v) :val(v) {}

    T operator[](size_t) const { return val; }
    T const& rep() const { return val; }
    size_t size() const { return 0; }
};

// Objects for calculating template expressions

// +
// Binary operation with delayed computation
template <typename T, typename OP1, typename OP2>
class A_Add
{
private:
    // Reference to operands (op1 + op2)
    typename A_Traits<OP1>::ExprRef op1;
    typename A_Traits<OP2>::ExprRef op2;
public:

    // Reference to operands (op1 + op2)
    A_Add(OP1 const& a, OP2 const& b)
        :op1(a), op2(b)
    {
    }

    // "Delayed" calculation of the operation
    // The sum is being calculated on demand, when accessing an array element
    T operator[](size_t i) const
    {
        return op1[i] + op2[i];
    }

    size_t size() const
    {
        //assert(op2.size() == 0);
        return op2.size();
    }

};

// *
// Binary operation with delayed computation
template <typename T, typename OP1, typename OP2>
class A_Mult
{
private:
    // Reference to operands (op1 * op2)
    typename A_Traits<OP1>::ExprRef op1;
    typename A_Traits<OP2>::ExprRef op2;
public:

    // Reference to operands (op1 + op2)
    A_Mult(OP1 const& a, OP2 const& b)
        :op1(a), op2(b)
    {
    }

    // "Delayed" calculation of the operation
    // The product is being calculated on demand, when accessing an array element
    T operator[](size_t i) const
    {
        return op1[i] * op2[i];
    }

    size_t size() const
    {
        return op2.size();
    }
};

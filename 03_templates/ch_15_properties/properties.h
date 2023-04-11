#pragma once

// Strategies and traits

// ------------------- Simple accumulator -------------------
// For types that support accumulation by addition
template <typename T>
inline T simple_accumulate(const T* begin, const T* end)
{

    // Total value
    T total = T();

    // Iterating
    while (begin != end) {
        total += *begin;
        ++begin;
    }
    return total;
}


// The disadvantage is that the return type is not protected against overflow
// E.g. if the total value is char or byte

// ------------------- Fixed type traits -------------------
// The possible solution is the explicit defining the return type depending on the summarized type
// Introduce a traits class, that defines the return type depending on the passed type
template <typename T>
class simple_accumulator_traits {};

// The types themselves are defined as partial specializations
template <>
class simple_accumulator_traits<char>
{
public:
    typedef int AccType;
};

template <>
class simple_accumulator_traits<short>
{
public:
    typedef int AccType;
};


template <>
class simple_accumulator_traits<int>
{
public:
    typedef long AccType;
};

template <>
class simple_accumulator_traits<unsigned int>
{
public:
    typedef unsigned long AccType;
};

template <>
class simple_accumulator_traits<float>
{
public:
    typedef double AccType;
};

// Then the function will look like
template <typename T> inline typename
simple_accumulator_traits<T>::AccType traits_accumulate(const T* begin, const T* end)
{

    // Total value with zero initialization
    typedef typename simple_accumulator_traits<T>::AccType AccT;
    AccT total = AccT();

    // Iteration
    while (begin != end) {
        total += *begin;
        ++begin;
    }
    return total;
}


// ------------------- value traits -------------------
// Besides types, traits can also store values
// E.g. "the zero value", which is called by the default constructor
template <typename T>
class value_accumulator_traits;

template <>
class value_accumulator_traits<char>
{
public:
    typedef int AccType;
    static AccType zero() { return 0; }
};

template <>
class value_accumulator_traits<short>
{
public:
    typedef int AccType;
    static AccType zero() { return 0; }
};


template <>
class value_accumulator_traits<int>
{
public:
    typedef long AccType;
    static AccType zero() { return 0l; }
};

template <>
class value_accumulator_traits<unsigned int>
{
public:
    typedef unsigned long AccType;
    static AccType zero() { return 0u; }
};

template <>
class value_accumulator_traits<float>
{
public:
    typedef double AccType;
    static AccType zero() { return 0.; }
};

template <typename T>
class value_multiply_traits;

template <>
class value_multiply_traits<int>
{
public:
    typedef int AccType;
    static AccType zero() { return 1; }
};


// Then the function will look like
template <typename T> inline typename
value_accumulator_traits<T>::AccType traits_vals_accumulate(const T* begin, const T* end)
{

    // Total value with custom initialization
    typedef typename value_accumulator_traits<T>::AccType AccT;
    AccT total = value_accumulator_traits<T>::zero();

    // Iteration
    while (begin != end) {
        total += *begin;
        ++begin;
    }
    return total;
}

// ------------------- Traits with parameters  -------------------
// The above traits called fixed because they cannot be replaced in the algorithm
// This can be solved by adding to the algorithm a template parameter with a default value 
template <typename T, typename AccTraits = value_accumulator_traits<T> >
class custom_accumulator_traits
{
public:
    static typename AccTraits::AccType accum(const T* begin, const T* end)
    {
        typename AccTraits::AccType total = AccTraits::zero();

        while (begin != end) {
            total += *begin;
            ++begin;
        }
        return total;
    }
};

// ------------------- Strategies -------------------
// Obviously, accumulation can be more than just addition
// We can, for example, accumulate by multiplication. 
// To do so, let's introduce the concept of accumulation strategy (accumulation policy)
class SumPolicy
{
public:
    template <typename T1, typename T2>
    static void accumulate(T1& total, const T2& value)
    {
        total += value;
    }
};

class MultPolicy
{
public:
    template <typename T1, typename T2>
    static void accumulate(T1& total, const T2& value)
    {
        total *= value;
    }
};

template <typename T
    , typename Policy = SumPolicy
    , typename AccTraits = value_accumulator_traits<T> >
    class strategy_accumulator_traits
{
public:
    static typename AccTraits::AccType accum(const T* begin, const T* end)
    {
        typename AccTraits::AccType total = AccTraits::zero();

        while (begin != end) {
            Policy::accumulate(total, *begin);
            ++begin;
        }
        return total;
    }
};

// ------------------- Strategies with template argument -------------------
// We can replace the strategy classes with templates
// And add a third parameter for the initial element
template <typename T1, typename T2>
class SumPolicyT
{
public:
    static void accumulate(T1& total, const T2& value)
    {
        total += value;
    }
};

template <typename T1, typename T2>
class MultPolicyT
{
public:
    static void accumulate(T1& total, const T2& value)
    {
        total *= value;
    }
};

template <typename T
    , template<typename, typename> class Policy = SumPolicyT
    , typename AccTraits = value_accumulator_traits<T> >
    class strategy_tmpl_accumulator_traits
{
public:
    static typename AccTraits::AccType accum(const T* begin
        , const T* end
        , typename AccTraits::AccType total_1st = AccTraits::zero())
    {

        typedef AccTraits::AccType AT;
        AT total = total_1st;

        while (begin != end) {
            Policy<AT, T>::accumulate(total, *begin);
            ++begin;
        }
        return total;
    }
};

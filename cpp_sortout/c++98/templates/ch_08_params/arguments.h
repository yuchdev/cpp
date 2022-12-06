#pragma once

// --- Function template arguments ---
// Could be defined:
//- Explicitly <int, int>
//- using class name (type_t a; X<a>)
//- by default
//- argument deduction

// Some function template arguments impossible to deduct, e.g.
template <typename DestType, typename SrcType>
inline DestType implicit_cast(SrcType t)
{
    return static_cast<DestType>(t);
}
// Here SrcType can be deducted, but DestType can't

// It's possible to make template function design a bit worse, 
// placing SrcType as a first template param.
// Now none of them can be deducted
template <typename SrcType, typename DestType>
inline DestType implicit_cast2(SrcType t)
{
    return static_cast<DestType>(t);
}

// During template function overload non-ambigious argument deduction is not always possible
// E.g. take a template function, accepting function pointer and its argument
template <typename Func, typename T>
inline void apply(Func fptr, T t)
{
    fptr(T);
}

// Create the family of overloaded functions
template <typename T>
T multi(T t)
{
    return t;
}

template <typename T>
T multi(T* t)
{
    return *t;
}

void test_functions_family()
{
    // Uncomment for error
    // error C2784: 'void apply(Func,T)' : could not deduce template argument for 'overloaded function type' from 'overloaded function type'
    //apply(multi<int>, 3);
    //apply(multi<int>, 5);
    // Both overloaded functions match the overload, even we explicitly define the type
}

// ------------------- Type arguments -------------------
// Unnamed classes and enums can't be template params (except defined through typedef)
// E.g.
template <typename T>
class List_t
{
public:
    List_t() : _t() {}
    List_t(const T& t) : _t(t) {}
    T _t;
};

// Unnamed structure with and without typedef 
struct
{
    double x;
} str1;

typedef struct
{
    double x;
} str2;

// Unnamed enum with and without typedef 
enum { red, green, blue } color1;
typedef enum { red1, green1, blue1 } color_t;

void test_argument_types()
{
    struct association
    {
        int* p;
    };
    // It should not compile?
    List_t<association*> err1;

    // Unnamed structores don't work, except declared through typedef
    //List_t<str1> s1;
    List_t<str2> s1;

    // Same for enums
    //List_t<color1> c1;
    List_t<color_t> c2;
}

// Arguments of function templaes considered equivalent
// if they are pairwise identical during the compilation

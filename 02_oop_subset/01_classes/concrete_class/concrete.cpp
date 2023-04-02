#include <iostream>
#include <vector>
#include <map>
#include <memory>

/*

1. Class complex
* {}-init

2. Class vector
* rvalue reference
* move semantic
* initializer_list constructors

3. Return unique_ptr
* move semantic

4. Lambdas

5. Variadic templates
* nested args

6. Type aliases

*/

namespace cpp4
{

// 1. Class complex
// Concrete class that behaves like a built-in type
class complex
{
public:

    // constructor uses {} notation
    complex(double re = 0., double im = 0.) : re_ { re }, im_ { im }{}

    double real() const { return re_; }
    double imag() const { return im_; }

    // TODO: different operators...

private:
    double re_;
    double im_;
};


// 2. Class vector
template <typename T>
class vector
{
public:

    // default constructor without any additional init
    // already initialized at declaration
    vector() = default;

    // The std::initializer_list used to define the initializer-list constructor 
    // Passed by value
    vector(std::initializer_list<T> l) : data_ { new T[l.size()] }
    {
        // so that keep exception-safety
        std::copy(l.begin(), l.end(), data_);
        sz_ = l.size();
    }

    ~vector()
    {
        delete[] data_;
    }

    // copy semantic
    // should be suppressed by = delete notation
    // it can be used to suppress any operation
    vector(const vector& rhs) : data_ { new T[rhs.size()] }
    {
        // so that keep exception-safety
        std::copy(rhs.data_, rhs.data_ + rhs.size(), data_);
        sz_ = rhs.size();
    }

    // assign semantic
    // should be suppressed by = delete notation
    // it can be used to suppress any operation
    vector& operator=(const vector& rhs)
    {

        if (&rhs != this) {
            // so that keep exception-safety
            T* data = nullptr;
            try {
                data = new T[rhs.size()];
                std::copy(rhs.data_, rhs.data_ + rhs.size(), data);
                delete data_;
                data_ = data;
                sz_ = rhs.size();
            }
            catch (...) {
                delete[] data;
                throw;
            }
        }
        return *this;
    }

    // move semantic
    // The && means 'rvalue reference' and is a reference to which we can bind an rvalue
    // A move constructor does not take a const argument: after all, 
    // a move constructor is supposed to remove the value from its argument
    // A move assignment is defined similarly
    vector(vector&& rhs) noexcept
    {
        data_ = rhs.data_;
        sz_ = rhs.sz_;
        rhs.data_ = nullptr;
        rhs.sz_ = 0;
    }

    // If the definition of a class X does not explicitly declare a move constructor, 
    // one will be implicitly declared as defaulted if and only if
    // * X does not have a user-declared copy constructor,
    // * X does not have a user-declared copy assignment operator,
    // * X does not have a user-declared move assignment operator,
    // * X does not have a user-declared destructor, and
    // * the move constructor would not be implicitly defined as deleted.

    // Plainly speaking, the move constructor will be implicitly declared if:
    // The class does not have user-declared any of the other special member functions.
    // The move constructor can be sensibly implemented by moving all its members and bases.

    size_t size() const
    {
        return sz_;
    }

private:
    size_t sz_ = 0;
    T* data_ = nullptr;
};

// 3. Return unique_ptr
std::unique_ptr<int> ret_unique_ptr()
{
    std::unique_ptr<int> p(new int(10));
    return p;
    // behaves like return move( p );

    // Copy elision refers to a compiler optimization technique 
    // that eliminates unnecessary copying of objects.
    // This elision of copy/move operations, called copy elision, is permitted
    // in a return statement in a function with a class return type, 
    // when the expression is the name of a non-volatile automatic object 
    // with the same qualifier - unqualified type as the function return type

    // So this should call copy construction, however calls move
    // and works like a new RVO
}

// 4. Functor
// One particularly useful kind of template is the function object (sometimes called a functor)
// A predicate is something that we can invoke to return true or false

// The notation [&](int a){ return a<x; } is called a lambda expression. 
// It generates a function object exactly like Less_than<int>{x}. 
// The [&] is a capture list specifying that local names used (such as x) 
// will be passed by reference. Had we wanted to 'capture' only x, we could have said so: [&x]
// Had we wanted to give the generated object a copy of x, we could have said so: [=x]. 
// Capture nothing is [], capture all local names used by reference is [&], 
// and capture all local names used by value is [=]


// 5. Variadics
// an arbitrary number of arguments of arbitrary types
// Should be at least 3 functions:
// * recursive call for all params
// * call for the head param
// * end-of-recursion

// recursive call for all params
template <typename T>
void process(T t)
{
    std::cout << t << "=" << typeid(t).name() << std::endl;
}

// close recursion
void variadic_func() {}

// call for the head param
template <typename T, typename... Tail>
void variadic_func(T head, Tail... tail)
{
    process(head);
    variadic_func(tail...);
}

// 6. Type aliases
// introduce a synonym for a type or a template

// as a synonym of typedef
using inch_t = unsigned long;

// The using syntax has an advantage when used within templates. 
// If you need the type abstraction, but also need to keep template parameter 
// to be possible to be specified in future. You should write something like this
template<typename C>
using ElementType = typename C::value_type;

template<typename Container> void algo(Container& c)
{
    vector<ElementType<Container>> vec;
    std::cout << vec.size() << std::endl;
}

// another example
template<typename Key>
using StringMap = std::map<Key, std::string>;

} // namespace cpp4


void show_complex()
{
    cpp4::complex c1;
    cpp4::complex c2 { 1.0 };
    cpp4::complex c3 { 1.0, 2.0 };

    std::cout << c1.imag() << std::endl;
    std::cout << c2.imag() << std::endl;
    std::cout << c3.imag() << std::endl;
}

void show_vector()
{
    cpp4::vector<int> v1 { 1,2,3,4,5 };
    std::cout << v1.size() << std::endl;
}

void show_return_by_move()
{
    std::unique_ptr<int> p = cpp4::ret_unique_ptr();
    std::cout << *p << std::endl;
}

// show lambdas in 11

void show_variadics()
{
    cpp4::variadic_func(1, 1.2, "hello");
}

void show_type_aliaces()
{
    std::vector<int> v;
    cpp4::algo(v);

    cpp4::StringMap<int> m;
    m[1] = "atatat";
    m[2] = "tatata";
}

int main()
{
    show_complex();
    show_vector();
    show_variadics();
    show_type_aliaces();
    return 0;
}

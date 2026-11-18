#include <cassert>
#include <compare>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

/*

1. Class complex
* {}-init
* operator<=>, operator== (C++20)

2. Class vector
* rvalue reference
* move semantic
* initializer_list constructors

3. Return unique_ptr
* move semantic, guaranteed copy elision

4. Lambdas

5. Type aliases

*/

namespace cpp
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

    complex& operator+=(const complex& rhs)
    {
        re_ += rhs.re_;
        im_ += rhs.im_;
        return *this;
    }

    friend complex operator+(complex lhs, const complex& rhs) { return lhs += rhs; }

    // Complex numbers have no natural mathematical total order -- there is
    // no "a < b" that is both well-defined and useful the way it is for
    // real numbers. Defining <=> here is therefore a DESIGN CHOICE, not a
    // mathematical fact, and the choice is debatable: this orders
    // lexicographically by (re_, im_), purely because that is what a
    // defaulted <=> does for free given the members' declaration order.
    // An equally defensible, genuinely different choice would compare by
    // magnitude (re_*re_ + im_*im_) instead -- which would make, say,
    // (3, 0) and (0, 3) compare EQUAL under <=> while still comparing
    // unequal under == (since == is specified separately, see below).
    // Pick deliberately; don't let "the compiler can default it" decide
    // what "less than" means for your type.
    auto operator<=>(const complex&) const = default;

    // operator== is specified independently of <=> (a defaulted <=> alone
    // does not make == ... a defaulted == is still required if you want
    // one, though the compiler WILL synthesize !=, <, >, <=, >= from a
    // single defaulted <=>).
    bool operator==(const complex&) const = default;

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
    // chould be suppressed by = delete notation
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
                delete[] data_;
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
    // Returning a NAMED local object is (N)RVO: the compiler is PERMITTED
    // to construct it directly in the caller's storage and skip the move
    // entirely, but it isn't required to -- unique_ptr compiles here only
    // because it has an accessible move constructor to fall back on if
    // the compiler doesn't elide.
}

// A type with NEITHER a copy constructor NOR a move constructor. There is
// no fallback: if returning one by value ever required an actual copy or
// move call, this type simply could not be returned by value at all.
struct immovable_tag
{
    explicit immovable_tag(int id) : id_(id) {}
    immovable_tag(const immovable_tag&) = delete;
    immovable_tag(immovable_tag&&) = delete;
    int id_;
};

// This is the C++17 GUARANTEE, not the older permitted-but-optional RVO
// above: the return expression is a genuine PRVALUE (the result of a
// function call, constructed directly, never a named local), so the
// language requires it to be constructed directly in the caller's
// storage -- "copy elision" here isn't optional, and isn't really
// "eliding" anything, because no copy or move was ever going to happen
// in the first place. The fact that this function compiles at all, for a
// type with copy and move both deleted, is the proof.
immovable_tag make_immovable_tag(int id)
{
    return immovable_tag(id);
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


// 5. Type aliases
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

} // namespace cpp


void show_complex()
{
    cpp::complex c1;
    cpp::complex c2 { 1.0 };
    cpp::complex c3 { 1.0, 2.0 };

    std::cout << c1.imag() << std::endl;
    std::cout << c2.imag() << std::endl;
    std::cout << c3.imag() << std::endl;

    assert((c2 + c3) == cpp::complex(2.0, 2.0));

    // The defaulted <=> orders lexicographically by (re_, im_), so (0,5)
    // sorts as LESS than (1,0) even though (0,5) has the larger magnitude
    // -- living proof that this "ordering" carries no mathematical
    // meaning about complex numbers, only the one the member declaration
    // order happened to assign it.
    cpp::complex small_magnitude_but_lex_greater(1.0, 0.0);
    cpp::complex large_magnitude_but_lex_less(0.0, 5.0);
    assert(large_magnitude_but_lex_less < small_magnitude_but_lex_greater);

    // A single defaulted <=> gives every relational operator for free.
    assert(c2 < c3);
    assert(c3 > c2);
    assert(c2 <= c2);
    assert(c2 >= c2);

    // But == needed its own separate `= default` -- <=> alone does not
    // imply it.
    assert(cpp::complex(1.0, 2.0) == c3);
    assert(c1 != c2);
}

void show_vector()
{
    cpp::vector<int> v1 { 1,2,3,4,5 };
    std::cout << v1.size() << std::endl;
}

void show_return_by_move()
{
    std::unique_ptr<int> p = cpp::ret_unique_ptr();
    std::cout << *p << std::endl;

    cpp::immovable_tag tag = cpp::make_immovable_tag(42);
    assert(tag.id_ == 42);
}

// show lambdas in 11

void show_type_aliaces()
{
    std::vector<int> v;
    cpp::algo(v);

    cpp::StringMap<int> m;
    m[1] = "atatat";
    m[2] = "tatata";
}

int main()
{
    show_complex();
    show_vector();
    show_return_by_move();
    show_type_aliaces();
    return 0;
}

#include <iostream>
#include <cmath>
#include <complex>
#include <algorithm>

using namespace std;

/*

New features:
* Language mapping of {}-list to std::initializer_list (3.2.1.3, 17.3.4)
* Delegating constructors (17.4.3)
* In-class member initializers (17.4.4)
* Control of defaults: default (17.6) and delete (17.6.4)
* initializer_list constructors for containers (3.2.1.3, 17.3.4, 31.3.2)
* Move semantics for containers (3.3.1, 17.5.2, 31.3.2)

Examples:
1. Memberwise init (struct and by friend) (17.2.3)
2. Explicit destroy (17.2.4)
3. initializer_list Constructor Disambiguation (17.3.4.1)
4. Delegating Constructors (17.4.3)
5. static member init(17.4.5)
6. operator= safety (17.5.1)
7. Prevent slicing (17.5.1.4)
8. Using Default Operations (17.6.3)
9. Deleted functions (17.6.4)
*/

//1. Memberwise init(struct and by friend) (17.2.3)
namespace cpp4
{

// test memberwise init completely opened
struct opened
{
    int a;
    double b;
};

// test memberwise init by friend
class closed
{
    friend void show_memberwise_init();
    int a;
    double b;
};

// place in the same namespace so that find friend
void show_memberwise_init()
{
    // all members are opened, ok
    cpp4::opened op = { 1, 1.0 };

    // members are closed, does not work even for friend
    //cpp4::closed cl { 1, 1.0 };
}

} // namespace cpp4 



//2. Explicit destroy(17.2.4)
namespace cpp4
{

// We can prevent destruction of an X by declaring its destructor =delete (17.6.4) or private
class prevent_destruction_private
{
public:

    // explicit destruction
    void destroy()
    {
        this->~prevent_destruction_private();
    }
private:

    // could be = delete, we must destruct in destroy() manually then
    ~prevent_destruction_private() {}
    int i = 0;
};

// We can prevent destruction of an X by declaring its destructor =delete (17.6.4) or private
class prevent_destruction_delete
{
public:

    // explicit destruction
    void destroy()
    {

        // we can't do the same as private: with = delete
        // error C2280: 'void *cpp4::prevent_destruction_delete::__delDtor(unsigned int)': attempting to reference a deleted function
        // this->~prevent_destruction_delete();

        // do some manual removal stuff instead
    }

    ~prevent_destruction_delete() = delete;
private:
    int i = 0;
};

} // namespace cpp4 

void show_explicit_destroy()
{
    // we can't create automatic object as destructor is private
    // error C2248: 'cpp4::prevent_destruction_private::~prevent_destruction_private': 
    // cannot access private member declared in class 'cpp4::prevent_destruction_private'
    // cpp4::prevent_destruction_private p1;

    // free object is ok
    cpp4::prevent_destruction_private* p2 = new cpp4::prevent_destruction_private;

    // we can't create automatic object if destructor is deleted
    // error C2280: 'cpp4::prevent_destruction_delete::~prevent_destruction_delete(void)': 
    // attempting to reference a deleted function
    // cpp4::prevent_destruction_delete p3;

    cpp4::prevent_destruction_delete* p4 = new cpp4::prevent_destruction_delete;

    // can't delete automatic object this way
    // p1.destroy();

    p2->destroy();

    // // can't delete automatic object this way
    // p3.destroy();

    p4->destroy();
}

//3. initializer_list Constructor Disambiguation(17.3.4.1)
namespace cpp4
{

// For selecting a constructor, default and initializer lists take precedence
struct my_constructor
{
    my_constructor()
    {
        std::cout << "default constructor" << std::endl;
    }

    my_constructor(std::initializer_list<int> l)
    {
        std::cout << "initializer_list constructor" << std::endl;
    }

    my_constructor(int i)
    {
        std::cout << "int constructor" << std::endl;
    }
};

} // namespace cpp4 

void show_constructors_precedence()
{
    // empty list or default?
    cpp4::my_constructor m1 {};
    // default have precedence

    // one-element list or int?
    cpp4::my_constructor m2 { 1 };
    // initializer_list have precedence

    // two-element list
    cpp4::my_constructor m3 { 1, 2 };
    // initializer_list explicitly

    // {} is immutable, does not have move-ctor, does not have operator[]
}

//4. Delegating Constructors(17.4.3)
namespace cpp4
{

// so that not to call some common init code in constructor 
//in C++11 delegating (forwarding) constructors are present
class my_delegate
{
public:

    // non-default
    my_delegate(int i) : i_ { i } {}

    // use delegate in default constructor
    my_delegate() : my_delegate { 42 } {}

private:
    int i_ = 0;
};

} // namespace cpp4 

void show_delegate_constructor()
{
    cpp4::my_delegate d1;
    cpp4::my_delegate d2 { 666 };
}

//5. static member init(17.4.5)
namespace cpp4
{

// It is possible to initialize a static member in the class declaration. 
// The static member must be a const of an integral or enumeration type, 
// or a constexpr of a literal type (10.4.3), and the initializer must be a constant-expression
struct my_statics
{
    // all these statics ok in the class
    static const int i1 = 1;
    static const unsigned long l1 = 1ul;
    static const long long l2 = 1ll;

    static const int i2 = 1 + 2;

    // sqrt not constexpr
    // static const int i2 = std::sqrt(9);

    static constexpr std::complex<int> c1 { 1,2 };
};

} // namespace cpp4 

//6. Movable. operator= safety(17.5.1)
namespace cpp4
{
// Typically, a move cannot throw, whereas a copy might (because it may need to acquire a resource), 
// and a move is often more efficient than a copy. When you write a move operation,
// you should leave the source object in a valid but unspecified state

// Some types could be moved only (not copied)

// TODO:
// ~suppress copy&move
// their generation is deprecated, not banned
// declare non-const => delete const
class my_movable
{
public:

    // construction
    my_movable() : count_ {}, i_ { nullptr } {}
    my_movable(size_t count) : count_ { count }, i_ { new int[count] } {}

    ~my_movable()
    {
        delete[] i_;
    }

    // copy (throwable)
    my_movable(const my_movable& rhs) : count_ { rhs.count_ }, i_ { new int[rhs.count_] }
    {
        try {
            std::copy(rhs.i_, rhs.i_ + count_, i_);
        }
        catch (...) {
            delete[] i_;
            throw;
        }

    }

    // move (nothrow)
    my_movable(my_movable&& rhs) noexcept : count_ { rhs.count_ }, i_ {}
    {
        i_ = rhs.i_;
        rhs.count_ = 0;
        rhs.i_ = nullptr;
    }

    // Important! move-constructor and move-assign must be implemented both
    my_movable& operator=(my_movable&& rhs) noexcept
    {
        count_ = rhs.count_;
        i_ = rhs.i_;

        rhs.count_ = 0;
        rhs.i_ = nullptr;

        return *this;
    }

    // assign (throw)
    // Note that I did not protect copy assignment against self-assignment, m=m. 
    // The reason I did not test is that self-assignment of the members is already safe: 
    // both my implementations of copy assignment will work correctly and reasonably efficiently for m=m
    my_movable& operator=(const my_movable& rhs)
    {
        // create a copy
        my_movable tmp { rhs };

        // my: hmm, it is safe, but anyway not efficient
        // I would add != this check

        std::swap(tmp, *this);

        return *this;
    }

private:
    size_t count_;
    int* i_;
};


} // namespace cpp4 

void show_movable()
{
    cpp4::my_movable m1(10);
    cpp4::my_movable m2;
    cpp4::my_movable m3(m1);

    // copy-swap trick, based on std::move
    m2 = m1;
}

//7. Prevent slicing(17.5.1.4)
namespace cpp4
{

class base_sliced
{
public:
private:
    int b = 0;
};

class derived_sliced : public base_sliced
{
public:
private:
    int d = 0;
};

class base_private_protected
{
public:
    base_private_protected() {}
protected:
    // could be = default;
    base_private_protected& operator =(base_private_protected& rhs)
    {
        b = rhs.b;
        return *this;
    }
private:
    int b = 0;
};

class derived_private_protected : public base_private_protected
{
public:
private:
    int d = 0;
};

} // namespace cpp4 

void show_slicing_protection()
{
    // create slicing
    cpp4::base_sliced b1;
    cpp4::derived_sliced d1;
    b1 = d1;

    //
    cpp4::base_private_protected b2;
    cpp4::derived_private_protected d2;
    cpp4::derived_private_protected d3;

    // preventing slicing
    // error: cannot access protected member declared in class 'cpp4::base_private_protected'
    // b2 = d2;
    d2 = d3;
}

//8. Using Default Operations(17.6.3)
namespace cpp4
{

class no_move
{
public:

    // all operations by default
    no_move() = default;
    ~no_move() = default;

    // also could be =default, implemented for debugging
    no_move(const no_move& rhs) :i_ { rhs.i_ }
    {
        std::cout << "no_move(const no_move&)" << std::endl;
    }
    no_move& operator=(const no_move& rhs)
    {
        std::cout << "no_move& operator=(const no_move& rhs)" << std::endl;
        i_ = rhs.i_;
        return *this;
    }

    // move operations do not defined so they are not generated
private:
    int i_ = 0;
};

} // namespace cpp4 

void show_default()
{
    // these 
    cpp4::no_move m1;
    cpp4::no_move m2;

    // explicitly called lvalue copy and assign
    // (move-operations don't generated)
    std::swap(m1, m2);
}

// 9. Deleted functions (17.6.4)
namespace cpp4
{

// delete template specialization
template <typename T>
T* clone(T* p)
{
    return new T { *p };
}

// don't clone int
int* clone(int*) = delete;

// delete unnecessary conversion
struct restrict_conversion
{
    explicit restrict_conversion(int i) : i_ { i } {}

    // don't try co convert double->int
    restrict_conversion(double) = delete;

    int i_ = 0;
};

// allocation control
struct heap_allocated
{
    // don't call destructor 
    ~heap_allocated() = delete;

    // call destroy() instaed
    void destroy() {}
};

struct stack_allocated
{
    // could be created only on stack
    void* operator new(size_t) = delete;
};

} // namespace cpp4 

void show_deleted()
{
    int a = 0;

    // error C2280: 'int *cpp4::clone(int *)': attempting to reference a deleted function
    // cpp4::clone(&a);
}

// See more examples with =delete in Ch18

int main()
{
    cpp4::show_memberwise_init();
    show_explicit_destroy();
    show_constructors_precedence();
    show_delegate_constructor();
    show_movable();
    show_slicing_protection();
    show_default();

    return 0;
}

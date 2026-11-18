#include <iostream>
#include <cmath>
#include <complex>
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>

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
namespace cpp
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
    cpp::opened op = { 1, 1.0 };

    // members are closed, does not work even for friend
    //cpp::closed cl { 1, 1.0 };
}

} // namespace cpp

//2. Explicit destroy(17.2.4)
namespace cpp
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
        // error C2280: 'void *cpp::prevent_destruction_delete::__delDtor(unsigned int)': attempting to reference a deleted function
        // this->~prevent_destruction_delete();

        // do some manual removal stuff instead
    }

    ~prevent_destruction_delete() = delete;
private:
    int i = 0;
};

} // namespace cpp

void show_explicit_destroy()
{
    // we can't create automatic object as destructor is private
    // error C2248: 'cpp::prevent_destruction_private::~prevent_destruction_private':
    // cannot access private member declared in class 'cpp::prevent_destruction_private'
    // cpp::prevent_destruction_private p1;

    // free object is ok
    cpp::prevent_destruction_private* p2 = new cpp::prevent_destruction_private;

    // we can't create automatic object if destructor is deleted
    // error C2280: 'cpp::prevent_destruction_delete::~prevent_destruction_delete(void)':
    // attempting to reference a deleted function
    // cpp::prevent_destruction_delete p3;

    cpp::prevent_destruction_delete* p4 = new cpp::prevent_destruction_delete;

    // can't delete automatic object this way
    // p1.destroy();

    p2->destroy();

    // // can't delete automatic object this way
    // p3.destroy();

    p4->destroy();
}

//3. initializer_list Constructor Disambiguation(17.3.4.1)
namespace cpp
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

} // namespace cpp

void show_constructors_precedence()
{
    // empty list or default?
    cpp::my_constructor m1 {};
    // default have precedence

    // one-element list or int?
    cpp::my_constructor m2 { 1 };
    // initializer_list have precedence

    // two-element list
    cpp::my_constructor m3 { 1, 2 };
    // initializer_list explicitly

    // {} is immutable, does not have move-ctor, does not have operator[]
}

//4. Delegating Constructors(17.4.3)
namespace cpp
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

} // namespace cpp

void show_delegate_constructor()
{
    cpp::my_delegate d1;
    cpp::my_delegate d2 { 666 };
}

// A static member of integral/enumeration type (static const) or literal
// type (static constexpr) may be initialized directly in the class body,
// as long as the initializer is itself a constant expression -- see
// my_statics in the README; not repeated here since it's pure syntax with
// nothing to actually exercise at runtime.

//6. Movable. operator= safety(17.5.1)
namespace cpp
{
// Typically, a move cannot throw, whereas a copy might (because it may need to acquire a resource), 
// and a move is often more efficient than a copy. When you write a move operation,
// you should leave the source object in a valid but unspecified state
// Some types could be moved only (not copied)

// TODO:
// ~suppress copy&move
// declaring non-const member causes delete const version
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


} // namespace cpp

void show_movable()
{
    cpp::my_movable m1(10);
    cpp::my_movable m2;
    cpp::my_movable m3(m1);

    // copy-swap trick, based on std::move
    m2 = m1;
}

//7. Prevent slicing(17.5.1.4)
namespace cpp
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

} // namespace cpp

void show_slicing_protection()
{
    // create slicing
    cpp::base_sliced b1;
    cpp::derived_sliced d1;
    b1 = d1;

    //
    cpp::base_private_protected b2;
    cpp::derived_private_protected d2;
    cpp::derived_private_protected d3;

    // preventing slicing
    // error: cannot access protected member declared in class 'cpp::base_private_protected'
    // b2 = d2;
    d2 = d3;
}

//8. Using Default Operations(17.6.3)
namespace cpp
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

} // namespace cpp

void show_default()
{
    // these 
    cpp::no_move m1;
    cpp::no_move m2;

    // explicitly called lvalue copy and assign
    // (move-operations don't generated)
    std::swap(m1, m2);
}

// 9. Deleted functions (17.6.4)
namespace cpp
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

    // call destroy() instead
    void destroy() {}
};

struct stack_allocated
{
    // could be created only on stack
    void* operator new(size_t) = delete;
};

} // namespace cpp

void show_deleted()
{
    int a = 0;

    // error C2280: 'int *cpp::clone(int *)': attempting to reference a deleted function
    // cpp::clone(&a);
}

// See more examples with =delete in Ch18

// 10. Rule of 0/3/5 suppression rules, proven at compile time and run time
//
// A user-declared destructor suppresses the implicit move constructor and
// move assignment outright -- they simply don't exist. It does NOT delete
// the implicit copy constructor; that is only *deprecated* (since C++11,
// still true through C++23 -- no standard has actually removed it, no
// matter how often that claim gets repeated).
//
// The deceptive consequence: std::is_move_constructible_v<T> can be true
// for a T with no move constructor at all, because the surviving copy
// constructor can bind an rvalue too. The trait answers "can T be
// constructed from a T&&", not "will that actually move anything".
namespace cpp
{

struct counting_member
{
    static inline int copies = 0;
    static inline int moves = 0;

    counting_member() = default;
    counting_member(const counting_member&) { ++copies; }
    counting_member(counting_member&&) noexcept { ++moves; }
};

struct rule_of_0
{
    counting_member member;
};

// The ONLY special member declared here is the destructor.
struct declares_destructor_only
{
    ~declares_destructor_only() {}
    counting_member member;
};

static_assert(std::is_move_constructible_v<rule_of_0>);
static_assert(std::is_move_constructible_v<declares_destructor_only>); // true, but misleadingly so

} // namespace cpp

void show_destructor_suppresses_move_not_copy()
{
    cpp::counting_member::copies = 0;
    cpp::counting_member::moves = 0;

    cpp::rule_of_0 a;
    cpp::rule_of_0 b(std::move(a));
    // No user-declared special members at all: the implicit move
    // constructor is generated normally, and actually moves.
    assert(cpp::counting_member::moves == 1);
    assert(cpp::counting_member::copies == 0);

    cpp::counting_member::moves = 0;
    cpp::counting_member::copies = 0;

    cpp::declares_destructor_only c;
    cpp::declares_destructor_only d(std::move(c));
    // is_move_constructible_v said this type is move-constructible.
    // It compiles. It runs. It is, in fact, a copy -- std::move(c) binds
    // to the (deprecated, still-generated) copy constructor, because no
    // move constructor exists to prefer instead.
    assert(cpp::counting_member::copies == 1);
    assert(cpp::counting_member::moves == 0);
}

// 11. noexcept on the move constructor changes what std::vector actually
// does on reallocation, not just what it's "allowed" to do
namespace cpp
{

struct throwing_move_element
{
    static inline int copies = 0;
    static inline int moves = 0;

    throwing_move_element() = default;
    throwing_move_element(const throwing_move_element&) { ++copies; }
    throwing_move_element(throwing_move_element&&) /* not noexcept */ { ++moves; }
};

struct nothrow_move_element
{
    static inline int copies = 0;
    static inline int moves = 0;

    nothrow_move_element() = default;
    nothrow_move_element(const nothrow_move_element&) { ++copies; }
    nothrow_move_element(nothrow_move_element&&) noexcept { ++moves; }
};

} // namespace cpp

void show_noexcept_move_changes_vector_growth_strategy()
{
    {
        std::vector<cpp::throwing_move_element> v;
        v.reserve(1);
        for (int i = 0; i < 8; ++i)
            v.emplace_back();
        // vector uses std::move_if_noexcept internally: since this
        // element's move constructor is not noexcept (and it HAS an
        // accessible copy constructor to fall back on), reallocation
        // copies the existing elements into the new buffer instead of
        // moving them -- a throwing move mid-reallocation would leave
        // vector unable to honor its exception-safety guarantee.
        assert(cpp::throwing_move_element::copies > 0);
    }
    {
        std::vector<cpp::nothrow_move_element> v;
        v.reserve(1);
        for (int i = 0; i < 8; ++i)
            v.emplace_back();
        // A noexcept move constructor can't throw partway through, so
        // move_if_noexcept actually moves.
        assert(cpp::nothrow_move_element::copies == 0);
        assert(cpp::nothrow_move_element::moves > 0);
    }
}

int main()
{
    cpp::show_memberwise_init();
    show_explicit_destroy();
    show_constructors_precedence();
    show_delegate_constructor();
    show_movable();
    show_slicing_protection();
    show_default();
    show_destructor_suppresses_move_not_copy();
    show_noexcept_move_changes_vector_growth_strategy();

    return 0;
}

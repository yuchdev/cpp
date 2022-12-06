#include <iostream>
#include <typeinfo>
#include <utility>
#include <type_traits>

using namespace std;

/*

New features:
* Variadic templates (28.6)
* Tuples (28.5)
* Predicates


Examples:
1. Predicates (is_polymorphic, underlying). Type aliases, constexpr func wrapper (28.2)
2. Select (28.2)
3. conditional and holder (28.2)
4. Traits and decltype (28.2.4)
5. enable_if declaration (28.4)
6. SFINAE and enable_if (28.4.4)
7. type-safe print (28.6.1)
*/

//1. Predicates(is_polymorphic, underlying).Type aliases, constexpr func wrapper(28.2)
//2. Select(28.2)
namespace cpp4
{


// select types from list by index
template <size_t N, typename... Cases>
struct select_n;

template <size_t N, typename T, typename... Cases>
struct select_n<N, T, Cases...> : select_n<N - 1, Cases...> {};

template <typename T, typename... Cases>
struct select_n<0, T, Cases...>
{
    using type = T;
};

template<unsigned N, typename... Cases>
using select = typename select_n<N, Cases...>::type;

// get integer of desired size
template <size_t N>
struct sized_integer
{
    using error_t = void;
    using type = cpp4::select<N, error_t, signed char, error_t, error_t, long, error_t, error_t, long long>;
};

} // namespace cpp4 

void show_predicates()
{

    // some enums
    enum class Axis : char { x, y, z };
    enum flags { off, x = 1, y = x << 1, z = x << 2, t = x << 3 };

    // get underlying types
    std::cout << typeid(typename std::underlying_type<Axis>::type).name() << std::endl;
    std::cout << typeid(typename std::underlying_type<flags>::type).name() << std::endl;

    // check polymorphic
    if (is_polymorphic<int>::value)
        cout << "Big surprise!";

    // show select and desired integer type
    typename cpp4::sized_integer<1>::type x1 = 0;
    typename cpp4::sized_integer<4>::type y1 = 0;

    std::cout << typeid(x1).name() << std::endl;
    std::cout << typeid(y1).name() << std::endl;
}


//3. conditional and holder(28.2)
namespace cpp4
{

// let this object placed on the heap
template <typename T>
class heap_placement {};

// let this object placed on the stack
template <typename T>
class stack_placement {};

// Is the object big?
template <typename T>
constexpr bool is_big_object()
{
    return (sizeof(T) > 1024);
}

// We decide where to place obj
template <typename T>
struct conditional_placement
{
    using type = std::conditional< is_big_object<T>(), heap_placement<T>, stack_placement<T> >;
    type t;
};

// Compile time selection (stack or heap?) example is not hypothetical. For example, the C++ standard contains 
// the following comment in its definition of the function type

} // namespace cpp4 

//4. Traits and decltype (28.2.4)
// http://en.cppreference.com/w/cpp/memory/allocator_traits
// create traits for own allocator: pointer types, whether allocator copyable etc

// http://en.cppreference.com/w/cpp/string/char_traits
// finding and comparator rules, assign, copy and move, conversions

// http://en.cppreference.com/w/cpp/iterator/iterator_traits
// pointers and reference types

// http://en.cppreference.com/w/cpp/regex/regex_traits
// TODO?

// http://en.cppreference.com/w/cpp/memory/pointer_traits
// element type and difference type

// ? time_traits, type_traits
void show_traits() {}

//5. enable_if declaration(28.4)
namespace cpp4
{

template <typename T>
class simple_ptr
{
public:
    simple_ptr() :_ptr {} {}

    explicit simple_ptr(T* mem) :_ptr { mem } {}

    ~simple_ptr()
    {
        delete _ptr;
    }

    // enable -> only for classes
    // You may consider this exotic, but providing (defining) operations conditionally is very common
    // The standard library provides many examples of conditional definition, such as Alloc::size_type 
    // and pair being movable if both of their elements are
    std::enable_if_t<std::is_class<T>::value, T*> operator->() const
    {
        return _ptr;
    };

    T& operator*() const
    {
        return *_ptr;
    };

    T* get() const
    {
        return _ptr;
    };

    T* release()
    {
        T* ptr = _ptr;
        _ptr = 0;
        return ptr;
    };

private:
    T* _ptr;
};

struct example_class
{
    int i = 0;
};

} // namespace cpp4 

void show_enable_if()
{
    cpp4::simple_ptr<cpp4::example_class> p1 { new cpp4::example_class };
    std::cout << p1->i << std::endl;

    // does not instantiated (?)
    //cpp4::simple_ptr<int> p2{ new int{42} };
    //p2->
}
#if 0
//6. SFINAE and enable_if(28.4.4)
namespace cpp4
{

// question 'Can we call f(x) if x is of type X?' 
// Defining has_f to answer that question gives an opportunity to demonstrate some of the techniques used

// represent a failure to declare something
struct substitution_failure {};

template<typename T>
struct substitution_succeeded : std::true_type {};

// specialization for the fail condition
template<>
struct substitution_succeeded<substitution_failure> : std::false_type {};

// Answer the question
template<typename T>
struct get_f_result
{
public:

    // 'std::declval' converts any type T to a reference type, 
    // making it possible to use member functions in decltype expressions without the need to go through constructors
    using type = decltype(check(std::declval<T>()));
private:

    // can call f(x)
    template<typename X>
    static auto check(X const& x) -> decltype(f(x));

    // cannot call f(x)
    static substitution_failure check(...);
};

template<typename T>
struct has_f : substitution_succeeded<typename get_f_result<T>::type> {};

// X<T> has a member use_f() if and only if f(t) can be called for a T value t
template<typename T>
class X
{
    // ...
    std::enable_if_t<has_f<T>()> use_f(const T&)
    {
        // ... f(t); // ...
    }
    // ... 
};

} // namespace cpp4 
#endif

//7. type - safe print(28.6.1)
namespace cpp4
{
} // namespace cpp4 



int main()
{
    show_predicates();
    //show_traits();
    show_enable_if();
    return 0;
}

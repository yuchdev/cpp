#include <iostream>
#include <typeinfo>
#include <memory>
#include <vector>

using namespace std;


/*

New features:
* Local classes as template arguments (25.2.1)
* Default template arguments for function templates (25.2.5.1)
* More explicit control of template instantiation: extern templates (26.2.2)

Examples:
1. Local classes as template arguments
2. Default template arguments for function templates
3. More explicit control of template instantiation: extern templates
*/


// 1. Local classes as template arguments
// 2. Default template arguments for function templates
namespace cpp4
{

template <typename T>
struct S
{
    T member;
};

// functions also may have default tmpl args
template <typename T = int, typename Cont = std::vector<T>>
void container()
{
    Cont v { 1,2,3,4,5 };
    std::cout << *v.begin() << std::endl;
}

} // namespace cpp4 

void show_local_and_defaults()
{
    // now local classes are available for template params
    class my_local
    {
        int i = 0;
    };

    cpp4::S<my_local> l;

    // if all function tmpl args are defaulted
    // it is called like a plain function
    cpp4::container();
}

// 3. More explicit control of template instantiation: extern templates
// http://stackoverflow.com/questions/8130602/using-extern-template-c0x
// You should only use extern template to force the compiler to not instantiate a template 
// when you know that it will be instantiated somewhere else, it used to reduce compile time and reduce object file size
// It could be useful for huge template functions

/*

New features:
* Type and template aliases (23.6)
* Simpler and more general SFINAE rule (23.5.3.2)
* Reference forwarding (23.5.2.1)

Examples:
1. Nested class in templates (23.4.6.3)
2. Template friends (23.4.7)
3. Reference deduction and forwarding (23.5.2.1)
4. Argument Substitution Failure (23.5.3.2)

*/

//1. Nested class in templates(23.4.6.3)
namespace cpp4
{

template <typename T>
class list
{
public:

    void insert(const T& t)
    {
        link* new_link = new link { t, tail, nullptr };
        if (tail) {
            tail->next = new_link;
            tail = new_link;
        }
        else {
            head = new_link;
            tail = new_link;
        }
    }

    // ... other methods

    ~list()
    {
        link* l = head;
        while (l) {
            link* n = l->next;
            delete l;
            l = n;
        }
    }

private:

    // Nested class in template KNOWS about template params
    // even it is not template itself
    // However it is recommended to do it dependent like <T>
    struct link
    {
        T val;
        link* prev;
        link* next;
    };

    link* head = nullptr;
    link* tail = nullptr;
};


} // namespace cpp4 

void show_nested_class()
{
    cpp4::list<int> l1;
    l1.insert(1);
}


//2. Template friends(23.4.7)
namespace cpp4
{

// Assume we have a vector and matrix that could be multiplied
// <> notation set the friend as a template function

/*
template <typename T>
class matrix {
public:
    // friends could be dependent on T arg
    friend vector operator*<>(const matrix<T>&, const vector<U>&);
};

template <typename T>
class vector {
public:
    friend vector operator*<>(const matrix<T>&, const vector<U>&);
};
*/

} // namespace cpp4 


//3. Reference deduction and forwarding(23.5.2.1)
namespace cpp4
{

// Due to reference collapse lvalue reference & have precedence
// & & = &
// && & = &
// & && = &
// && && = &&

// Universal references
// T&& t here is not just rvalue. It's an Universal Reference that turns 
// - T& if t is lvalue
// - T if t is rvalue
template <class T>
void func(T&& t)
{
    std::cout << typeid(t).name() << std::endl;
}

// Perfect forwarding is implemented this way
template<class T>
T&& forward(typename std::remove_reference<T>::type& t) noexcept
{
    return static_cast<T&&>(t);
}
// note: std::remove_reference instead of T&& is used for type-deduction

// it could be used this way:
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(cpp4::forward<Args>(args)...));
}
// Args&&... means here a number of universal references

struct example_class
{
    example_class(int a, int b) :a_ { a }, b_ { b } {}
    int a_ = 1;
    int b_ = 2;
};

} // namespace cpp4 

void show_reference_forwarding()
{
    // lvalues
    int i = 0;
    int& ri1 = i;
    const int& ri2 = i;

    // turns to int&
    cpp4::func(i);
    cpp4::func(ri1);
    cpp4::func(ri2);

    // rvalue
    // turns to int
    cpp4::func(1U);
    cpp4::func(1UL);

    // usage forwarding and universal references
    std::unique_ptr<cpp4::example_class> p = cpp4::make_unique<cpp4::example_class>(3, 4);
}


//4. Argument Substitution Failure(23.5.3.2)
namespace cpp4
{

// Let us have two template overloads
template<typename Iter>
typename Iter::value_type mean(Iter first, Iter last)
{
    auto it = first + (last - first) / 2;
    return *(it);
}

template<typename T>
T mean(T* first, T* last)
{
    int* m = first + (last - first) / 2;
    return *m;
}

} // namespace cpp4 

void show_sfinae_functions()
{
    std::vector<int> v { 1,2,3,4,5 };
    int a[] = { 1,2,3,4,5 };

    // works 1-st overload
    int m1 = cpp4::mean(v.begin(), v.end());

    // int* does not have ::value_type, but we just skip 1-st defenition
    // works 2-nd overload, use SFINAE
    int m2 = cpp4::mean(begin(a), end(a));
}


int main()
{
    show_nested_class();
    show_reference_forwarding();
    show_sfinae_functions();
    return 0;
}

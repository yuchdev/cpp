#pragma once

// In order to use a pointer or reference to a template, as well as for a class, 
// it is not necessary to define it completely - a forward declaration is sufficient
template <typename T> class C;

struct TemplatePreuse
{
    TemplatePreuse() :pc() {}
    C<int>* pc;
};

// Now define this class
template <typename T> class C
{
public:
    void func() {}
};

// However, in order to use an instance of a class or method, you already need its full definition
void show_defined()
{
    C<int> c;
    c.func();
}

// During template instantiation, the code is implemented only for used class members
// (so-called "lazy instantiation")
// The only exceptions are unnamed unions and virtual functions
// 
// As an example, let's implement a class template where almost all members are undefined
// Unless it was a template, the linker would return an error
// But since unused class members are not instantiated, there is no error
template <typename T>
struct Safe {};

template <int N>
struct Danger
{
    // If N<=0 we have an error
    char block[N];
};

template <typename T, int N>
class InstantByRequest
{
public:
    // virtual functions are always instantiated, we need a defenition
    virtual ~InstantByRequest()
    {

    }

    // Declaration only
    void no_body_this_function(Safe<T> s);

    // If this function was instantiated, we would've got an array of zero size (which is error)
    void inclass()
    {
        Danger<N> no_boom;
    }

    // operator-> is undefined, but there's no error
    T operator->();

    struct Nested
    {
        Danger<N> d;
    };

    union
    {
        int align;
        Safe<T> anonimous;
    };
};

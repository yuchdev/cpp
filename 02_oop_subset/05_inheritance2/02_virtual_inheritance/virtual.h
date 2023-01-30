#pragma once

// Demonstration of virtual multiple inheritance
enum lang
{
    cpp_lang = 0,
    java_lang = 1,
    php_lang = 2,
    lang_count
};

// Multiple inheritance is well managed in the case of abstract base class
// It is inherited virtually from this class,
// in other words, make it a virtual base class.
// Such a class is realized not by an object, but by a pointer to an object
// thus, it can occur in the hierarchy as many times as it takes
class code_generator_base
{
public:
    code_generator_base(int i);
    ~code_generator_base();

    // interface virtual method
    virtual void print() = 0;
    virtual void generate(lang l);
protected:
    // declaring data as protected indicates a design error
private:
    int _i;
};

// In RAM an object that includes a virtual base class represented like this

// cpp_code_generator:
// vptr
// pointer to code_generator_base

// java_code_generator:
// vptr
// pointer to code_generator_base

// php_code_generator:
// vptr
// pointer to code_generator_base

// code_formatter:
// vptr

// code_generator_base:
// vptr
// int

class cpp_code_generator : public virtual code_generator_base
{
public:
    explicit cpp_code_generator(int i);
    ~cpp_code_generator();

    // virtual method print() is overridden
    virtual void print() override;
    virtual void f1();
private:
};

class java_code_generator : public virtual code_generator_base
{
public:
    java_code_generator(int i);
    ~java_code_generator();
    virtual void f1();
    // virtual method print() is inherited
private:

};

class php_code_generator : public virtual code_generator_base
{
public:
    php_code_generator(int i);
    ~php_code_generator();
    virtual void f1();
    // virtual method print() is inherited
private:

};

// The downnost class in the hierarchy inherits non-virtually already
class code_formatter : public cpp_code_generator
    , public java_code_generator
    , public php_code_generator
{
public:
    code_formatter(int i);
    ~code_formatter();

    // The dominance principle is as follows:
    // the virtual method print() is inherited from all three base classes
    // but is executed only where it is explicitly overridden
    // warning C4250: 'code_formatter' : inherits 'cpp_code_generator::print' via dominance
    virtual void generate(lang l);

    // If single-valuedness occurs, it can be resolved with the using directive 
    // or explicitly override (`virtual void f1()`)
    using php_code_generator::f1;

protected:

private:
};


// Another hierarchy involving virtual and non-virtual inheritance
class Base
{
public:
    virtual void f_base() = 0;
};

// Inheritance is diamond-shaped, but one of the classes is inherited non-virtual
class Derived1 : public Base
{
    virtual void f1() = 0;
};

class Derived2 : virtual public Base
{
    virtual void f2() = 0;
};

class Final : public Derived1, public Derived2
{
public:
    virtual void f_base();
    virtual void f1();
    virtual void f2();
};

// An interesting pattern when a class inherits from an interface class publicly,
// and from the implementation privately

// If a method is overridden in several classes, it is considered ambiguous
// Even if the method is private!
// Changing the access modifier should not change the logic of the program

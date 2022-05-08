#pragma once

// Chapter 16.3 Nicolai Josuttis
// Curious Recursive Template Pattern (CRTP)

// This pattern works by passing a descendant class to one of the base classes as an argument
// Has lots of applications, but the simplest is counting instances of a certain type
template <typename T>
class object_counter
{
private:
    static size_t count;
public:
    object_counter() { ++object_counter<T>::count; }
    ~object_counter() { --object_counter<T>::count; }
    static size_t live() { return count; }
};
template <typename T>
size_t object_counter<T>::count = 0;

// count1 and count2 are completely different types, and inherit from different classes 
// (different instances of the same template)
class count1 : public object_counter< count1 > {};
class count2 : public object_counter< count2 > {};

// With CRTP we can implement static polymorphism
template <typename Derived>
class BasicWorker
{
public:

    void work()
    {
        // here is the common code for inherited classes
        get_derived().work(t);
    }

    const Derived& get_derived() const
    {
        static_cast<const Derived&>(this);
    }

    Derived& get_derived()
    {
        return static_cast<Derived&>(this);
    }

    // ... More methods
};

class SpecificWorker1 : public BasicWorker<SpecificWorker1>
{
public:

    int work() const
    {
        // implementation
        return 1;
    }
};

class SpecificWorker2 : public BasicWorker<SpecificWorker2>
{
public:

    int work() const
    {
        // implementation
        return 2;
    }
};

/*
When should we use CRTP? For example, there are two classes that don't have to be connected
by belonging to same hierarchy through a common base class, but they share common logic.
Static polymorphism allows this code not to be duplicated by writing it just once.
Pros: no call overhead through virtual function, no obstacles to inlining,
no increase in object size because of the VPTR table.
Cons: putting the template class code in the header, bloating the code.
However, in many cases it is even desirable to inline small functions,
so for small types the duplication of "base classes" is insignificant.
*/

#pragma once
#include <typeinfo>
#include <limits>
namespace std
{

// Just like iterator, memory allocator is a pure abstraction
// A class that supports minimal STL allocator interface
template <typename T>
class custom_allocator
{
public:
    // define types-properties
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

    // Support for a different type of allocator
    // This can be needed if the data structure stores not only the data,
        //  but some service information (decks, hash tables, etc.)
    // allocate memory for it
    template <typename U>
    struct rebind
    {
        typedef custom_allocator<U> other;
    };

    // Allocator has no state, so constructors and destructor are trivial
    custom_allocator() throw()
    {
        cout << "Type: " << typeid(T).name() << endl;
        cout << "Ref: " << typeid(reference).name() << endl;
        cout << "Ptr: " << typeid(pointer).name() << endl;
    }
    custom_allocator(const custom_allocator&) throw() {}
    ~custom_allocator() throw() {}

    // constructor for service types
    // for example, for list nodes, dec arrays, hash tables
    template <typename U>
    custom_allocator(const custom_allocator<U>&) throw() {}

    // address in RAM
    pointer address(reference val)
    {
        return &val;
    }

    const_pointer address(const_reference val)
    {
        return &val;
    }

    // supported number of elements
    size_type max_size() const throw()
    {
        return numeric_limits<size_t>::max() / sizeof(T);
    }

    // allocate memory for n elements without initialization
    // Note, elements are not being constructed here
    // the meaning of `hint` depends on the implementation
    // can be used when overloading `operator new()`, as a pointer to the previous memory block
    pointer allocate(size_type num, typename custom_allocator<T>::const_pointer hint = nullptr)
    {
        return reinterpret_cast<pointer>(::operator new(num * sizeof(T)));
    }

    // initialize one element of the selected block with the value val
    void construct(pointer p, const T& val)
    {
        new (reinterpret_cast<void*>(p)) T(val);
    }

    // remove a single element
    void destroy(pointer p)
    {
        // the case when you need explicit destructor call
        p->~T();
    }

    // remove the block itself
    // by this point, the item must be destroyed
    void deallocate(pointer p, size_type)
    {
        // in general case, `deallocate` does not give guarantees when passing nullptr
        ::operator delete(reinterpret_cast<void*>(p));
    }
private:
};

// void specialization is a must
template <>
class custom_allocator<void>
{
public:
    // define types-properties
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef void value_type;
    typedef void* pointer;
    typedef const void* const_pointer;

    template <typename U>
    struct rebind
    {
        typedef custom_allocator<U> other;
    };
};

// template `operator==` tells us that that memory can be allocated by one and destroyed by another
template <typename T1, typename T2>
bool operator==(const custom_allocator<T1>&, const custom_allocator<T2>&) throw()
{
    return true;
}

template <typename T1, typename T2>
bool operator!=(const custom_allocator<T1>&, const custom_allocator<T2>&) throw()
{
    return false;
}


} //namespace std

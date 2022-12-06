#pragma once

#include <iostream>
#include <cassert>

void print_debug(const char* s)
{
    std::cout << s << std::endl;
}

// The class implements a straightforward dynamic array of vector type with index access
// and matrix arithmetic operations
template <typename T>
class SArray
{
public:

    // explicit constructor
    explicit SArray(size_t s) :_storage(new T[s]), _storage_size(s)
    {
        init();
    }

    // deep copy
    SArray(SArray<T> const& rhs)
        :_storage(new T[rhs._storage_size])
        , _storage_size(rhs._storage_size)
    {

        copy(rhs);
    }

    // release the memory
    ~SArray()
    {
        destroy();
    }

    // deep copy
    SArray<T>& operator=(SArray<T> const& rhs)
    {
        if (&rhs != this) {
            copy(rhs);
        }
        return *this;
    }

    // accessors
    T operator[](size_t i) const { return _storage[i]; }

    T& operator[](size_t i) { return _storage[i]; }

    inline size_t size() const { return _storage_size; }
protected:

    // Zero init
    void init()
    {
        print_debug("init");
        for (size_t i = 0; i < size(); ++i) {
            _storage[i] = T();
        }
    }

    // Copy from another array
    void copy(SArray<T> const& rhs)
    {
        print_debug("copy");
        assert(size() == rhs.size());
        for (size_t i = 0; i < size(); ++i) {
            _storage[i] = rhs[i];
        }
    }

    // Ceanup
    void destroy()
    {
        print_debug("destroy");
        delete[] _storage;
    }

private:
    T* _storage;
    size_t _storage_size;
};

// Let's implement overloaded vector arithmetic functions
// We do not care about the performance here

// Vector addition (value return)
template <typename T>
SArray<T> operator+(SArray<T> const& a, SArray<T> const& b)
{

    assert(a.size() == b.size());
    SArray<T> result(a.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = a[i] + b[i];
    }
    return result;
}

// Vector multiplication (return by value)
template <typename T>
SArray<T> operator*(SArray<T> const& a, SArray<T> const& b)
{

    assert(a.size() == b.size());
    SArray<T> result(a.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = a[i] + b[i];
    }
    return result;
}

// Vector multiplication by a scalar (return by value)
template <typename T>
SArray<T> operator*(T const& a, SArray<T> const& b)
{

    SArray<T> result(b.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = a * b[i];
    }
    return result;
}

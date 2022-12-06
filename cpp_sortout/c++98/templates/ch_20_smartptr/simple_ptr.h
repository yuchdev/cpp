#pragma once

// a simple smart pointer without copying and passing the lead
// An analog to scoped_ptr
template <typename T>
class simple_ptr
{
public:

    // Trivial constructor
    simple_ptr() :_ptr() {}

    // Aquire memory ownership
    explicit simple_ptr(T* mem) :_ptr(mem) {}

    // Release
    ~simple_ptr()
    {
        delete _ptr;
    }

    // Accessor
    T* operator->() const
    {
        return _ptr;
    };

    T& operator*() const
    {
        return *_ptr;
    };

    // Functional form of operator ->
    T* get() const
    {
        return _ptr;
    };

    // Release from ownership
    T* release()
    {
        T* ptr = _ptr;
        _ptr = 0;
        return ptr;
    };

private:
    T* _ptr;
};

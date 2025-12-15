#pragma once
#include <new>

// Mixture class that replaces standard `new_handler` definition
template <typename T>
class NewHandlerSupport
{
public:
    static std::new_handler set_new_handler(std::new_handler p);
    static void* operator new(size_t size);
    static void operator delete(void* p);
    static void no_more_memory();
private:
    static std::new_handler _current_handler;
};

template <typename T>
std::new_handler NewHandlerSupport<T>::_current_handler = &NewHandlerSupport<T>::no_more_memory;

template <typename T>
std::new_handler NewHandlerSupport<T>::set_new_handler(std::new_handler p)
{
    // behaves like standard set_new_handler
    std::new_handler old_handler = _current_handler;
    _current_handler = p;
    return old_handler;
}


template <typename T>
void NewHandlerSupport<T>::no_more_memory()
{
    // in our custom new_handler we simply display the message
    // and then proceed as a standard new_handler
    std::cout << "No more memory!\n";
    throw std::bad_alloc();
}


template <typename T>
void* NewHandlerSupport<T>::operator new(size_t sz)
{
    // receive the standards new_handler (if nullptr throw bad_alloc)
    std::new_handler global_handler = std::set_new_handler(_current_handler);
    void* mem = nullptr;

    try {
        // allocate memory using global `operator new()`
        // if we can't allocate, _current_handler() will be called
        // if _current_handler == nullptr, bad_alloc will be thrown
        mem = ::operator new(sz);
    }
    catch (const std::bad_alloc&) {
        // put the old global new_handler back
        std::set_new_handler(global_handler);
        throw;
    }
    set_new_handler(global_handler);
    return mem;
}


template <typename T>
void NewHandlerSupport<T>::operator delete(void* p)
{
    ::operator delete(p);
}

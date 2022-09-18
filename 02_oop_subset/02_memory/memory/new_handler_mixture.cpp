#include "new_handler_mixture.h"

template <typename T>
new_handler NewHandlerSupport<T>::_current_handler = &NewHandlerSupport<T>::no_more_memory;

template <typename T>
new_handler NewHandlerSupport<T>::set_new_handler(new_handler p)
{
    // behaves like standard set_new_handler
    new_handler old_handler = _current_handler;
    _current_handler = p;
    return old_handler;
}


template <typename T>
void NewHandlerSupport<T>::no_more_memory()
{
    // in our custom new_handler we simply dispray the message
    // and then proceed as a standard new_handler
    std::cout << "No more memory!\n";
    throw bad_alloc();
}


template <typename T>
void* NewHandlerSupport<T>::operator new(size_t sz)
{
    // receive the standards new_handler (if nullptr throw bad_alloc)
    new_handler global_handler = std::set_new_handler(_current_handler);
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

#pragma once


// Mixtury class that replaces standard `new_handler` defenition
template <typename T>
class NewHandlerSupport
{
public:
    static new_handler set_new_handler(new_handler p);
    static void* operator new(size_t size);
    static void operator delete(void* p);
    static void no_more_memory();
private:
    static new_handler _current_handler;
};

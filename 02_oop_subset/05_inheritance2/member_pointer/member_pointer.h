#pragma once

// Demonstration of pointers to a class member

class member_pointer
{
public:
    virtual ~member_pointer() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void suspend() = 0;
    virtual void resume() = 0;
};

class member_derived : public member_pointer
{
public:
    member_derived() {}
    virtual ~member_derived() {}

    virtual void start();
    virtual void stop();
    virtual void suspend();
    virtual void resume();

    static void static_init();

    // new and delete operators are implicitly static class members
    //void* operator new(size_t sz);
    //void* operator new[](size_t sz);
    //void operator delete(void* mem, size_t sz);
    //void operator delete[](void* mem, size_t sz);
};

// Pointer to a class member
typedef void(member_pointer::* p_mem)();

// Pointer to a static member
typedef void(*static_member_pointer)();

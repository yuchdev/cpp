#include <cassert>
#include <iostream>
#include <new>

struct X
{
    explicit X(bool should_throw)
    {
        if (should_throw)
            throw std::runtime_error("X object never be created");
    }

    // custom placement new
    static void* operator new(std::size_t sz, bool b)
    {
        std::cout << "custom placement new called, b = " << b << '\n';
        return ::operator new(sz);
    }

    // custom placement delete -- reachable automatically ONLY if the
    // matching placement new above succeeded but the constructor then
    // threw; otherwise it is dead code unless called explicitly.
    static void operator delete(void* ptr, bool b)
    {
        std::cout << "custom placement delete called, b = " << b << '\n';
        ::operator delete(ptr);
    }
};

// Proves the pairing rule: placement new runs, the constructor throws
// before X* can be assigned to anything, and the language automatically
// unwinds the partial allocation by calling the MATCHING placement
// operator delete(void*, bool) -- never the plain, non-placement one.
void show_placement_delete_runs_on_constructor_failure()
{
    bool reached_assignment = false;
    try {
        X* p1 = new (true) X(true);
        reached_assignment = true;
        (void)p1;
    }
    catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
    assert(!reached_assignment);
}

// The other half of the rule, which has no automatic trigger at all: a
// placement operator delete is reachable through ordinary `delete p`
// ONLY for the non-placement overload. To invoke THIS one outside of
// constructor unwinding, it must be called explicitly, in function-call
// syntax -- there is no `delete`-expression spelling for it.
void show_placement_delete_called_explicitly()
{
    void* raw = X::operator new(sizeof(X), false); // allocate only, no construction
    X::operator delete(raw, false);                // explicit call: the only other way to reach it
}

int main()
{
    show_placement_delete_runs_on_constructor_failure();
    show_placement_delete_called_explicitly();
    return 0;
}

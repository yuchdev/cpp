#include <cassert>
#include <cstdio>
#include <iostream>
#include <limits>
#include <new>

using namespace std;

// The forms of operator new():
// https://en.cppreference.com/w/cpp/memory/new/operator_new

void show_new1()
{
    int* i = new int(6);
    printf("%d\n", *i);
    delete i;
}

void show_new2()
{
    int* i = new int(12);
    printf("*i = %d\n", *i);

    // Placement new into memory we already own: *i's storage is reused
    // in place, not a fresh allocation.
    int* j = new(i) int(7);
    printf("*j = %d\n", *j);
    printf("*i = %d\n", *i); // same address as j: i and j alias the same int
    assert(i == j);
    assert(*i == 7);
    delete i; // fine here: scalar types have no destructor to skip
}

class MyClass
{
public:
    MyClass() : imember()
    {
        cout << "Construction MyClass." << this << endl;
    }

    ~MyClass()
    {
        imember = 0;
        cout << "Destructing MyClass." << this << endl;
    }
    int imember;
};

void show_new_delete()
{
    // Regular form: throws bad_alloc on failure.
    MyClass* fPtr = new MyClass;
    delete fPtr;

    // Placement new: construct into memory the caller already owns.
    // Nothing was allocated, so there is nothing to `delete` -- the
    // object must be destroyed with an explicit destructor call.
    char x[sizeof(MyClass)];
    MyClass* fPtr2 = new(x) MyClass;
    fPtr2->~MyClass();
    cout << "The address of x[0] is : " << static_cast<void*>(&x[0]) << endl;

    // nothrow form: failure reported by null instead of an exception.
    MyClass* fPtr3 = new(nothrow) MyClass;
    assert(fPtr3 != nullptr); // this allocation is tiny; it will succeed
    delete fPtr3;
}

// A naive "allocate something huge" test to provoke bad_alloc is NOT
// reliable on a modern 64-bit system: virtual memory overcommit means a
// multi-gigabyte request can succeed immediately (no physical memory is
// touched until the pages are actually written), only to fail later and
// unpredictably when the pages ARE touched -- or not fail at all if
// enough swap/overcommit headroom exists. Requesting something close to
// the theoretical maximum size_t sidesteps this: no system has that much
// addressable memory, overcommit or not, so this fails deterministically.
void allocate_impossible_amount()
{
    // NOT constexpr: a compile-time-constant array bound this large is
    // rejected by the compiler itself ("array is too large") before
    // runtime even enters the picture. A plain runtime variable defers
    // the size check to where this section is actually about: the
    // allocation function, at runtime.
    size_t n = std::numeric_limits<size_t>::max() / sizeof(long) - 1;
    long* l = new long[n];
    delete[] l; // unreachable: the line above always throws first
}

void no_more_memory_exit()
{
    cout << "no more memory: new_handler (would exit here)" << endl;
    // A real new_handler's contract is one of:
    // 1. free some memory and return (allocation is retried)
    // 2. install a different handler and return
    // 3. throw bad_alloc (or a derived type)
    // 4. terminate the program (abort()/exit())
    // Strategy 4 is the one this function's NAME describes, but calling
    // exit() for real would end this whole demo process -- see
    // show_new_handler_throws_instead() for a handler that exercises
    // strategy 3, which keeps the process alive so the rest of main()
    // still runs.
}

void no_more_memory_throws()
{
    cout << "no more memory: new_handler installs itself as throwing" << endl;
    throw bad_alloc(); // strategy 3: a handler is allowed to just throw
}

void show_bad_alloc()
{
    // No handler installed (the default: nullptr): failure goes straight
    // to bad_alloc.
    try {
        allocate_impossible_amount();
        assert(false); // must not reach here
    }
    catch (bad_alloc&) {
        cout << "no more memory: bad_alloc handler" << endl;
    }

    // A handler is invoked repeatedly until it does one of the four
    // things above; here it throws, which is caught exactly like the
    // no-handler case -- but note we never reach the handler-exits-the-
    // process strategy for real, since that would end this executable.
    new_handler old = set_new_handler(no_more_memory_throws);
    try {
        allocate_impossible_amount();
        assert(false);
    }
    catch (bad_alloc&) {
        cout << "caught bad_alloc after the installed handler threw" << endl;
    }
    set_new_handler(old); // restore: a process-wide handler must not leak
                           // into whichever demo runs next
}

int main()
{
    show_new1();
    show_new2();
    show_new_delete();
    show_bad_alloc();
    return 0;
}

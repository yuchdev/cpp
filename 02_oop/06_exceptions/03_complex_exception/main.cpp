#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace
{
    template <typename F>
    std::string captured_output(F&& f)
    {
        std::ostringstream out;
        std::streambuf* old = std::cout.rdbuf(out.rdbuf());
        f();
        std::cout.rdbuf(old);
        return out.str();
    }
}

void complex_object_exception()
{

    // create a complex class with multiple inheritance
    // and ineternal classes and throw an exception from constructor
    class internal
    {
        int i;
    public:
        internal() : i() { cout << "internal()\n"; }
        ~internal() { cout << "~internal()\n"; }

    };

    class base
    {
    public:
        base() { cout << "base()\n"; }
        virtual ~base() { cout << "~base()\n"; }
    };

    class derived1 : public virtual base
    {
    public:
        derived1() { cout << "derived1()\n"; }
        ~derived1() { cout << "~derived1()\n"; }
    };

    class derived2 : public virtual base
    {
    public:
        derived2() { cout << "derived2()\n"; }
        ~derived2() { cout << "~derived2()\n"; }
    };

    class finalizer : public derived1, public derived2
    {
    public:
        internal _i;
        finalizer()
        {
            throw std::runtime_error("Exception from constructor");
        }
        ~finalizer() { cout << "~finalizer()\n"; }
    };

    bool caught = false;
    std::string log = captured_output([&] {
        try {
            finalizer f;
            (void)f;
        }
        catch (const std::exception& e) {
            // finalizer has not been created
            caught = true;
            cout << e.what();
        }
    });

    assert(caught);
    // Everything that successfully finished constructing is torn down in
    // exact reverse order -- base (the shared virtual base) first, then
    // derived1, then derived2, then the member `_i` -- then the
    // constructor throws. ~finalizer() never runs: finalizer itself
    // never finished coming into existence, so there is nothing to
    // destroy at that level.
    assert(log ==
        "base()\nderived1()\nderived2()\ninternal()\n"
        "~internal()\n~derived2()\n~derived1()\n~base()\n"
        "Exception from constructor");
}


// The function demonstrates private exception inheritance
class my_exception : std::exception
{ // private inheritance
public:
    // std::exception has no portable constructor taking a message (GCC and
    // MSVC both offer one as a non-standard extension; libc++ does not) --
    // the portable way to give an exception type its own message is to
    // store it and override what().
    const char* what() const noexcept override { return "my_exception"; }
};

void foo()
{
    throw my_exception();
}

void show_exception_private_inherit()
{
    enum class caught_as { none, base, derived };
    caught_as which = caught_as::none;

    try {
        foo();
    }
    // will not be caught here, even though the underlying exception
    // inheritance is private, and what() is closed
    catch (const std::exception&) {
        which = caught_as::base;
    }
    catch (const my_exception&) {
        which = caught_as::derived;
    }

    // Proof, not just a comment: private inheritance makes std::exception
    // an INACCESSIBLE base from outside my_exception, so the compiler
    // will not consider catch(const std::exception&) a match at all --
    // only the exact-type handler can catch it.
    assert(which == caught_as::derived);
}

int main()
{
    complex_object_exception();
    show_exception_private_inherit();
    return 0;
}

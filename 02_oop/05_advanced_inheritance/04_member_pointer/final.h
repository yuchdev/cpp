#pragma once

// The "no further derivation" idiom, as it had to be done before C++11
// gave us the `final` keyword. The trick: give the class a private
// VIRTUAL base whose constructor only the class itself may call.
//
// finalizer_lock's constructor is private, visible only to its one
// friend, finalizer. Normally that would be enough -- but finalizer_lock
// is inherited as a VIRTUAL base, and the language rule for virtual bases
// is that the MOST-DERIVED class in any hierarchy is the one responsible
// for constructing them directly, no matter how many classes sit in
// between. So if something derives from finalizer, the compiler needs
// THAT class's constructor to call finalizer_lock's constructor directly
// -- and it can't, because the friendship is only extended to finalizer,
// not to classes derived from it. See main.cpp for the resulting error.

class finalizer_lock
{
    friend class finalizer;
    finalizer_lock() {}
};

class finalizer : private virtual finalizer_lock
{
public:
    finalizer();
    void do_work() const;
};

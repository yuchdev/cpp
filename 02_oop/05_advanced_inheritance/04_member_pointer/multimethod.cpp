#include "multimethod.h"

#include <iostream>

using std::cout;
using std::endl;

base::~base()
{
}

// The (base&) overload only ever runs as the FIRST half of a double
// dispatch: it re-calls multiop on `other`, now passing `*this` with its
// real, derived_a-specific static type, which lets `other`'s own virtual
// dispatch pick the (derived_a&) or (derived_b&) overload instead of
// falling back to (base&) again.
void derived_a::multiop(base& other) { other.multiop(*this); }
void derived_a::multiop(derived_a& other) { cout << "derived_a::multiop(derived_a&)\n"; (void)other; }
void derived_a::multiop(derived_b& other) { cout << "derived_a::multiop(derived_b&)\n"; (void)other; }

void derived_b::multiop(base& other) { other.multiop(*this); }
void derived_b::multiop(derived_a& other) { cout << "derived_b::multiop(derived_a&)\n"; (void)other; }
void derived_b::multiop(derived_b& other) { cout << "derived_b::multiop(derived_b&)\n"; (void)other; }

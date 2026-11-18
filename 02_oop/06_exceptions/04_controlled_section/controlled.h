#pragma once
#include <vector>

class controlled
{
public:
    // should_fail defaults to true so a default-constructed `controlled`
    // demonstrates the throwing-constructor path by default, matching
    // this leaf's original intent; pass false to construct successfully
    // instead (needed to reach controlled_method() at all -- see below).
    explicit controlled(bool should_fail = true);
    ~controlled() throw();

    void controlled_method(bool should_fail = true) const;

private:
    std::vector<int> _vect;
};

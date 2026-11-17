#include "diamond.h"

#include <iostream>

using std::cout;
using std::endl;

base_v::base_v() : _tag(0)
{
    cout << "base_v()\n";
}

base_v::~base_v()
{
    cout << "~base_v()\n";
}

void base_v::identify() const
{
    cout << "base_v::identify\n";
}

bottom_v::bottom_v() : base_v()
{
    cout << "bottom_v()\n";
}

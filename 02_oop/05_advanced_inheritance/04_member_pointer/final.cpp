#include "final.h"

#include <iostream>

using std::cout;
using std::endl;

finalizer::finalizer()
{
    cout << "finalizer()\n";
}

void finalizer::do_work() const
{
    cout << "finalizer::do_work\n";
}

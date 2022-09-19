#include "abstract.h"
#include "classes.h"
#include "currency.h"
#include "adapter.h"
#include "virtual.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

/*
Upcast is conversion of pointer or reference of derived class type to pointer or reference of base class type, going up in the inheritance tree.
Upcasting is implicit in C++, and is used a lot when you deal with virtual dispatching.
In other words, you have a pointer to Base from which you can access the common interface of a whole hierarchy of classes,
and the selection can be done at runtime. This assumes that your interface functions are marked virtual

Downcast is the conversion of pointer or reference of base class type to pointer or reference type of its derived class, going down in the inheritance tree.
Downcasting is less useful, and should be avoided whenever one can.
In general is a sign of bad design, as one rarely needs to convert a Base object to a derived one.
It can be done (and the result checked) via dynamic_cast
*/

int main()
{
    return 0;
}

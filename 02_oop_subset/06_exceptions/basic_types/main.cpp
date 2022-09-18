#include <cmath>
#include <string>
#include <iostream>
// declaration of functions for floating point types
#include <cfloat>
#include <cfenv>
#include <vector>
#include <stdexcept>

#include "ex.h"
#include "controlled.h"
#include "finished.h"
#include "sehhandler.h"
#include "autoptr.h"
#include "zero_divide.h"
#include "structured_exception.h"

using std::vector;
using std::cout;
using std::endl;

// Throwing exceptions of base type
void show_base_type_ex(int b)
{

    // When throwing exceptions of base type
    // no implicit conversion takes place 
    // (double -> int, unsigned -> signed, etc.)
    if (b)
        throw int(1);
    else
        throw double(0.);
    // It is better not to throw exceptions of base type
    // and don't allocate memory for exceptions from the heap
    // e.g. `throw new int(1)`
}

void show_base_type()
{
    try {
        //show_base_type_ex(1);
        show_base_type_ex(0);
    }
    catch (const int e) {
        cout << "integer " << e << endl;
    }
    catch (const double e) {
        cout << "double " << e << endl;
    }
    // this handler catches all pointer exceptions
        // what to do here except of analyzing stack is unclear
        // we can not process p in any way, as its type is unknown
    catch (const void* p) {
    }

}

// Catch standard exception 
void show_out_of_range()
{
    vector<int> vi;
    vi.push_back(1);
    vi.push_back(1);
    vi.push_back(1);

    try {
        oor_exception_ex(vi);
    }
    catch (const std::exception& e) {
        cout << e.what() << endl;
    }
}

int main()
{
    show_base_type();
    show_simple_ex();
    return 0;
}

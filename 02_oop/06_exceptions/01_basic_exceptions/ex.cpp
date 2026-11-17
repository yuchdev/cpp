#include "ex.h"

void oor_exception_ex(const std::vector<int>& v)
{
    v.at(v.size()); // one past the end: guaranteed out_of_range, not UB
}

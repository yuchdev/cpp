#pragma once

#include <vector>

// Deliberately indexes one past the end with vector::at(), which throws
// std::out_of_range -- unlike operator[], which has no bounds check and
// would simply be undefined behavior here instead of a catchable error.
void oor_exception_ex(const std::vector<int>& v);

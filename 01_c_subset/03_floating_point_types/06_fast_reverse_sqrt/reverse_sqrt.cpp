#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <iomanip>
#include <bitset>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

#include <utilities/bitwise.h>
#include <utilities/generate.h>


// http://en.wikipedia.org/wiki/Fast_inverse_square_root
// return 1/sqrt(number)
float quick_rsqrt(float number)
{
	const float x2 = number * 0.5F;
	const float threehalfs = 1.5F;

	union {
		float f;
		uint32_t i;
	} conv  = { .f = number };
	conv.i  = 0x5f3759df - ( conv.i >> 1 );
	conv.f  *= threehalfs - ( x2 * conv.f * conv.f );
	return conv.f;
}


void benchmark()
{
    RandomReal<double> rr;
    std::vector<double> randoms = rr.gen(10000, 0.0, 10.0);
}

int main()
{
    // TODO: insert any function call
    return 0;
}

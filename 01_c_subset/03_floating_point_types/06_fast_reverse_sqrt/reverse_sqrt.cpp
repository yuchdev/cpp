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
#include <utilities/elapsed.h>


// http://en.wikipedia.org/wiki/Fast_inverse_square_root
// return 1/sqrt(number)
float quick_rev_sqrt(float number)
{
	const float x2 = number * 0.5F;
	const float three_halfs = 1.5F;

	union {
		float f;
		uint32_t i;
	} conv  = { .f = number };
	conv.i  = 0x5f3759df - ( conv.i >> 1 );
	conv.f  *= three_halfs - (x2 * conv.f * conv.f );
	return conv.f;
}


void benchmark()
{
    RandomReal<double> rr;
    std::vector<double> randoms;
    MeasureTime t;
    randoms = rr.generate(0.0, 10.0, 10000);
    long long elapsed = t.elapsed_mcsec()
    std::cout << "Elapsed time: " << elapsed << " microseconds" << std::endl;
}

int main()
{
    // TODO: insert any function call
    return 0;
}

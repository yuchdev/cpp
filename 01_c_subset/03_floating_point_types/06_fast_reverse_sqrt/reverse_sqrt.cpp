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
float reverse_sqrt(float number)
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
    RandomReal<double> random_real;
    std::vector<double> randoms;
    MeasureTime t;
    randoms = random_real.generate(0.0, 10.0, 10000);
    long long elapsed = t.elapsed_mcsec();
    std::cout << "Elapsed time: Create "
        << randoms.size() << " elements in "
        << elapsed << " microseconds" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <function>\n";
        return 1;
    }
    std::string func = argv[1];

    std::cout << "Current C++ version: " << __cplusplus << '\n';

    if (func == "reverse_sqrt") {
        float rev = reverse_sqrt(4.0);
        std::cout << "1/sqrt(4) = " << rev << '\n';
    }
    else if (func == "benchmark") {
        benchmark();
    }
    else {
        std::cerr << "Unknown function: " << func << '\n';
        return 1;
    }

    return 0;
}

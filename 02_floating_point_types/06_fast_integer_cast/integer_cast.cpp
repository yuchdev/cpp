#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <iomanip>
#include <bitset>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

#include <bitwise.h>
#include <generate.h>

/*
*  Significand is stored without whole part, which is 1.0 anyway
*  So we can set 1 and shift significand to the right.
*
*  CPU makes significand shift itself before any operation under floating point numbers
*  with different exponent value. To be exact, it moves significand of a smaller number,
*  until it matches significand of bigger number. Say, we want to add 36.72 and 10000.0
*  They could be written in a scientific notation as 3.672e1 and 1.0e5
*  CPU takes 3.672e1 and moves it 5-1=4 digits right, receiving 0.0003672e5
*  Now we can add both number and receive 1.0003672e5
*  Now imagine, if we keep moving 3672 to the right, eventually 2 and then 7 would be truncated,
*  and 36 is everything what left. But this is exactly the whole part of 36.72!
*  For 32-bit floating point value you shoud move it to the size of exponent (TODO: check)
* 
*  TODO:
*  However, with negative numbers this trick won't work.
*  In case of adding decimal numbers, we should 
*/

namespace{
union convertible {
    float f;
    int i;
};
}

void classic_ftol()
{
    // Converting floating point to integer
    // Using any type of cast, C++ or C style, we implicitly call C function ftol(),
    // which saves rounding mode, set rounding mode for the conversion, convert and restore rounding mode
    // Let's see some assembly representation
    // (cvttss2si == Convert with Truncation Scalar Single-Precision Floating-Point Value to Integer)
    float fp_single = 9.99f;
    int i = static_cast<int>(fp_single);

    // GCC 11
    // movss        xmm0, DWORD PTR fp_single[rip]
    // cvttss2si    eax, xmm0
    // mov          DWORD PTR i[rip], eax

    // Clang 12
    // cvttss2si    eax, dword ptr [fp_single]
    // mov          dword ptr[i1], eax

    double fp_double = 9.99;
    i = static_cast<int>(fp_double);
    
    // GCC 11
    //  movsd       xmm0, QWORD PTR fp_double[rip]
    // cvttsd2si    eax, xmm0
    // mov          DWORD PTR i[rip], eax

    // Clang 12
    // cvttsd2si    eax, qword ptr [fp_double]
    // mov          dword ptr[i2], eax

    // We can take advantage of this
    // these techniques are applicable to any high-performance application that mixes
    // floating-point and integer math on modern processors

    convertible f;
    f.f = 1.0f;
    for (size_t i = 0; i < 22; ++i) {
        f.i = f.i >> 1;
        std::cout << bitwise(f.i) << '\n';
    }
}

int fast_ftoi(float x)
{
    // alignment shift to change the bit representation of a floating-point number
    // until it's the same as an integer's bit representation,
    // and then we can just read it like a normal integer

    // This trick works for positive numbers,
    // but if you try to convert a negative number it will fail

    // the normalization step screws it up because now that we've
    // borrowed from the implicit 1 bit, it's no longer the most significant bit

    // We can get this 1 bit simply by multiplying our large number by 1.5. 1.5 in binary is
    // 1.1, and the first 1 becomes the implicit 1 bit, and the second becomes the
    // most significant bit of the mantissa

    // If you subtract the integer representation of our large, floating-point shift number
    // (in other words, treat its bits like an integer instead of a float) from the
    // integer representation of the number we just converted, it will remove all the
    // high bits properly for both types of numbers, making the bits equal zero for
    // positive values and filling them in with ones for negative values
    convertible magic;
    // 1 << 22 = 2^23 (e.g. convert float to integer)
    // 150 << 23 is 1.5 trick for negative conversion
    // For double the same masks are (1LL << 51) and (1075LL << 52)
    magic.i = (150 << 23) | (1 << 22);

    convertible c;
    c.f = x + magic.f;
    // Not to create 'if' condition for the negatives we use subtraction trick
    // it will remove all the high bits properly for both types of numbers(positive and negative),
    // making the bits equal zero for positive values and filling them in with ones for negative values.
    return c.i - magic.i;
}

int flol_debug(float x)
{
    std::cout << "\nConverting to int x = " << x << '\n';
    const int magic1 = (150 << 23);
    std::cout << "magic1 = " << bitwise(magic1) << '\n';

    const int magic2 = (1 << 22);
    std::cout << "magic2 = " << bitwise(magic2) << '\n';

    int magic = magic1 | magic2;
    std::cout << "magic =  " << bitwise(magic) << '\n';


    // append to the converted number
    // float representation of the magic integer mask
    std::cout << "x =  " << x << ", x2 = " << bitwise(x) << '\n';
    std::cout << "magic =  " << *(reinterpret_cast<float*>(&magic)) << ", magic2 = " << bitwise(magic) << '\n';
    std::cout << "x += magic" << '\n';
    x += *(reinterpret_cast<float*>(&magic));
    std::cout << "x  = " << x << " x2 = " << bitwise(x) << '\n';

    // subtract from integer representation of the converted number
    // magic int mask
    std::cout << "x = " << bitwise(*(reinterpret_cast<int*>(&x))) << '\n';
    std::cout << "magic = " << bitwise(magic) << '\n';
    int res = *(reinterpret_cast<int*>(&x)) - magic;

    std::cout << "x - magic = " << res << " res2 = " << bitwise(res) << '\n';
    return res;
}


long long fast_dtoll(double d) 
{

    union castable {
        double d;
        long long l;
    };

    castable magic;
    magic.l = (1075LL << 52) | (1LL << 51);

    castable c;
    c.d = d + magic.d;
    return c.l - magic.l;
}

template <typename T>
struct TPTraitsBase {};

template <>
struct TPTraitsBase<float>
{
    typedef int ret_type;
    typedef int mask_type;
    typedef float convert_type;
    static const mask_type mask = (150 << 23) | (1 << 22);

};

template <>
struct TPTraitsBase<double>
{
    typedef long long ret_type;
    typedef long long mask_type;
    typedef double convert_type;
    static const mask_type mask = (1075LL << 52) | (1LL << 51);
};

template <typename T>
struct TPTraits : TPTraitsBase<T>
{
    union castable
    {
        typename TPTraitsBase<T>::convert_type f;
        typename TPTraitsBase<T>::mask_type i;
    };
};


template <typename FP>
typename TPTraits<FP>::ret_type fast_fpconvert(FP f)
{
    typename TPTraits<FP>::castable magic;
    magic.i = TPTraits<FP>::mask;

    volatile typename TPTraits<FP>::castable ret;
    ret.f = f + magic.f;
    return ret.i - magic.i;
}

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

void test_fast_ftoi()
{
    float f = 1.0;
    int i = fast_fpconvert(f);
    std::cout << f << " -> " << i << '\n';

    f = 1.5;
    i = fast_fpconvert(f);
    std::cout << f << " -> " << i << '\n';

    f = -1.5;
    i = fast_fpconvert(f);
    std::cout << f << " -> " << i << '\n';


    double d = 1.0;
    i = fast_fpconvert(d);
    std::cout << d << " -> " << i << '\n';

    d = 1.5;
    i = fast_fpconvert(d);
    std::cout << d << " -> " << i << '\n';

    d = -1.5;
    i = fast_fpconvert(d);
    std::cout << d << " -> " << i << '\n';
}


void benchmark()
{
    RandomReal<double> rr;
    std::vector<double> randoms = rr.gen(10000, 0.0, 10.0);
}

int main()
{
    // TODO: insert any function call
    classic_ftol();
    return 0;
}

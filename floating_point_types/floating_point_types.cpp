#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

//This header also defines the following macro constants :
//MATH_ERRNO
//MATH_ERREXCEPT
//Bitmask value with the possible values math_errhandling can take.
//
//FP_FAST_FMA
//FP_FAST_FMAF
//FP_FAST_FMAL
//Each, if defined, identifies for which type fma is at least as efficient as x*y + z.
//
//FP_INFINITE
//FP_NAN
//FP_NORMAL
//FP_SUBNORMAL
//FP_ZERO
//The possible values returned by fpclassify.
//
//FP_ILOGB0
//FP_ILOGBNAN
//Special values the ilogb function may return.

// for atan2
#include <valarray>

#include "bit_operations.h"

using namespace std;



/*
http://en.wikipedia.org/wiki/Floating_point

Число с плавающей запятой состоит из:
Знака мантиссы(указывающего на отрицательность или положительность числа)
Мантиссы(выражающей значение числа без учёта порядка) (Significand)
Знака порядка
Порядка(выражающего степень основания числа, на которое умножается мантисса) (Exponent)

Нормальной формой числа с плавающей запятой называется такая форма, в которой мантисса
(без учёта знака) в десятичной системе находится на полуинтервале [0; 1).
Такая форма записи имеет недостаток: некоторые числа записываются неоднозначно

Числа одинарной точности (float)
Порядок 8 бит (-127 .. 128)
Мантисса 23 бит

Числа двойной точности
Порядок 11 бит (-1024 .. 1024)
Мантисса 52 бит

На 64 битах, мантисса составляет 1 бит знак + 52 бита, показатель — 1 бит знак + 10 бит.
Таким образом получаем диапазон точности примерно
от 4.94 * 10^-324 до 1.79 * 10^308 (от 2^-52 * 2^-1024 до 1*2^1024)

В отличие от чисел с фиксированной запятой, сетка чисел,
которые способна отобразить арифметика с плавающей запятой, неравномерна:
она более густая для чисел с малыми порядками и более редкая — для чисел с большими порядками.
Т.е. представимы только те сверхбольшие числа, в которых много нулей

Машинной эпсилон называется наименьшее положительное число, что (1 +/- Epsilon) != 1
DBL_EPSILON стандартной библиотеки примерно равен 10^-16

FP также имеют несколько специальных значений +/- INF и NaN (Not a number)
Two kinds of NaN: a quiet NaN (qNaN) and a signaling NaN (sNaN)

Представление согласно стандарта:
32 бит: 7 чисел	 e +/- 96
64 бит: 16 чисел e +/- 384

Системы компьютерной алгебры, как Maxima или Mathematica, часто "знают" точное представление
основных трансцендентных чисел (e, pi, sqrt(2), sqrt(3),...)
*/

/*
http://habrahabr.ru/post/112953/
s E        M
1 11111111 11111111111111111111111
(-1)^s * M * B^E, где s - знак, B - основание, E - порядок, M - мантисса
B=2
В новом стандарте IEE754-2008 кроме чисел с основанием 2 присутствуют числа с основанием 10,
так называемые десятичные (decimal) числа с плавающей запятой

Т.е. нормализованное число имеет следующий вид:
(-1)^s * 1.M * 2^E
Эффективный порядок определяется как E-127
*/

void print_double_binary(double d1){

    long long* double_hack = reinterpret_cast<long long*>(&d1);
    static_assert(sizeof(d1) == sizeof(*double_hack), "Double and long long should have equal size");

    cout << "Binary representation of " << d1 << " =\n\t " << *double_hack
        << " =\n\t " << bitset<sizeof(double)*8>(*double_hack) << '\n';
}

void print_float_binary(float f1){
    long* float_hack = reinterpret_cast<long*>(&f1);
    static_assert(sizeof(f1) == sizeof(*float_hack), "Float and long should have equal size");

    cout << "Binary representation of " << f1 << " =\n\t " << *float_hack
        << " =\n\t " << bitset<sizeof(float)* 8>(*float_hack) << '\n';
}

// Порядок записан со сдвигом - 15.
// То есть чтобы получить актуально значение порядка нужно вычесть из него сдвиг.
// Сдвиг можно получить по формуле 2^(b-1) - 1,
// где b - число бит, отведенное на хранение порядка
// (в случае числа одинарной точности b=8, двойной b=11).

//Целые от нуля до 2048 передаются как есть.
//Целые от 2049 до 4096 округляются к ближайшему чётному целому.
//Целые от 4097 до 8192 округляются до ближайшего целого, делящегося нацело на 4.
//Целые от 8193 до 16384 округляются до ближайшего целого, делящегося на 8.
//Целые от 16385 до 32768 округляются до ближайшего целого, делящегося на 16.
//Целые от 32769 до 65535 округляются до ближайшего целого, делящегося на 32.


// Bit access to float
// http://en.wikipedia.org/wiki/Single-precision_floating-point_format
// (see russian version)

// fraction same as mantissa
template <typename T>
struct floating_point_traits{};

template <>
struct floating_point_traits<float>{
    static const size_t significand = 23;
    static const size_t exponent = 31;
};

template <>
struct floating_point_traits<double>{
    static const size_t significand = 52;
    static const size_t exponent = 63;
};

template <typename T>
void extract_fp_components(T val){

    static const size_t exponent = floating_point_traits<T>::exponent;
    static const size_t fraction = floating_point_traits<T>::significand;

    union
    {
        T fl;
        long dw;
    } f;
    f.fl = val;
    int s = (f.dw >> exponent) ? -1 : 1;
    int e = (f.dw >> fraction) & 0xFF;
    int m =
        e ?
        (f.dw & 0x7FFFFF) | 0x800000 :
        (f.dw & 0x7FFFFF) << 1;

    e -= 127;
    cout << "sign = " << s
        << " mantissa = " << m
        << " exponent = " << e << endl;
}

// объяснение формата мантиссы
// http://www.rsdn.ru/forum/cpp/5573790.1
void extract_fp_components(float val){

    static_assert(sizeof(long) == sizeof(float), "sizeof(long) should be equal sizeof(float)");

    union
    {
        float fl;
        long dw;
    }f;

    f.fl = val;

    // extract sign bit
    int s = (f.dw >> 31) ? -1 : 1;

    // extract exponent
    int e = (f.dw >> 23) & 0xFF;

    // extract mantissa
    int mantissa = f.dw & 0x7FFFFF;

    cout << "Mantissa binary representation = "<< bitset<32>(mantissa) << endl;

    int m =
        e ?
        mantissa | 0x800000 :
        mantissa << 1;

    double m1 = double(m) / pow(2., 23.);

    // exponenta shift
    e -= 127;

    cout << "s = " << s << "; e =  " << e << "; m(2) = " << m << "; m(10) = " << m1 << endl;

}


void show_float(){

    // Перед мантиссой всегда 1.!
    // Эффективный порядок определяется как E-127!

    // see float conversion formula at
    // http://en.wikipedia.org/wiki/Single-precision_floating-point_format

    //       s E        M
    // 1.0 = 0 01111111 00000000000000000000000
    // E = 01111111 = 127 - 127 = 0
    // 1.0 = (-1)^s * 1.M * 2^E = (1-)^0 * 1.000 * 2^0 = 1.0
    float d = 1.0;
    print_float_binary(d);
    extract_fp_components(d);


    //       s E        M
    // 1.5 = 0 01111111 10000000000000000000000
    // E = 01111111 = 127 - 127 = 0
    // 1.1(2) = 2^0 + 2^(-1) = 1 + 1/2 = 1.5
    // 1.5 = (-1)^s * 1.M * 2^E = (1-)^0 * 1.1(2) * 2^0 = 1.5
    d = 1.5;
    print_float_binary(d);
    extract_fp_components(d);

    // 0.15 = 0 01111100 00110011001100110011010
    // E = 01111100 = 124-127 = -3
    // 1.00110011001100110011010 = 2^(-3) + 0^(-4) + 0^(-5) + 2^(-5) + 2^(-6) + ...
    // 0.15 = (-1)^s * 1.M * 2^E = (1-)^0 * 1.00110011001100110011010(2) * 2^(-3) = 1.5
    d = 0.15;
    print_float_binary(d);
    extract_fp_components(d);
}




void show_double(){

    // Double slower than float (single-precision)
    //  On average, on a PC of year 2012 build,
    // on CPUs calculations with double precision are 1.1–1.6 times slower than with single precision
    // on GPUs calculations with double precision are 3 to 8 times slower than float

    // 1.0 = 0 01111111111 0000000000000000000000000000000000000000000000000000
    // s = 0 (+)
    // E = 01111111111(2)-1023(10) = 1023-1023 = 0
    // M = 0
    // 1.0 = s * 1.M * 2^E = 1 * 1.0 * 2^0
    print_double_binary(1.0);

    // changed sign bit

    //-1.0 = 1 01111111111 0000000000000000000000000000000000000000000000000000
    // s = 1 (-)
    // E = 01111111111(2)-1023(10) = 1023-1023 = 0
    // M = 0
    //-1.0 = s * 1.M * 2^E = -1 * 1.0 * 2^0
    print_double_binary(-1.0);

    // 1.5 = 0 01111111111 1000000000000000000000000000000000000000000000000000
    // s = 0 (+)
    // E = 01111111111(2)-1023(10) = 1023-1023 = 0
    // M = 1(2) = 1 * 2^(-1) = 1/2 = 0.5
    // 1.0 = s * 1.M * 2^E = 1 * 1.M * 2^0 = 1 * 1.5 * 2^0
    print_double_binary(1.5);

    // 2.0 = 0 10000000000 0000000000000000000000000000000000000000000000000000
    // s = 0 (+)
    // E = (2)-1023(10) = 1024-1023 = 1
    // M = 0
    // 2.0 = s * 1.M * 2^E = 1 * 1.M * 2^E = 1 * 1.0 * 2^1
    print_double_binary(2.0);


    // 1e9 = 0 10000011100 1101110011010110010100000000000000000000000000000000
    // s = 0 (+)
    // E = 10000011100(2)-1023(10) = 1052-1023 = 29
    // M = 1101110011010110010100000000000000000000000000000000
    // 1.1101110011010110010100000000000000000000000000000000 = 1*2^(-29) + 1*2^(-28) + 0*2^(-27) + ...
    // 1e9 = s * 1.M * 2^E
    print_double_binary(1000000000.0);

    // etc

    //1.5e9= 0 10000011101 0110010110100000101111000000000000000000000000000000
    print_double_binary(1500000000.0);

    // 2e9 = 0 10000011101 1101110011010110010100000000000000000000000000000000
    print_double_binary(2000000000.0);

    //0x 0000 0000 0000 0000 = 0
    //0x 8000 0000 0000 0000 = -0
    //0x 7ff0 0000 0000 0000 = Infinity
    //0x fff0 0000 0000 0000 = -Infinity
    //0x 7fff ffff ffff ffff = NaN
}


void show_floating_point(){
    show_float();
    show_double();
}

void print_roundings_header(){
    cout
        << "val" << '\t'
        << "ceil" << '\t'
        << "floor" << '\t'
        << "trunc" << '\t'
        << "round" << '\t'
        << "lround" << '\t'
        << "rint" << '\t'
        << "lrint" << '\t'
        << "nearbyint" << '\n';
}


void print_roundings(double val){
    cout << fixed;
    cout << setprecision(4);
    cout
        << val << '\t'
        << ceil(val) << '\t'
        << floor(val) << '\t'
        << trunc(val) << '\t'
        << round(val) << '\t'
        << lround(val) << '\t'
        << rint(val) << '\t'
        << lrint(val) << '\t'
        << nearbyint(val) << '\n';
}

void print_fpclassify(double val){
    int val_type = fpclassify(val);
    cout << val << " is ";
    switch (val_type)
    {
    case FP_INFINITE:
        cout << "infinite" << endl;
        break;
    case FP_NAN:
        cout << "NaN" << endl;
        break;
    case FP_ZERO:
        cout << "zero" << endl;
        break;
    case FP_SUBNORMAL:
        cout << "subnormal" << endl;
        break;
    case FP_NORMAL:
        cout << "normal";
        if (signbit(val))
        {
            cout << " negative" << endl;
        }
        else
        {
            cout << " positive or unsigned" << endl;
        }
        break;
    default:
        cout << "Error! Should not be here!" << endl;
    }
}

void show_cmath_fpoint_operations(){

    //
    // Trigonometric
    double pi = M_PI;

    double c1 = cos(pi);
    cout << "cos PI = " << c1 << '\n';

    double s1 = sin(pi); // close to 0, but not a 0
    cout << "sin PI = " << s1 << '\n';

    double t1 = tan(pi);
    cout << "tan PI = " << t1 << '\n';

    double ac1 = acos(c1);
    cout << "acos c1 = " << ac1 << '\n';

    double as1 = asin(s1);
    cout << "asin s1 = " << as1 << '\n';

    double at1 = atan(t1);
    cout << "atan t1 = " << at1 << '\n';

    double at2 = atan2( 1.0, 2.0 );
    cout << "atan2 1/2 = " << at2 << '\n';

    // Hyperbolic supported as well

    // Exponential
    double param = 5.0;

    double result = exp(param);
    cout << "exp (" << param << ") = " << result << "\n";

    // Breaks the floating point number x into its binary significand
    // (a floating point value between 0.5(included)and 1.0(excluded))
    // and an integral exponent for 2, such that :
    // x = significand * 2^exponent
    int n = 0;
    param = 8.0;
    result = frexp(param, &n);
    cout << param << " = " << result << "*2^" << n << '\n';

    // Compose value back
    double back = ldexp(result, n);
    cout << back << " = " << result << "*2^" << n << '\n';

    // ln and lg are calculated as well

    // modf() splits value
    double fractpart = 0.0;
    double intpart = 0.0;
    fractpart = modf(pi, &intpart);
    cout << pi << " = " << intpart << " + " << fractpart << '\n';

    // exp2() returns the base-2 exponential function of x
    // which is 2 raised to the power x
    param = 8.0;
    double e2 = exp2(param);
    cout << "2 ^ " << param << " = " << e2 << '\n';

    // expm1() returns e raised to the power x minus one : e^x - 1
    cout << "expm1(1.0) = e^x - 1 = " << expm1(1.0) << '\n';

    // logb() returns the integral part of the logarithm of |x|, using FLT_RADIX as base
    // log1p() returns the natural logarithm of one plus x (log(1+x))
    // log2() returns the binary(base - 2) logarithm of x
    cout << "log(10.0) = " << log(10.0) << '\n';
    cout << "logb(10.0) = " << ilogb(10.0) << '\n';
    cout << "log1p(10.0) = log(1+x) = " << log1p(10.0) << '\n';
    cout << "log2(1024) = " << log2(1024) << '\n';

    // scalbn(x, n) = x * FLT_RADIX^n
    // scalbln(x, n) = ?
    double x = 2.0;
    int n1 = 4;
    cout << "FLT_RADIX = " << FLT_RADIX << '\n';
    cout << "x = " << x << '\n';
    cout << "n1 = " << n1 << '\n';
    cout << "scalbn(x, n) = x * FLT_RADIX^n = " << scalbn(x, n1) << '\n';
    cout << "scalbln(x, n) (?)= " << scalbln(x, n1) << '\n';

    // Roundings
    print_roundings_header();
    print_roundings(0.0);
    print_roundings(0.3);
    print_roundings(0.5);
    print_roundings(0.8);
    print_roundings(1.3);
    print_roundings(2.3);
    print_roundings(2.5);
    print_roundings(2.8);

    // Power functions
    cout << "sqrt(2.0) = " << sqrt(2.0) << '\n';
    cout << "cbrt(2.0) = " << cbrt(2.0) << '\n';

    // hypot() returns the hypotenuse of a right - angled triangle whose legs are x and y
    cout << "Hypotenuse of (3, 4) =" << hypot(3, 4) << '\n';

    // Error function/Gamma function (probability, statistics, PDE)
    // http://en.wikipedia.org/wiki/Error_function
    // http://en.wikipedia.org/wiki/Gamma_function
    cout << "Error function of 1 =" << erf(1.0) << '\n';
    cout << "Complimentary error function of 1 =" << erfc(1.0) << '\n';
    cout << "Gamma function of 1 =" << tgamma(1.0) << '\n';
    cout << "Log of Gamma function of 1 =" << tgammal(1.0) << '\n';

    // copysign copies sign of second value to the first
    cout << "copysign(-10.0, 1.0)" << copysign(-10.0, 1.0) << '\n';
    cout << "copysign(-10.0, -1.0)" << copysign(-10.0, -1.0) << '\n';
    cout << "copysign(10.0, -1.0)" << copysign(10.0, -1.0) << '\n';

    // nextafter() returns the next representable value after x in the direction of y
    cout << "Next representable value after 0 = " << nextafter(0.0, 1.0) << '\n';
    cout << "Next representable value before 0 = " << nextafter(0.0, -1.0) << '\n';
    // nexttoward() the same with long y

    // NAN (not a number)
    // he NaN values are used to identify undefined or non-representable values
    // for floating-point elements, such as the square root of negative numbers or the result of 0/0
    // Generate quiet NAN (const char* param is implementation-specific)
    double nn = nan("");
    float nf = nanf("");
    cout << "For " << nn << " isnan(nn) = " << std::isnan(nn) << '\n';
    cout << "For " << nf << " isnan(nf) = " << std::isnan(nf) << '\n';
    cout << "For " << "isnan sqrt(-1.0) = " << std::isnan(sqrt(-1.0)) << '\n';
    // isnormal is opposite to isnan, but also checks for INF and 0
    cout << "For " << "isnormal sqrt(-1.0) = " << isnormal(sqrt(-1.0)) << '\n';

    // isunordered() check at least one of values is NAN
    if (isunordered(sqrt(-1.0), 0.0))
        cout << "sqrt(-1.0) and 0.0 cannot be ordered\n";

    double myinf = INFINITY;
    if ((1 / sin(0.0) == myinf) && (!isnormal(myinf)))
    {
        cout << "1/0 is " << myinf << '\n';
    }

    // Value is too large
    double huge = pow(10.0, 1000000000);
    if (huge == HUGE_VAL)
    {
        cout << huge << " is HUGE_VAL\n";
    }

    // fpclassify() returns a value of type int that matches one of the classification macro constants
    //FP_INFINITE	Positive or negative infinity(overflow)
    //FP_NAN	Not - A - Number
    //FP_ZERO	Value of zero
    //FP_SUBNORMAL	Sub - normal value(underflow)
    //FP_NORMAL	Normal value(none of the above)
    print_fpclassify( 1/sin(0.0) );
    print_fpclassify(sin(0.0) / sin(0.0));
    print_fpclassify( 0.0 );
    print_fpclassify(1.0);
    print_fpclassify(-1.0);

    // subnormal numbers could not be representet a normal double
    // (less than minimal double)
    // requires expanded representation, works slower (10-100 times)
    double subnorm = 1.0;
    while (fpclassify(subnorm) != FP_SUBNORMAL)
    {
        subnorm /= 2;
    }
    print_fpclassify(subnorm);

}


/*
Так. Тут начался беспредел с ужасами про сравнение плавающей точки на равенство.
Требуется ликбез.Сравнивать плавающие числа(для педантов — значения переменных типа float или double)
вполне можно и даже нужно.
Но надо понимать сущность этой плавающей точки.
А сущность заключается в том, что числа с фиксированной точкой (целые — это частный случай
чисел с фиксированной точкой) имеют абсолютное значение погрешности, в отличие от чисел с плавающей точкой,
где значение погрешности находится в прямой пропорциональности от модуля числа.

Когда числа вычислены разными способами — одно через синус, а другое — через экспоненту,
проверка на равенство скорее всего не сработает.
Так же, как и не сработает сравнение с константой.
Но это же относится и к целым числам, если скажем, мы нормализуем значения от 0...1 к 0...1000000000.
То есть, не имеет значения, плавающие это числа или целые.
В определенных ситуациях сравнивать их на строгое равенство нельзя.
В этих ситуациях надо использовать некую Epsilon.
И вот здесь - то и вылезает наружу вся безграмотность. Что такое DBL_EPSILON ?
— а это вот что.Это минимальное значение, которое при прибавлении его к единице,
меняет значение этой единицы.Понимаете ? — к единице! Строгой единице, числу 1.0
и ни к какому другому.Поэтому сравнивать числа с плавающей точкой на + / -DBL_EPSILON
совершенно бессмысленно. Это сравнение выдает всю глубину невежества и неспособности думать мозгом.
Факты таковы — плавающие числа больше 2.0 эту DBL_EPSILON просто не ощущают.
Им что прибавляй ее, что что нет — ничего не меняет.
Для этих чисел DBL_EPSILON является строгим нулем и просто не существует. В то же время,
DBL_EPSILON имеет значение порядка 1e-16.Что это значит ? А это значит, что числа в диапазоне
Планковских масштабов(типа 1e-34) с точки зрения этой DBL_EPSILON будут все равны. То есть,
эта 1e-16 становится слоном в посудной лавке. А ведь постоянная Планка ничуть не хуже скорости света
— для этого собственно и были придуманы числа с плавающей точкой, чтобы отображать большие диапазоны
значений с неким фиксированным количеством знаков.
*/

// Так для чего же все - таки нужна эта самая DBL_EPSILON(ну или FLT_EPSILON) ?
// Нужна - ли ? — нужна!Есть ситуации, когда действительно надо сравнивать числа
// в неком допустимом интервале. В каком ? — А вот это как раз и зависит от
// абсолютного значения чисел и сущности вычислений.
// Короче говоря, надо эту Epsilon умножить на значение числа.
// А поскольку у нас два числа, то все усложняется — какое из них брать.
// То есть, корректное сравнение выглядит так :
bool close_enough(double a, double b){
    if (fabs(a - b) <= DBL_EPSILON * fmax(fabs(a), fabs(b)))
    {
        return true;
    }
    return false;
}

/*
Дорого ? Да, дорого, а все остальное неправильно, такие дела. Но и это тоже неправильно!
Дело в том, что этот DBL_EPSILON определяет разницу в 1 (один!) значащий бит экспоненты
в приложении к числу 1.0.На практике такой разницы не встречается — числа либо строго равны,
либо могут различаться больше чем на один значащий бит. Поэтому надо брать что - то типа
16 * DBL_EPSILON, чтобы игнрорировать разницу в 4 младших бита
(или примерно полторы последние значащие десятичные цифры из примерно 16 имеющихся).

Конечно же, есть случаи, когда диапазон чисел более - менее известен и предсказуем.
Скажем, 0...1000. В этом случае, для сравнения на приблизительное равенство можно взять константу,
типа 1000 * 16 * DBL_EPSILON.
Но надо иметь в виду, что такое сравнение фактически превращает всю идею плавающей точки
в фиксированную точку(догадайтесь, почему).

Я вообще поражен уровню невежества — даже в весьма грамотной библиотеке GPC by Alan Murta
используется тупое сравнение с константной Epsilon.
На диапазонах экранных координат это все равно, что сравнение на строгое равенство,
а на 1e-20 алгоритм вообще перестает работать.
*/

// We can take advantage of this
// alignment shift to change the bit representation of a floating-point number
// until it’s the same as an integer’s bit representation,
// and then we can just read it like a normal integer

// This trick works for positive numbers,
// but if you try to convert a negative number it will fail

// the normalization step screws it up because now that we’ve
// borrowed from the implicit 1 bit, it’s no longer the most significant bit

// We can get this 1 bit simply by multiplying our large number by 1.5. 1.5 in binary is
// 1.1, and the first 1 becomes the implicit 1 bit, and the second becomes the
// most significant bit of the mantissa

// If you subtract the integer representation of our large, floating-point shift number
// (in other words, treat its bits like an integer instead of a float) from the
// integer representation of the number we just converted, it will remove all the
// high bits properly for both types of numbers, making the bits equal zero for
// positive values and filling them in with ones for negative values

// http://stereopsis.com/sree/fpu2006.html

int fast_float2int(float x) {

    union castable{
        float f;
        int i;
    };

    castable magic;
    // 1 << 22 = 2^23 (e.g. convert float to integer)
    // 150 << 23 is 1.5 trick for negative conversion
    // For double the same masks are (1LL << 51) and (1075LL << 52)
    magic.i = (150 << 23) | (1 << 22);

    volatile castable c;
    c.f = x + magic.f;
    // Not to create 'if' condition for the negatives we use subtraction trick
    // it will remove all the high bits properly for both types of numbers(positive and negative),
    // making the bits equal zero for positive values and filling them in with ones for negative values.
    return c.i - magic.i;
}

int fast_double2int(double d){

    union castable{
        double d;
        long long l;
    };

    castable magic;
    magic.l = (1075LL << 52) | (1LL << 51);

    volatile castable c;
    c.d = d + magic.d;
    return c.l - magic.l;
}

template <typename T>
struct TPTraitsBase{};

template <>
struct TPTraitsBase<float>{
    typedef int ret_type;
    typedef float convert_type;
    typedef int mask_type;
    static const mask_type mask = (150 << 23) | (1 << 22);

};

template <>
struct TPTraitsBase<double>{
    typedef long ret_type;
    typedef double convert_type;
    typedef long long mask_type;
    static const mask_type mask = (1075LL << 52) | (1LL << 51);
};

template <typename T>
struct TPTraits : TPTraitsBase<T>{
    union castable{
        typename TPTraitsBase<T>::convert_type f;
        typename TPTraitsBase<T>::mask_type i;
    };
};


template <typename FP>
typename TPTraits<FP>::ret_type fast_fp2long(FP f)
{
    typename TPTraits<FP>::castable magic;
    magic.i = TPTraits<FP>::mask;

    volatile typename TPTraits<FP>::castable ret;
    ret.f = f + magic.f;
    return ret.i - magic.i;
}


void show_universal_fast_float2int(){
    float f = 1.0;
    int i = fast_fp2long(f);
    cout << f << " -> " << i << endl;

    f = 1.5;
    i = fast_fp2long(f);
    cout << f << " -> " << i << endl;

    f = -1.5;
    i = fast_fp2long(f);
    cout << f << " -> " << i << endl;



    double d = 1.0;
    i = fast_fp2long(d);
    cout << d << " -> " << i << endl;

    d = 1.5;
    i = fast_fp2long(d);
    cout << d << " -> " << i << endl;

    d = -1.5;
    i = fast_fp2long(d);
    cout << d << " -> " << i << endl;
}


int fast_float2int_debug(float x)
{
    /*
    Оказалось что очень просто работает. Флоаты в IEEE стандарте лежат так - в младших битах мантиса,
    потом экспонента со смещением, и самый старший - знак. Вся соль фокуса в том, как лежит мантиса -
    лежит она без целой части, которая все равно всегда единица. Зная это можно сконвертировать вообще
    без плавающих операций - дописать единичку и сдвинуть мантису вправо на столько разрядов,
    сколько всего в мантисе минус то, что в экспоненте и, в случае отрицательного числа,
    отнять полученное от нуля. Эт конечно гимморой, но, к великому счастью, такие операции делает сам ФПУ.

    Во-первых сдвиг ФПУ делает, когда хочит провести какую - нибудь операцию над двумя аргументами
    с разными экспонентами и двигает мантису меньшего числа вправо пока экспонента у него не повысится
    до экспоненты большего числа. То есть двигает вправо на разницу, если поставить экспоненту в большем
    числе равной числу разрядов в мантисе то как раз будет сдвиг на столько разрядов, сколько надо!
    Приведу пример в десятичной системе : допустим у нас есть 5 разрядов мантисы и есть число 36.72,
    которое запишеться как 3.67200е1. Допустим мы хотим сложить его с 100000 или 1.00000е5.
    ФПУ подгонит его под экспоненту 5 сдвигом вправо на 5 - 1 = 4 и получится 0.0003672е5.
    Поскольку у нас всего 5 разрядов на все про все останется только 0.00036е5 или 0.00037е5
    в зависимости от того, стоит ли округление отброшенных разрядов или нет.
    В конце сложения получим 1.00036е5, число, в котором мантиса в младших разрядах
    равна целой части нашего исходного аргумента.

    */
    cout << "\nConverting to int x = " << x << endl;
    const int magic1 = (150 << 23);
    cout << "magic1 = " << bitset<32>(magic1) << endl;

    const int magic2 = (1 << 22);
    cout << "magic2 = " << bitset<32>(magic2) << endl;

    int magic = magic1 | magic2;
    cout << "magic =  " << bitset<32>(magic) << endl;

    /*
    С отрицательным же числом этот номер не пройдет, потому что после сложений произойдет
    перенос из старшего разряда и экспонента уменьшится и все сдвинется  влево! Как же быть ?
    В десятичном случае - элементарно, нужно складывать не с 1е5, а 2е5 - тогда перенесется
    из двойки, а в двоичном - чуть сложнее, поскольку двойки у нас нет нужно использовать 1.1еХ
    и жертвовать одним разрядом точности.То есть результирующие волшебное число для float
    с 23 битами мантисы это 1.1е1011 (в бинарной форме, в десятичной - 1.5 * 2 ^ 23),
    поскольку мантиса сдвинута на 127 число это получается как(150 << 23) | (1 << 22).
    */

    // append to the converted number
    // float representation of the magic integer mask
    cout << "x =  " << x << ", x2 = " << bitset<32>(x) << endl;
    cout << "magic =  " << *(reinterpret_cast<float*>(&magic)) << ", magic2 = " << bitset<32>(magic) << endl;
    cout << "x += magic" << endl;
    x += *(reinterpret_cast<float*>(&magic));
    cout << "x  = " << x << " x2 = " << bitset<32>(x) << endl;

    /*
    После сдвига хорошо бы почистить старшие разряды от экспоненты и знака.
    В той конверсии, что я раньше видел, обходили этот шаг хитро - использовали double
    вместо float и просто брали младшее двойное слово результата, в котором была одна мантиса.
    Собственно из за этого я и стал рыть, ибо дабл использовать не хотелось.
    Занулить старшие биты маской - не пройдет ибо будет работать только с положительными числами.
    Писать условия или битовую логику тоже не хочется. К счастью есть простейший способ обработать
    старшие биты одной операцией!

    В самом деле, если рассматривать float то у нас возможно только два варианта результата :
    знак - всегда 0, экспонента - всегда 150, а вот самый старший бит мантисы меняется -
    в случае положительного числа 1, в случае отрицательного 0. Если отбросить младшие 22 бита,
    то можно заметить, что для положительных чисел результат равен ровно нашему волшебному числу,
    а для отрицательных - меньше его ровно на 1! То есть если отнять от результата волшебное число как int
    то младшие 22 бита не изменятся, в старших битах положительного числа будут одни нули,
    а у отрицательного - одни единички, как раз то, что нам надо.
    */

    // subtract from integer representation of the converted number
    // magic int mask
    cout << "x = " << bitset<32>(*(reinterpret_cast<int*>(&x))) << endl;
    cout << "magic = " << bitset<32>(magic) << endl;
    int res = *(reinterpret_cast<int*>(&x)) - magic;

    cout << "x - magic = " << res << " res2 = " << bitset<32>(res) << endl;
    return res;
    /*
    Сишное приведение вызовет функцию _ftol, которая помимо использования медленной инструкции для конверсии
    (ФПУ обычно не оптимизированы для таких операций) еще будет сохранять текущий режим округления,
    устанавливать режим округления соответсвующий стандарту С, конвертить, потом востанавливать старый.
    Замерь сам сколько занимает каст и сколько конверсия волшебным числом.
    И кроме того сишный каст и даже инлайн ассемблер(который по определению непереносим) не могут сконвертить к фиксд пойнту.

    Это, конечно, не я сам придумал, а вычитал в умной статье умного дядько Криса Хекера,
    найденой вот тут http ://www.d6.com/users/checker/pdfs/gdmfp.pdf
    Но за то я теперь сам могу написать конверсию и не только в целые,
    а в любой фиксд пойнт и никогда не забуду волшебных чисел.

    */
}



// http://en.wikipedia.org/wiki/Fast_inverse_square_root
// return 1/sqrt(number)
float quick_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;                       // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1);               // what the fuck?
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y));   // 1st iteration
    //y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    return y;
}


//////////////////////////////////////////////////////////////////////////
// Hamming weight (bits count)

//types and constants used in the functions below

const uint64_t m1 = 0x5555555555555555; //binary: 0101...
const uint64_t m2 = 0x3333333333333333; //binary: 00110011..
const uint64_t m4 = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const uint64_t m8 = 0x00ff00ff00ff00ff; //binary:  8 zeros,  8 ones ...
const uint64_t m16 = 0x0000ffff0000ffff; //binary: 16 zeros, 16 ones ...
const uint64_t m32 = 0x00000000ffffffff; //binary: 32 zeros, 32 ones
const uint64_t hff = 0xffffffffffffffff; //binary: all ones
const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

//This is a naive implementation, shown for comparison,
//and to help in understanding the better functions.
//It uses 24 arithmetic operations (shift, add, and).
int popcount_1(uint64_t x) {
    x = (x & m1) + ((x >> 1) & m1); //put count of each  2 bits into those  2 bits 
    x = (x & m2) + ((x >> 2) & m2); //put count of each  4 bits into those  4 bits 
    x = (x & m4) + ((x >> 4) & m4); //put count of each  8 bits into those  8 bits 
    x = (x & m8) + ((x >> 8) & m8); //put count of each 16 bits into those 16 bits 
    x = (x & m16) + ((x >> 16) & m16); //put count of each 32 bits into those 32 bits 
    x = (x & m32) + ((x >> 32) & m32); //put count of each 64 bits into those 64 bits 
    return x;
}

//This uses fewer arithmetic operations than any other known  
//implementation on machines with slow multiplication.
//It uses 17 arithmetic operations.
int popcount_2(uint64_t x) {
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    x += x >> 8;  //put count of each 16 bits into their lowest 8 bits
    x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
    x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
    return x & 0x7f;
}

//This uses fewer arithmetic operations than any other known  
//implementation on machines with fast multiplication.
//It uses 12 arithmetic operations, one of which is a multiply.
int popcount_3(uint64_t x) {
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}


// The above implementations have the best worst - case behavior of any known algorithm.
// However, when a value is expected to have few nonzero bits, it may instead be more efficient 
// to use algorithms that count these bits one at a time.
// The bitwise and of x with x − 1 differs from x only in zeroing out the least significant nonzero bit : 
// subtracting 1 changes the rightmost string of 0s to 1s, and changes the rightmost 1 to a 0. 
// If x originally had n bits that were 1, then after only n iterations of this operation,
// x will be reduced to zero.The following implementation is based on this principle.


//This is better when most bits in x are 0
//It uses 3 arithmetic operations and one comparison/branch per "1" bit in x.
int popcount_4(uint64_t x) {
    int count;
    for (count = 0; x; count++)
        x &= x - 1;
    return count;
}


// If we are allowed greater memory usage, we can calculate the Hamming weight faster than the above methods.
// With unlimited memory, we could simply create a large lookup table of the Hamming weight of every 64 bit integer.
// If we can store a lookup table of the hamming function of every 16 bit integer, 
// we can do the following to compute the Hamming weight of every 32 bit integer.


static uint8_t wordbits[65536] = { /* bitcounts of integers 0 through 65535, inclusive */ };
static int popcount(uint32_t i)
{
    return (wordbits[i & 0xFFFF] + wordbits[i >> 16]);
}

void show_hamming_weight()
{
    int bits1 = popcount_1(1);
    cout << "Hamming weight(1) = " << bits1 << endl;

    int bits2 = popcount_2(2);
    cout << "Hamming weight(2) = " << bits2 << endl;

    int bits3 = popcount_3(3);
    cout << "Hamming weight(3) = " << bits3 << endl;

    int bits4 = popcount_4(8);
    cout << "Hamming weight(8) = " << bits4 << endl;
}

int show_MXCSR_registry()
{
    set_fp_exceptions();
    // Floating-point overflow
    cout << exp(3671.81) << endl;
}

// Switch on access to floating-point environment
//http://www.cplusplus.com/reference/cfenv/FENV_ACCESS/
#pragma STDC FENV_ACCESS ON

void show_close_enough()
{

    // nextafter() returns the next representable value after x in the direction of y
    double d = 1.0;
    double e = nextafter(d, 1000.0);
    if (close_enough(d, e))
        cout << d << " == " << e << endl;


    d = 0.00000000000000000001;
    e = nextafter(d, 1.0);
    if (close_enough(d, e))
        cout << d << " == " << e << endl;
}

void show_fast_float2int()
{
    float f = 1.0;
    int i = fast_float2int(f);
    int i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;


    f = 1.1;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;

    f = 1.5;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;

    f = 2.0;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;

    f = 10.0;
    i = fast_float2int(f);
    cout << f << " -> " << i << endl;

    double d = 1.0;
    i = fast_double2int(d);
    cout << d << " -> " << i << endl;
}

void show_fast_sqrt()
{

    float f = 4.0;
    float i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;

    f = 16.0;
    i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;

    f = 2.0;
    i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;

    f = -1.0;
    i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;
}


void show_fp_coltrol_noexcept(){

	double d1 = 0., d2 = 0., d3 = 0., s = 0.;
	int res = 0;

	printf("Floating point state flags:\n");
	printf(" FE_DIVBYZERO: %x\n", FE_DIVBYZERO);
	printf(" FE_INEXACT: %x\n", FE_INEXACT);
	printf(" FE_INVALID: %x\n", FE_INVALID);
	printf(" FE_OVERFLOW: %x\n", FE_OVERFLOW);
	printf(" FE_UNDERFLOW: %x\n", FE_UNDERFLOW);

	printf("Floating point rounding flags\n");
	printf(" FE_DOWNWARD: %x\n", FE_DOWNWARD);
	printf(" FE_TONEAREST: %x\n", FE_TONEAREST);
	printf(" FE_TOWARDZERO: %x\n", FE_TOWARDZERO);
	printf(" FE_UPWARD: %x\n", FE_UPWARD);

	printf("Current exceptions flags: %x\n", fetestexcept(FE_ALL_EXCEPT));

	printf("Current rounding mode: %x\n", fegetround());

	// флаги исключений очищены
	// далее они будут устанавливаться по мере накопления ошибок
	feclearexcept(FE_ALL_EXCEPT);

	/* Вызовем ситуацию исчезновения порядка */
	d1 = 1.0;
    size_t steps = 0;
	do {
		d1 /= 2.0;
        ++steps;
	} while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
    printf("Inexact/Underflow Exceptions in %d steps: %x\n", steps, res);
	printf("2^-inf: %g\n", d1);

	feclearexcept(res);

	/* Вызовем ситуацию переполнения */
	d2 = 1.0;
    steps = 0;
	do {
		d2 *= 2.0;
        ++steps;
	} while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
    printf("Inexact/Overflow Exceptions in %d steps: %x\n", steps, res);
	printf("2^+inf: %g\n", d2);

	feclearexcept(res);

	/* Вызовем ситуацию деления на нуль */
	d3 = 1 / d1;
	res = fetestexcept(FE_ALL_EXCEPT);
	printf("Zero Div Exceptions: %x\n", res);
	printf("1/0: %g\n", d3);

	feclearexcept(res);

	/* Пример того, как может возникать потеря точности */
	s = 1;
    steps = 0;
	do {
		s = (s + 2 / s) * 0.5;
        ++steps;
	} while ((s * s - 2) > 0);
    printf("Inexact Exceptions in %d steps: %x\n", steps, fetestexcept(FE_ALL_EXCEPT));
	printf("sqrt (2): %g\n", s);
}

void show_fp_coltrol(){

	/*
	The floating-point environment maintains a series of status flags and specific control modes.
	Specific about the contents of the floating-point environment depend on the implementation,
	but the status flags generally include the floating-point exceptions and their associated information,
	and the control modes include at least the rounding direction.
	*/

	// FE_* macro - floating point exceptions and rounding rules

	// 1. State (exception) flags
	//FE_DIVBYZERO Pole error exception(macro)
	//FE_INEXACT Inexact result exception(macro)
	//FE_INVALID Invalid argument exception(macro)
	//FE_OVERFLOW Overflow range error exception(macro)
	//FE_UNDERFLOW Underflow range error exception(macro)
	//FE_ALL_EXCEPT All exceptions(macro)

	// 2. Rounding rules
	//FE_DOWNWARD Downward rounding direction mode
	//FE_TONEAREST To - nearest rounding direction mode(macro)
	//FE_TOWARDZERO Toward - zero rounding direction mode(macro)
	//FE_UPWARD Upward rounding direction mode(macro)

	int ret = 0;

	// http://www.intuit.ru/studies/courses/53/53/lecture/1585?page=9
	// http://ru.wikipedia.org/wiki/Fenv.h
	// http://www.cplusplus.com/reference/cfenv/

	// Флаг состояния вещественной арифметики – это системная переменная, 
	// значение которой устанавливается(но никогда не очищается) при возбуждении 
	// исключительной ситуации и содержит дополнительную информацию о случившемся исключении

	// default FP environment (system-dependent!)
	fenv_t fenv = *FE_DFL_ENV;

	// получить текущее окружение FP
	ret = fegetenv(&fenv);
	cout << "fegetenv() returned " << ret << endl; // 0 is no errors

	// pair function setting fenv_t
	ret = fesetenv(&fenv);
	cout << "fesetenv() returned " << ret << endl; // 0 is no errors

	// Attempts to clear the floating-point exceptions specified
	feclearexcept(FE_ALL_EXCEPT);
	ret = fegetenv(&fenv);

	// feholdexcept() saves exception flags clears 
	double x = sin(0.0); // = 0
	fenv_t fe;
	feholdexcept(&fe);
	x = log(x); // should raise exception, but fenv is clear

	// Функция feupdateenv() выполняет еще более сложные действия. 
	// Она сохраняет в своей локальной памяти информацию о текущей исключительной ситуации, 
	// устанавливает новую среду по аргументу fenvp и затем пытается возбудить в ней сохраненное исключение.
	// Подобные манипуляции полезны, когда массовые вычисления производятся в безостановочном режиме, 
	// а затем режим меняется и обрабатывается все то нехорошее, что накопилось за это время.
	feupdateenv(&fe);
	cout << "log 0 = " << x << endl;
	if (!fetestexcept(FE_ALL_EXCEPT))
		cout << "no exceptions raised" << endl;
}


void set_fp_exceptions_MXCSR()
{
    //Set the x86 floating-point control word according to what
    //exceptions you want to trap.

    _clearfp(); //Always call _clearfp before setting the control

    //word
    //Because the second parameter in the following call is 0, it
    //only returns the floating-point control word

    unsigned int cw = _controlfp(0, 0); //Get the default control

    //word
    //Set the exception masks off for exceptions that you want to
    //trap. When a mask bit is set, the corresponding floating-point
    //exception is //blocked from being generating.

    cw &= ~(EM_OVERFLOW | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);

    //For any bit in the second parameter (mask) that is 1, the
    //corresponding bit in the first parameter is used to update
    //the control word.

    unsigned int cwOriginal = _controlfp(cw, MCW_EM); //Set it.

    //MCW_EM is defined in float.h.
    //Restore the original value when done:
    //_controlfp(cwOriginal, MCW_EM);

    unsigned long cntrReg;

    _asm
    {
        stmxcsr[cntrReg] //Get MXCSR register

        //bit 7 - invalid instruction mask
        and [cntrReg], 0FFFFFF7Fh

        //bit 9 - divide-by-zero mask
        //bit 10 - overflow mask
        //bit 11 - underflow mask
        ldmxcsr[cntrReg] //Load MXCSR register
    }
}

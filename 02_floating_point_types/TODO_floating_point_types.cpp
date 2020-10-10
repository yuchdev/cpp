#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

// for atan2
#include <valarray>

/// Returns bitset with bitwise representation of 'value'
template <typename T>
std::bitset<sizeof(T) * 8> bitwise(T value)
{
    constexpr size_t bits_size = sizeof(T) * 8;
    return std::bitset<bits_size>(value);
}

// Converting floating point to integer
// Using any type of cast, C++ or C style, we implicitly call C function ftol(),
// which saves rounding mode, set rounding mode for the conversion, convert and restore rounding mode.
// Let's see some assembly representation

// We can take advantage of this
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
    std::cout << f << " -> " << i << '\n';

    f = 1.5;
    i = fast_fp2long(f);
    std::cout << f << " -> " << i << '\n';

    f = -1.5;
    i = fast_fp2long(f);
    std::cout << f << " -> " << i << '\n';



    double d = 1.0;
    i = fast_fp2long(d);
    std::cout << d << " -> " << i << '\n';

    d = 1.5;
    i = fast_fp2long(d);
    std::cout << d << " -> " << i << '\n';

    d = -1.5;
    i = fast_fp2long(d);
    std::cout << d << " -> " << i << '\n';
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
    std::cout << "\nConverting to int x = " << x << '\n';
    const int magic1 = (150 << 23);
    std::cout << "magic1 = " << bitset<32>(magic1) << '\n';

    const int magic2 = (1 << 22);
    std::cout << "magic2 = " << bitset<32>(magic2) << '\n';

    int magic = magic1 | magic2;
    std::cout << "magic =  " << bitset<32>(magic) << '\n';

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
    std::cout << "x =  " << x << ", x2 = " << bitset<32>(x) << '\n';
    std::cout << "magic =  " << *(reinterpret_cast<float*>(&magic)) << ", magic2 = " << bitset<32>(magic) << '\n';
    std::cout << "x += magic" << '\n';
    x += *(reinterpret_cast<float*>(&magic));
    std::cout << "x  = " << x << " x2 = " << bitset<32>(x) << '\n';

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
    std::cout << "x = " << bitset<32>(*(reinterpret_cast<int*>(&x))) << '\n';
    std::cout << "magic = " << bitset<32>(magic) << '\n';
    int res = *(reinterpret_cast<int*>(&x)) - magic;

    std::cout << "x - magic = " << res << " res2 = " << bitset<32>(res) << '\n';
    return res;
    /*
    Сишное приведение вызовет функцию _ftol, которая помимо использования медленной инструкции для конверсии
    (ФПУ обычно не оптимизированы для таких операций) еще будет сохранять текущий режим округления,
    устанавливать режим округления соответсвующий стандарту С, конвертить, потом востанавливать старый.
    Замерь сам сколько занимает каст и сколько конверсия волшебным числом.
    И кроме того сишный каст и даже инлайн ассемблер(который по определению непереносим) не могут сконвертить к фиксд пойнту.

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
    
    // evil floating point bit level hacking
    i = *(long *)&y;                       
    
    // WFT?
    i = 0x5f3759df - (i >> 1);               
    y = *(float *)&i;

    // 1st iteration
    y = y * (threehalfs - (x2 * y * y));

    // 2nd iteration, this can be removed
    y  = y * ( threehalfs - ( x2 * y * y ) );

    return y;
}

int show_MXCSR_registry()
{
    // TODO:
    // set_fp_exceptions();
    // Floating-point overflow
    std::cout << exp(3671.81) << '\n';
}


void show_fast_float2int()
{
    float f = 1.0;
    int i = fast_float2int(f);
    int i1 = fast_float2int_debug(f);
    std::cout << f << " -> " << i << '\n';


    f = 1.1;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    std::cout << f << " -> " << i << '\n';

    f = 1.5;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    std::cout << f << " -> " << i << '\n';

    f = 2.0;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    std::cout << f << " -> " << i << '\n';

    f = 10.0;
    i = fast_float2int(f);
    std::cout << f << " -> " << i << '\n';

    double d = 1.0;
    i = fast_double2int(d);
    std::cout << d << " -> " << i << '\n';
}

void show_fast_sqrt()
{

    float f = 4.0;
    float i = quick_rsqrt(f);
    std::cout << "sqrt " << f << " = " << 1 / i << '\n';

    f = 16.0;
    i = quick_rsqrt(f);
    std::cout << "sqrt " << f << " = " << 1 / i << '\n';

    f = 2.0;
    i = quick_rsqrt(f);
    std::cout << "sqrt " << f << " = " << 1 / i << '\n';

    f = -1.0;
    i = quick_rsqrt(f);
    std::cout << "sqrt " << f << " = " << 1 / i << '\n';
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
	std::cout << "fegetenv() returned " << ret << '\n'; // 0 is no errors

	// pair function setting fenv_t
	ret = fesetenv(&fenv);
	std::cout << "fesetenv() returned " << ret << '\n'; // 0 is no errors

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
	std::cout << "log 0 = " << x << '\n';
	if (!fetestexcept(FE_ALL_EXCEPT))
		std::cout << "no exceptions raised" << '\n';
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

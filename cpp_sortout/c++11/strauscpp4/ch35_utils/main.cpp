#include <iostream>
#include <chrono>
#include <ratio>
#include <string>

using namespace std;

/*

New features:
* Type traits, such as is_integral and is_base_of (35.4)
* Time utilities: duration and time_point(35.2)
* Compile-time rational arithmetic: ratio(35.3)
* string to numeric value conversions (36.3.5)

Examples:
1. Duration (35.2.1), time_point (35.2.2)
2. Clocks (35.2.3), Time Traits (35.2.4)
3. Rational (35.3)
4. Type functions (35.4)
5. string to numeric value conversions (36.3.5)
*/

//1. Duration(35.2.1), time_point(35.2.2)
//2. Clocks(35.2.3), Time Traits(35.2.4)
void show_time()
{
    // Duration consists of a count of ticks of type Rep and a tick period, where the tick period
    // is a compile-time rational constant representing the number of seconds from one tick to the next
    using shakes = std::chrono::duration<int, std::ratio<1, 100000000>>;
    using jiffies = std::chrono::duration<int, std::centi>;

    using microfortnights = std::chrono::duration<float, std::ratio<12096, 10000>>;
    using nanocenturies = std::chrono::duration<float, std::ratio<3155, 1000>>;

    std::chrono::seconds sec(1);
    std::cout << "1 second is:\n";
    std::cout << std::chrono::duration_cast<shakes>(sec).count() << " shakes\n";
    std::cout << std::chrono::duration_cast<jiffies>(sec).count() << " jiffies\n";

    // A duration's period holds the number of clock ticks of the period
    std::cout << microfortnights(sec).count() << " microfortnights\n";
    std::cout << nanocenturies(sec).count() << " nanocenturies\n";

    // Literals defined in inline namespace std::literals::chrono_literals (since C++14)
    using namespace std::literals::chrono_literals;
    std::chrono::hours h1 = 1h;
    std::chrono::microseconds m1 = 1us;

    // time point
    auto tp = std::chrono::steady_clock::now();

    // days since start of epoch
    auto d1 = std::chrono::time_point_cast<std::chrono::hours>(tp).time_since_epoch().count() / 24;

    //a day's duration
    using days = std::chrono::duration<long, ratio<24 * 60 * 60, 1>>;
    auto d2 = std::chrono::time_point_cast<days>(tp).time_since_epoch().count(); // days since start of epoch
}


//3. Rational(35.3)
void show_ratio()
{
    std::ratio<1, 1000000> r1;
    // no any useful members
    //std::cout << r1 << std::endl;
    std::cout << r1.num << '/' << r1.den << std::endl;
}

//4. Type functions(35.4)
namespace cpp4
{

class A
{
public:
    int i = 0;
};

template <typename T, typename U>
struct decay_equiv :
    std::is_same<typename std::decay<T>::type, U>::type
{};

} // namespace cpp4 

void show_types()
{

    // Primary Type Predicates
    // is_void<X> is_integral<X> is_floating_point<X> is_array<X> is_pointer<X>
    // is_lvalue_reference<X> is_rvalue_reference<X> is_member_object_pointer<X> is_member_function_pointer<X> is_enum<X>
    // is_union<X> is_class<X> is_function<X>
    std::cout << std::is_floating_point<float>::value << '\n';

    // Composite Type Predicates
    // is_reference<X> is_arithmetic<X> is_fundamental<X> is_object<X> is_scalar<X> is_compound<X> is_member_pointer<X>
    std::cout << std::is_fundamental<float>::value << '\n';
    std::cout << std::is_compound<float>::value << '\n'; // !is_fundamental

    // Type property predicates
    // is_const<X> is_volatile<X> is_trivial<X> is_trivially_copyable<X>...
    std::cout << std::is_trivial<float>::value << '\n';
    std::cout << std::is_trivially_constructible<cpp4::A>::value << '\n';
    std::cout << std::is_trivially_move_assignable<cpp4::A>::value << '\n';

    // Type Property Queries
    // n=alignment_of<X>, n=rank<X>, n=extent<X,N>
    std::cout << std::alignment_of<cpp4::A>::value << '\n';

    // f X is an array, n is the number of dimensions; otherwise n==0
    std::cout << std::rank<int[1][2][3]>::value << '\n';
    std::cout << std::rank<int[][2][3][4]>::value << '\n';
    std::cout << std::rank<int>::value << '\n';

    // If X is an array, n is the number of elements in the Nth dimension; otherwise n==0
    std::cout << std::extent<int[3]>::value << '\n'; //< default dimension is 0
    std::cout << std::extent<int[3][4], 0>::value << '\n';
    std::cout << std::extent<int[3][4], 1>::value << '\n';
    std::cout << std::extent<int[3][4], 2>::value << '\n';
    std::cout << std::extent<int[]>::value << '\n';

    //Type Relations
    // is_same<X,Y>, is_base_of<X,Y,> is_convertible<X,Y>
    std::cout << std::is_convertible<int, double>::value << '\n';

    // const and volatile Modification
    // remove_const<X>, remove_volatile<X>, remove_cv<X>, add_const<X>, add_volatile<X>, struct add_cv<X>

    // Reference Modification
    // remove_reference<X>, add_lvalue_reference<X>, add_rvalue_reference<X>
    // decay<X> - The type passed by value for a function argument of type X

    // remove all references, remove array specifier, make function pointer
    // T&, T&&, const T -> T, [] -> *, ...
    std::cout << std::boolalpha
        << cpp4::decay_equiv<int, int>::value << '\n'
        << cpp4::decay_equiv<int&, int>::value << '\n'
        << cpp4::decay_equiv<int&&, int>::value << '\n'
        << cpp4::decay_equiv<const int&, int>::value << '\n'
        << cpp4::decay_equiv<int[2], int*>::value << '\n'
        << cpp4::decay_equiv<int(int), int(*)(int)>::value << '\n';
}

// 5. string to numeric value conversions (36.3.5)
void show_string_conv()
{
    // In <string>, the standard library provides a set of functions for extracting numeric values 
    // from their character representation in a string or wstring
    string s { "123.456" };

    int i = std::stoi(s);
    double d = std::stod(s);

    // The second argument of a stoxxx() function is a pointer used to indicate how far 
    // into the string the search for a numeric value progressed. For example:
    string ss = "123.4567801234";
    size_t dist = 0;
    auto x = stoi(ss, &dist);
    ++dist;
    auto y = stoll(&ss[dist]);

    //Initial whitespace is skipped
    s = " 123.45";
    auto x1 = stoi(s); // x1 = 123

    // If a conversion function doesn�t find characters in its string argument that it 
    // can convert to a number, it throws invalid_argument
    // If it finds a number that it cannot represent in its target type, it throws out_of_range; 
    // in addition, the conversions to floating-point types set errno to ERANGE
}

int main()
{
    show_time();
    show_ratio();
    show_types();
    show_string_conv();
    return 0;
}

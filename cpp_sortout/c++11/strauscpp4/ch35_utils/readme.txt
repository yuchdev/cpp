Questions:
chrono is still unclear

https://habrahabr.ru/post/259307/

New C++14 features:
* Type traits, such as is_integral and is_base_of (35.4)
* Time utilities: duration and time_point(35.2)
* Compile-time rational arithmetic: ratio(35.3)
* string to numeric value conversions (36.3.5)

Advices:
* All chrono facilities are in the std::chrono (sub)namespace
* If you want to know the current time_point, you can call now() for one of three clocks: 
  system_clock, steady_clock, high_resolution_clock
* These three clocks need not be different; a standard-library clock name may be an alias
* It turns out that time is far more complicated to deal with than we usually think. For example, 
  we have leap seconds, clocks that are not accurate and must be adjusted 
  (possibly causing time as reported by a clock to go backward), clocks of differing precision, etc
* In <chrono>, the standard library provides type duration to represent the time between two points 
* A duration’s period holds the number of clock ticks of the period
* duration_cast<D>(d) Convert duration d to duration type D;
  no implicit conversions are used for the representation or period
* The standard library provides some convenience aliases using the SI units from <ratio>
using nanoseconds = duration<si64,nano>; 
using microseconds = duration<si55,micro>; 
using milliseconds = duration<si45,milli>; 
using seconds = duration<si35>;
using minutes = duration<si29,ratio<60>>; 
using hours = duration<si23,ratio<3600>>;

* Here, siN means implementation-defined signed integer type of at least N bits
* In <chrono>, the standard library provides type time_point to represent a point in time of a given
  epoch as measured by a given clock
* An epoch is a range of time determined by a clock, measured in terms of a duration, starting at the duration::zero()
* The time_point operations that do not access a clock can be constexpr,
* Class system_clock represents ‘‘wall clock time’’ as obtained from a system’s real-time clock
* All clock data and function members are static. We don’t explicitly deal with clock objects
  Instead, we use clock types
* The common type of two durations is determined by computing their greatest common denominator (GCD)
* This implies that com- mon_type<R1,P1,R2,P2>::type can hold any value from duration<R1,R2> 
  and duration<R2,P2> without truncation error
* In words, to have a common_type, two time_points must have a common clock type
* The basic idea of std::ratio is to encode the numerator and denominator of a rational number 
  as (value) template arguments. The denominator must always be nonzero
* In <type_traits>, the standard library provides a large set of type functions that allow 
  to determine properties of a type or a pair of types. Their names are mostly self-explanatory
  * Primary Type Predicates (is_void<X>, is_integral<X>, is_floating_point<X>, is_array<X>,...)
  * Composite Type Predicates (is_reference<X>, is_arithmetic<X>, is_fundamental<X>, is_object<X>,...)
  * Type property predicates (is_const<X>, is_volatile<X>, is_standard_layout<X>, is_pod<X>,
    is_copy_assignable<X>, is_move_assignable<X>,...)
  * Type Property Queries (n=alignment_of<X>, n=rank<X>, n=extent<X,N>)
  * Type Relations (is_same<X,Y>, is_base_of<X,Y>, is_convertible<X,Y>)
  * const and volatile modification (remove_const<X>, remove_volatile<X>, add_const<X>, add_volatile<X>,...)
  * Reference Modification (remove_reference<X>, add_lvalue_reference<X>, add_rvalue_reference<X>,...)
  * Sign Modification (make_signed<X>, make_unsigned<X>)
  * Array Modification (remove_extent<X>, remove_all_extents<X>)
  * Pointer Modification (remove_pointer<X>, add_pointer<X>)
  * Alignments (aligned_storage<n,a>, aligned_storage<n>, aligned_union<n,X...>)
  * Other Transformations (enable_if<b,X>, conditional<b,T,F>, common_type<X>, underlying_type<X>, result_of<FX>)
* The pair of forward() functions are meant always to be available together, and selection between them 
  should be done by overload resolution. In that case, any lvalue goes to the first version and every rvalue to the second

* In <string>, the standard library provides a set of functions for extracting numeric values 
  from their character representation in a string or wstring
* The second argument of a stoxxx function is a pointer used to indicate how far into the string the search for a numeric value progressed
* If a conversion function doesn’t find characters in its string argument that it can convert to a number, 
  it throws invalid_argument. If it finds a number that it cannot represent in its target type, it throws out_of_range; 
  in addition, the conversions to floating-point types set errno to ERANGE
* The stoxxx functions encode their target type in their names. This makes them unsuitable 
  for generic code where the target can be a template parameter. In such cases, consider to<X>

Book advices: 
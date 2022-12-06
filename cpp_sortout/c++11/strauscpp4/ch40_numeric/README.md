Questions:
* gslice/matrix is still unclear (back after linear algebra recall)
* distributions (back after math. stat recall)
* given vector of unit coordinates. Find 10 closest with boundary ceses processing (complex)

New C++14 features:
* Random numbers: uniform_int_distribution, normal_distribution, random_engine, etc (40.7)


1. slice explanation
Constructs a new slice with parameters start, size, stride
This slice will refer to size number of elements, each with the position:
 start + 0*stride
 start + 1*stride
 ...
 start + (size-1)*stride

2. gslice explanation
 start + ... all of sum(permutation(lengths, strides))

For example, a gslice with starting index 3, strides {19,4,1} and lengths {2,4,3} generates the following set of indices:
3 + 0*19 + 0*4 + 0*1 = 3,
3 + 0*19 + 0*4 + 1*1 = 4,
3 + 0*19 + 0*4 + 2*1 = 5,
3 + 0*19 + 1*4 + 0*1 = 7,
3 + 0*19 + 1*4 + 1*1 = 8,
...
3 + 1*19 + 3*4 + 2*1 = 36
It is possible to construct std::gslice objects that select some indices more than once: 
if the above example used the strides {1,1,1} , the indices would have been {3, 4, 5, 4, 5, 6, ...}
Such gslices may only be used as arguments to the const version of std::valarray::operator[], otherwise the behavior is undefined.

Advices:
* To allow the programmer to best take advantage of hardware, limits properties are implementation-defined 
  rather than fixed by the rules of the language itself
* the general numeric_limits template is simply a notational handle for a set of constants and constexpr functions
* Each implementation of the standard library provides a specialization of numeric_limits for each fundamental numeric type
* From C, C++ inherited macros that describe properties of integers. They are found in <climits>
* Similarly, <cfloat> and <float.h> define macros describing properties of floating-point numbers
* In <cmath> we find what are commonly called the standard mathematical functions
  There are versions taking float, double, long double, and complex
* Errors are reported by setting errno from <cerrno> to EDOM for a domain error and to ERANGE for a range error
* For historical reasons, a few mathematical functions are found in <cstdlib> rather than in <cmath> (abs/labs/div/ldiv)
* There is a separate ISO standard for special mathematical functions [C++Math,2010]. An implementation may add these functions to <cmath>
  (assoc_laguerre(), assoc_legendre(), beta(), comp_ellint_2(), comp_ellint_3(), cyl_bessel_i(), ...)
* The standard library provides complex number types complex<float>, complex<double>, and complex<long double>
  A complex<Scalar> where Scalar is some other type supporting the usual arithmetic operations usually works 
  but is not guaranteed to be portable
* The standard-library complex does not protect against narrowing
complex<float> z1 = 1.33333333333333333; // narrows 
complex<double> z2 = 1.33333333333333333;// narrows 
z1=z2; //narrows

* To protect against accidental narrowing, use {} initialization: 
complex<float> z3 {1.33333333333333333}; // error : narrowing conversion

* valarray vectors are well supported by high-performance machine architectures, libraries relying on such vectors 
  are in wide use, and very aggressive optimization of code using such vec- tors is considered essential in many fields
* It provides the usual numeric vector arithmetic operations for an array type plus support for slices and strides
* For const arguments, the result contains copies of elements. For non-const arguments, the result holds references to elements
* If the lengths of the two arrays are not the same, the result of a binary operator on two valarrays is undefined
* Note that the non-assignment operations construct a new valarray
* A slice is an abstraction that allows us to manipulate a one-dimensional array (e.g., a built-in array, a vector, or a valarray)
  efficiently as a matrix of arbitrary dimension
* A stride is the distance (in number of elements) between two elements of the slice
* Following the usual C/C++ conventions, the valarray is laid out in memory with row elements first (row-major order) and contiguous
* One way of thinking of a slice is as an odd kind of iterator: a slice allows us to describe a sequence of indices for a valarray
* From a valarray and a slice, we can build something that looks and feels like a valarray but is really
  simply a way of referring to the subset of the array described by the slice: slice_array
* A user cannot directly create a slice_array. Instead, the user subscripts a valarray to create a slice_array for a given slice
* A slice_array can be copied
* A gslice is a 'generalized slice' that contains (almost) the information from n slices (see explanation above)
* The gslice_array offers the same set of members as slice_array. A gslice_array is the of using a gslice as the subscript of a valarray

* Numeric algorithms provide general versions of common operations on sequences of numerical values
* The simple version of accumulate() adds elements of a sequence using their + operator
* inner_product(): accumulating from a sequence is very common, and accumulating from a pair of sequences is not uncommon
* The partial_sum() and adjacent_difference() algorithms are inverses of each other and deal with the notion of incremental change
  Given a sequence a, b, c, d, etc, adjacent_difference() produces a, b-a, c-b, d-c
  partial_sum turns the sequence a, b, c, d into a, a+b, a+b+c, a+b+c+d
* random numbers are sequences of values produced according to mathematical formulas, rather than unguessable (truly random)
  numbers that could be obtained from a physical process, such as radioactive decay or solar radiation. If the implementation 
  has such a truly random device, it will be represented as a random_device.
* Four kinds of entities are provided:
	* A uniform random number generator
	* A random number engine is a uniform random number generator that can be created with a default state E{}
	  or with a state determined by a seed E{s}.
	* A random number engine adaptor (an adaptor) is a random number engine that takes values 
	  produced by some other random number engine and applies an algorithm to those values
	* A random number distribution (a distribution) is a function object returning values that are distributed 
	  according to an associated mathematical probability density function p(z)
* In simpler terms, the users terms, a random number generator is an engine plus a distribution
  The engine produces a uniformly distributed sequence of values, and the distribution bends 
  those into the desired shape (distribution)
* One important use of random numbers is for sampling algorithms. In such algorithms we need to choose a sample 
  of some size from a much larger population
* Standard Random Number Engines:
	* default_random_engine
	* linear_congruential_engine
	* mersenne_twister_engine
	* subtract_with_carry_engine
* Use the default_random_engine unless you have a real need and know what you are doing
* A random number engine adaptor takes a random number engine as an argument and produces a new random number engine 
  with different randomness properties
	* discard_block_engine
	* independent_bits_engine
	* shuffle_order_engine
* A few aliases are defined for useful engines
using minstd_rand0 = linear_congruential_engine<uint_fast32_t, 16807, 0, 2147483647>; 
using minstd_rand = linear_congruential_engine<uint_fast32_t, 48271, 0, 2147483647>; 
using mt19937 = mersenne_twister_engine<...

* If an implementation is able to offer a truly random number generator, that source of random numbers 
  is presented as a uniform random number generator called random_device
* A random number distribution is a function object that, when called with a random number generator argument, 
  produces a sequence of values of its result_type
	* The precondition field specifies requirements on the distribution arguments
	* The default field specifies default arguments
	* The result field specifies the range of the results
* In <cstdlib> and <stdlib.h>, the standard library provides a simple basis for the generation of random numbers:
int rand();
void srand(unsigned int i);

* In particular, the low-order bits of a random number are often suspect, so rand()%n is not a good portable way 
  of generating a random number between 0 and n-1. Often, int((double(rand())/RAND_MAX)*n) gives acceptable results
  However, for serious applications, generators based on uniform_int_distribution will give more reliable results
* A call srand(s) starts a new sequence of random numbers from the seed

Book advices: 
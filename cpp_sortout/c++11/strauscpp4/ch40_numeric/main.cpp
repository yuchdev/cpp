#include <iostream>
#include <numeric>
#include <valarray>
#include <cmath>
#include <complex>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <random>
#include <map>

using namespace std;

/*

New features:
* Random numbers: uniform_int_distribution, normal_distribution, random_engine, etc (40.7)

Examples:
1. Limits
2. valarray
3. Slices (+iterator)
4. gslice
5. Matrix
6. valarray mask, indirect valarray
7. complex
8. Numeric algorithm
9. Random generators
10. Special math function if exists

*/

//1. Limits
void show_limits()
{

    // limits examples

    if (numeric_limits<unsigned char>::digits != 8) {
        cout << "You have so strange bytes (!= 8 bit)\n";
    }

    int i = 65535;
    if (i < numeric_limits<unsigned char>::min() || numeric_limits<unsigned char>::max() < i) {
        cout << "i too long for short type\n";
    }

    // Check for double epsilon
    double d = 1.0;
    if (d > 0 && d < numeric_limits<double>::epsilon()) {
        d = 0.0;
    }

    if (numeric_limits<long>::is_specialized) {
        cout << "long type has numeric_limits specialization\n";
    }

    if (numeric_limits<long>::is_signed && numeric_limits<long>::is_integer) {
        cout << "long is integer and signed type\n";
    }
    // also contains many double descriptors: (has_NaN, has_infinity, ...)
}


//2. valarray
namespace cpp4
{
double my_square(double d)
{
    return d * d;
}
} // namespace cpp4

void show_valarray()
{

    // valarray - is a numeric-optimized vector
    // it is designed for fast low-level operations

    // Construct:
    // 1. by default
    valarray<double> v0;

    // 2. 1000 elements
    valarray<double> v1(1000);

    // 3. Unlike vector, default value is a 1st param
    valarray<double> v2(1.9, 1000);

    // from array or initializer_list
    double arr[] = { 1.0, 2.0, 3.0, 1.5, 1.9 };

    // no bound check!
    valarray<double> v3(arr, 5);

    // Assignment - no bound check!
    valarray<double> v4(5);
    v4 = v3;

    // assign to every elem
    v2 = 1.0;

    // access w/o check
    double d = v2[0];

    // operations - to every elem
    v3 *= 2.0;

    // accumulate, sum, etc
    double s = v4.sum();
    double m = v4.max();
    m = v4.min();

    // shift left by 2
    valarray<double> v5 = v4.shift(2);

    // cyclic shift by 3
    valarray<double> v6 = v4.cshift(-3);

    // in any case new vector is generated
    // (old one is immutable)

    // size could be changed manually
    // all data is being lost
    v6.resize(10);

    // apply some f() to every element
    // new vector is generated
    valarray<double> v7 = v5.apply(cpp4::my_square);

    // all arithmetic operations work per elements
    // and create a new vector
    valarray<double> v8 = v3 * v4;
    valarray<double> v9 = cos(v8);
}


//3. Slices(+iterator)
namespace cpp4
{

// valarray slice iterator (Straus 4 p.1173)
template <typename T>
class slice_iter
{
public:

    slice_iter(std::valarray<T>& v, std::slice s) :
        _vector(v),
        _slice(s),
        _curr()
    {
    }

    slice_iter end()
    {
        slice_iter end_slice = *this;
        end_slice._curr = _slice.size();
        return end_slice;
    }

    slice_iter& operator++()
    {
        ++_curr;
        return *this;
    }

    slice_iter operator++(int)
    {
        slice_iter t = *this;
        ++_curr;
        return t;
    }

    // [] indexing
    T& operator[](size_t i) { return ref(_curr = i); }

    T& operator()(size_t i) { return ref(_curr = i); }

    T& operator*() { return ref(_curr); }


private:
    // vector we iterate over
    std::valarray<T>& _vector;

    std::slice _slice;

    // current element index
    size_t _curr;

    T& ref(size_t s) const
    {
        return _vector[_slice.start() + s * _slice.stride()];
    }

public:
    // binary operators
    friend bool operator == (const slice_iter& v1, const slice_iter& v2)
    {
        return v1._curr == v2._curr
            && v1._slice.stride() == v2._slice.stride()
            && v1._slice.start() == v2._slice.start();
    }

    friend bool operator != (const slice_iter& v1, const slice_iter& v2)
    {
        return !(v1 == v2);
    }

    friend bool operator < (const slice_iter& v1, const slice_iter& v2)
    {
        return v1._curr < v2._curr
            && v1._slice.stride() == v2._slice.stride()
            && v1._slice.start() == v2._slice.start();
    }

};

} // namespace cpp4

void show_slices()
{

    // std::slice is a slice of eevery n-th elements of vector
    // could be used for matrix arithmetic
    // ex: 3x4 matrix
    valarray<int> foo(12);
    for (int i = 0; i < 12; ++i)
        foo[i] = i;
    // 0 4 8
    // 1 5 9
    // 2 6 10
    // 3 7 11

    // 1 matrix row
    slice s = std::slice(0, 3, 4);
    cout << "slice with start = " << s.start()
        << " stride = " << s.stride()
        << " size = " << s.size() << endl;
    valarray<int> bar1 = foo[s];

    // output by slices (i.e. rows)
    cout << "Matrix: " << endl;
    for (size_t i = 0; i < 4; ++i) {
        // beginning is i-th elements, step (stride) 4, size 3
        slice s = std::slice(i, 3, 4);

        // valarray slice, i.e. matrix string
        valarray<int> bar = foo[s];
        for (size_t n = 0; n < bar.size(); n++)
            cout << bar[n] << ' ';
        cout << endl;
    }
    cout << endl;

}

void show_slices_iterator()
{
    // slices could be used for matrix iteration
    // see Straus 4 p.1173

    // valarray-matrix from previous example
    valarray<int> foo(12);
    for (int i = 0; i < 12; ++i)
        foo[i] = i;

    // slice for rows/column iteration
    slice s = std::slice(0, 3, 4);

    cpp4::slice_iter<int> s_iter(foo, s);

    ++s_iter;
    double d = (*s_iter);

    ++s_iter;
    d = (*s_iter);

    // also could return slices, not elements
}


//4. gslice
void show_gslice()
{

    // general slice (gslice) could represent custom slice of valarray
    // i.e. angular matrix

    // gslice contains n strides and sizes

    // valarray-matrix from prev example
    valarray<int> foo(12);
    for (int i = 0; i < 12; ++i)
        foo[i] = i;
    // 0 4 8
    // 1 5 9
    // 2 6 10
    // 3 7 11

    // 2-3 represent dimensions 
    // 3-1 represent strides

    // 2 elements in the first dimension
    // 3 elements in the second dimension
    valarray<size_t> lengths { 2, 3 };

    // 3 is the stride for the first index
    // 1 is the stride for the second index
    valarray<size_t> strides { 3, 1 };

    // gslice with 2 pairs represent 2-dimensions
    gslice gs(0, lengths, strides);

    valarray<int> bar = foo[gs];
    // 0 4
    // 1 5
    // 2 6
}


//5. Matrix
namespace cpp4
{

// Based on valarray and slices
template <typename T>
class matrix
{
public:

    matrix(size_t d1, size_t d2) :_d1(d1), _d2(d2), _val(d1* d2) {}

    matrix(const matrix& rhs) :_d1(rhs._d1), _d2(rhs._d2), _val(rhs._val) {}

    matrix(const matrix&& rhs) :_d1(rhs._d1), _d2(rhs._d2), _val(std::move(rhs._val)) {}

    const matrix& operator=(const matrix& rhs)
    {
        _d1 = rhs._d1;
        _d2 = rhs._d2;
        _val = rhs._val;
    }

    const matrix& operator=(const matrix&& rhs)
    {
        _d1 = rhs._d1;
        _d2 = rhs._d2;
        _val = std::move(rhs._val);
    }

    // non-virtual destructor, not intended to be inherited
    ~matrix() {}

    // accessors
    size_t size() const { return _val; }
    size_t dim1() const { return _d1; }
    size_t dim2() const { return _d2; }

    // row-column iterators
    slice_iter<T> row(size_t i)
    {
        return slice_iter<T>(_val, std::slice(i, _d2, _d1));
    }

    slice_iter<T> column(size_t i)
    {
        return slice_iter<T>(_val, std::slice(i * _d2, _d2, 1));
    }

    const slice_iter<T> row(size_t i) const
    {
        return slice_iter<T>(_val, std::slice(i, _d2, _d1));
    }

    const slice_iter<T> column(size_t i) const
    {
        return slice_iter<T>(_val, std::slice(i * _d2, _d2, 1));
    }

    // access elements
    T& operator()(size_t x, size_t y)
    {
        return row(x)[y];
    }

    T operator()(size_t x, size_t y) const
    {
        return row(x)[y];
    }

private:
    std::valarray<T> _val;
    size_t _d1;
    size_t _d2;
};

} // namespace cpp4 

void show_matrix()
{

    cpp4::matrix<double> m1(3, 3);
    cpp4::matrix<double> m2(m1);

    m1(0, 0) = 1.0;
    m1(1, 1) = 2.0;
    m1(2, 2) = 3.0;

    cpp4::slice_iter<double> s_iter1 = m1.row(0);
    cout << "First row: ";
    double d1 = *s_iter1;
    cout << d1;

    d1 = *(++s_iter1);
    cout << d1;

    d1 = *(++s_iter1);
    cout << d1;

    cout << endl;

    cpp4::slice_iter<double> s_iter2 = m1.column(1);
    cout << "Second column: ";
    double d2 = *s_iter2;
    cout << d2;

    d2 = *(++s_iter2);
    cout << d2;

    d2 = *(++s_iter2);
    cout << d2;

    cout << endl;
}

//6. slice_array, valarray mask, indirect valarray
void show_slice_array()
{

    // slice_array allows to represent any subset of valarray

    double arr[] = { 1.0, 2.0, 3.0, 1.5, 1.9 };
    valarray<double> v(arr, 5);
    size_t half_size = v.size() / 2;

    // represent even and odd elements
    const slice_array<double>& v_even = v[slice(0, half_size + 1, 2)];
    const slice_array<double>& v_odd = v[slice(0, half_size, 2)];

    // slice_array and valarray could be used for modeling a matrix (see below)
    // matrices could support lazy evaluation (see Josuttis book examples)
}

void show_valarray_mask()
{

    double arr[] = { 1.0, 2.0, 3.0, 1.5, 1.9 };
    valarray<double> v(arr, 5);

    // mask_array creates a bit-mask for valarray
    bool b[] = { true, false, true, false, true };
    valarray<bool> mask(b, 5);

    // filtering by mask (only 'true')
    valarray<double> v1 = v[mask];
}

void show_indirect_array()
{

    // indirect_array creates valarray subsets and re-arrange elements
    double arr[] = { 1.0, 2.0, 3.0, 1.5, 1.9 };
    valarray<double> v(arr, 5);

    // create and indirect_array for rearranging elements
    // inverse order
    size_t s[] = { 4,3,2,1,0 };
    valarray<size_t> mask(s, 5);

    // get element log() from [4] to [0]
    valarray<double> v1 = log(valarray<double>(v[mask]));
}

//7. complex
void show_complex()
{

    // complex numbers
    // contain several specializations (float, double, long double)
    // for optimization purposes

    complex<double> c1(1.0, 2.0);
    complex<double> c2(1.0, 2.0);

    // classic operations
    complex<double> c3 = c1 + c1;
    complex<double> c4 = c1 * c1;

    // complex output
    cout << "c1 = " << c1 << endl;
    cout << "c2 = " << c2 << endl;
    cout << "c3 = " << c3 << endl;
    cout << "c4 = " << c4 << endl;

    // coordinates
    double r = c1.real();
    double im = c1.imag();

    // adjoint complex
    complex<double> c5 = conj(c1);
    cout << "c5 = " << c5 << endl;

    // polar coordinates(rho, theta)
    // real = rho * cos(theta);
    // imag = rho * sin(theta);
    complex<double> c6 = std::polar(2.0, 0.5);
    cout << "c6 = " << c6 << endl;
    double rho_ = abs(c6);
    double theta_ = arg(c6);
    double norm_ = norm(c6);

    // complex overload
    complex<double> sin_cmplx = std::sin(c6);
    cout << sin_cmplx << endl;
}

//8. Numeric algorithm
void show_algorithms()
{

    // accumulation
    std::vector<int> v1 = { 0, 1, 2, 3, 4, 5, 5, 4, 3, 2, 1, 0 };
    int acc_sum = std::accumulate(v1.begin(), v1.end(), 0);
    int acc_mul = std::accumulate(v1.begin(), v1.end(), 1, std::multiplies<int>());
    cout << "Sum accumulated = " << acc_sum << " product accumulated = " << acc_mul << endl;

    // accumulation from two sequences
    std::vector<int> v2 = { 5, 4, 3, 2, 1, 1, 2, 3, 4, 5, 0, 0 };
    acc_sum = std::inner_product(v1.begin(), v1.end(), v2.begin(), 0);
    acc_mul = std::inner_product(v1.begin(), v1.end(), v2.begin(),
        1, std::multiplies<int>(), std::multiplies<int>()); // 1 op1 (v1 op2 v2)
    cout << "Sum accumulated = " << acc_sum << " product accumulated = " << acc_mul << endl;

    // incremental change

    // adjacent_difference : a, b-a, c-b, d-c, ...
    std::vector<int> res1;
    std::adjacent_difference(v1.begin(), v1.end(), std::back_inserter(res1));

    // partial_sum : a, a+b, a+b+c, ...
    std::vector<int> res2;
    std::partial_sum(v1.begin(), v1.end(), std::back_inserter(res2));
}

//9. Random generators
namespace cpp4
{

// Print integer distribution using ACSII
template <typename Generator>
void print_distribution(size_t elements, Generator& g)
{

    std::map<int, size_t> distr;

    for (size_t i = 0; i < elements; ++i) {
        ++distr[g()];
    }

    // write out a bar graph cout << i << '\t';
    for (auto& mypair : distr) {
        for (int j = 0; j != mypair.second; ++j) cout << '*';
        cout << endl;
    }
}

// Examples of uniform distributions
class rand_int
{
public:

    rand_int(int lo, int hi) : p { lo, hi } {}

    int operator()()
    {
        return generator();
    }
private:

    // uniform distribution accepts range (see ctor)
    std::uniform_int_distribution<>::param_type p;

    // distribution has operator() that accepts engine, so we can bind to distribution
    std::function<int()> generator = std::bind(std::uniform_int_distribution<>{p}, std::default_random_engine {});
};


// Double uniform distribution is the same
class rand_double
{
public:

    rand_double(double lo, double hi) : p { lo, hi } {}

    double operator()()
    {
        return generator();
    }

private:
    std::uniform_real_distribution<>::param_type p;
    std::function<double()> generator = std::bind(std::uniform_real_distribution<>{p}, std::default_random_engine {});
};

// Double uniform distribution is the same
class rand_double_normal
{
public:

    rand_double_normal(double lo, double hi) : lo_ { lo }, hi_ { hi }, norm { 5.0, 2.0 } {}

    double operator()()
    {
        double rnd = generator();
        while ((rnd < lo_) || (rnd > hi_)) {
            rnd = generator();
        }
        return rnd;
    }

private:
    double hi_ = 0.0;
    double lo_ = 0.0;
    std::normal_distribution<double> norm;
    std::function<double()> generator = std::bind(norm, std::default_random_engine {});
};

} // namespace cpp4

void show_random()
{

    // Use the default_random_engine unless you have a real need and know what you are doing

    cpp4::rand_int int_gen(1, 20);
    cpp4::rand_double dbl_gen(1, 20);
    cpp4::rand_double_normal dbl_normal_gen(1, 20);

    cout << "Uniform int: \n";
    cpp4::print_distribution(200, int_gen);

    cout << "Uniform double: \n";
    cpp4::print_distribution(200, dbl_gen);

    cout << "Normal double: \n";
    cpp4::print_distribution(200, dbl_normal_gen);

    std::random_device rd;
    cout << "Entropy: " << rd.entropy() << '\n';
}

// 10. Special math function if exists
void show_special()
{

#if defined(_WIN32) || defined(_WIN64)

#else
    assoc_laguerre();
    assoc_legendre();
    beta();
    comp_ellint_2();
    comp_ellint_3();
    cyl_bessel_i();
    cyl_bessel_k();
    cyl_neumann();
    ellint_1();
    ellint_3();
    expint();
    hermite();
    legendre();
    riemann_zeta();
    sph_bessel();
    sph_neumann();

    comp_ellint_1();
    cyl_bessel_j();
    ellint_2();
    laguerre();
    sph_legendre();
#endif
}

int main()
{
    show_limits();
    show_valarray();
    show_slices();
    show_slices_iterator();
    show_matrix();
    show_gslice();
    show_valarray_mask();
    show_indirect_array();
    show_complex();
    show_algorithms();
    show_random();
    return 0;
}

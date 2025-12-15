#include "complex.h"

// Aany expression can be passed in the initialization list
// In our case new complex_t*()
complex_matrix::complex_matrix(const size_t count_x)
    : _count_x(count_x)
    , _count_y()
    , _matrix(new complex_t* ())
{

    _matrix[0] = new complex_t[count_x];
}

complex_matrix::complex_matrix(const size_t count_x, const size_t count_y)
    : _count_x(count_x)
    , _count_y(count_y)
    , _matrix(new complex_t* [count_y])
{

    for (unsigned i = 0; i < count_y; i++) {
        _matrix[i] = new complex_t[count_x];
    }
}

complex_matrix::complex_matrix(const complex_matrix& c)
    : _count_x(c._count_x)
    , _count_y(c._count_y)
    , _matrix(new complex_t* [c._count_y])
{

    for (unsigned i = 0; i < c._count_y; i++) {
        _matrix[i] = new complex_t[c._count_x];

        for (unsigned j = 0; j < c._count_x; j++) {
            _matrix[i][j] = c(i, j);
        }
    }
}

// This is a classic case of the assignment constructor
// It explains assignment logic well, but it is quite ineffective
#ifdef _NO_SWAP
complex_matrix& complex_matrix::operator =(const complex_matrix& c)
{

    // check self-assignment
    if (this != &c) {

        // the new dimensions
        size_t new_size_x = c._count_x;
        size_t new_size_y = c._count_y;

        // allocated memory for the new dimensions
        complex_t** new_matrix = new complex_t * [c._count_y];
        for (unsigned i = 0; i < c._count_y; i++) {
            new_matrix[i] = new complex_t[c._count_x];

            for (unsigned j = 0; j < c._count_x; j++) {
                new_matrix[i][j] = c(i, j);
            }
        }
        // cleared out the old memory
        clear();

        // aquired ownership of a new memory
        _matrix = new_matrix;

    }
    return *this;
}
#else
// This version is more effective, as all the work on memory allocation and clearing 
// is being performed by the constructor and the clear() function
complex_matrix& complex_matrix::operator =(const complex_matrix& c)
{

    // Checking in the self-assignment, as it is possible that resources will already be released,
    // and there would be nothing to assign
    // There's also an option of comparing objects using `operator==()` instead of pointers, this may increase efficiency
    // in case of pools of objects it is possible to compare by an ID
        // This could be actual for any other functions that accept reference to itself
    if (this == &c) {
        return *this;
    }

    complex_matrix* tmp = new complex_matrix(c);
    clear();
    _matrix = tmp->_matrix;
    return *this;
}
#endif // _NO_SWAP



complex_matrix::~complex_matrix()
{
    clear();
}

void complex_matrix::clear()
{
    for (unsigned i = 0; i < _count_y; i++) {
        delete[] _matrix[i];
    }
    delete[] _matrix;
    _matrix = nullptr;
}

// 2 versions of indexing operators
complex_t* complex_matrix::operator[](unsigned index)
{
    return _matrix[index];
}

complex_t& complex_matrix::operator()(unsigned index_x, unsigned index_y)
{
    return _matrix[index_x][index_y];
}

const complex_t* complex_matrix::operator[](unsigned index) const
{
    return _matrix[index];
}

const complex_t& complex_matrix::operator()(unsigned index_x, unsigned index_y) const
{
    return _matrix[index_x][index_y];
}

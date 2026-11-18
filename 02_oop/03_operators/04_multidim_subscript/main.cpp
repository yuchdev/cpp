// Requires C++23 (multidimensional operator[], P2128). This target sets
// its own CXX_STANDARD in CMakeLists.txt rather than the project-wide
// C++20, since this is specific to this one example.
#include <cassert>
#include <vector>

// Before C++23, operator[] accepted exactly one subscript -- the
// language simply had no grammar for `m[i, j]` as a single call to one
// function. 02_complex_number/complex_matrix.h works around this the
// two historical ways: operator()(i, j) (a genuinely different operator,
// just one that happens to accept two arguments), or operator[](i)
// returning an intermediate row pointer so a SECOND, ordinary
// operator[](j) can be chained onto it (m[i][j]). Both are workarounds
// for a gap in operator[] itself, not in the language's expressiveness
// generally -- operator() and a function call never had an arity limit.
class matrix_2d
{
public:
    matrix_2d(unsigned rows, unsigned cols) :
        rows_(rows), cols_(cols), data_(static_cast<std::size_t>(rows) * cols)
    {
    }

    // operator[] can now take more than one subscript directly: this is
    // ONE function, not two chained calls and not a differently-named
    // operator pressed into service for the job.
    double& operator[](unsigned row, unsigned col)
    {
        return data_[static_cast<std::size_t>(row) * cols_ + col];
    }

    double operator[](unsigned row, unsigned col) const
    {
        return data_[static_cast<std::size_t>(row) * cols_ + col];
    }

    unsigned rows() const { return rows_; }
    unsigned cols() const { return cols_; }

private:
    unsigned rows_, cols_;
    std::vector<double> data_;
};

void show_multidim_subscript()
{
    matrix_2d m(2, 3);

    // A single operator[] call, two subscripts -- not operator()(i, j),
    // not m[i][j] through an intermediate row object.
    m[0, 0] = 1.0;
    m[0, 1] = 2.0;
    m[1, 2] = 9.0;

    assert(m[0, 0] == 1.0);
    assert(m[0, 1] == 2.0);
    assert(m[1, 2] == 9.0);
    assert(m[1, 0] == 0.0); // default-initialized by std::vector<double>'s constructor

    const matrix_2d& view = m;
    assert(view[1, 2] == 9.0); // the const overload reads the same storage
}

int main()
{
    show_multidim_subscript();
    return 0;
}

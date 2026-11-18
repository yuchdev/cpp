// Requires C++23 (explicit object parameters / "deducing this", P0847).
// This target sets its own CXX_STANDARD in CMakeLists.txt rather than the
// project-wide C++20, since this is specific to this one example.
#include <cassert>
#include <type_traits>
#include <utility>

namespace cpp
{

// Before deducing this, giving one member both read and write access
// through const/non-const call sites (see 03_constness's accessor_pair)
// meant writing one overload per combination of constness and value
// category that mattered to you:
//     const int& at() const &;
//     int& at() &;
//     int&& at() &&;          // lets a caller move out of a temporary
// -- all doing the same thing, differing only in the type `self`/`this`
// is treated as. An explicit object parameter collapses the whole family
// into one function template, with the compiler deducing which case
// applies from the actual call site.
class accessor_pair_deduced
{
public:
    explicit accessor_pair_deduced(int v) : value_(v) {}

    template <typename Self>
    auto&& at(this Self&& self)
    {
        return std::forward<Self>(self).value_;
    }

private:
    int value_;
};

} // namespace cpp

void show_deducing_this_collapses_overload_family()
{
    cpp::accessor_pair_deduced mutable_obj(1);
    const cpp::accessor_pair_deduced const_obj(2);

    // Called through a non-const lvalue: Self deduces to
    // accessor_pair_deduced&, so at() returns int& -- writable.
    auto& ref = mutable_obj.at();
    static_assert(std::is_same_v<decltype(ref), int&>);
    ref = 99;
    assert(mutable_obj.at() == 99);

    // Called through a const lvalue: Self deduces to const
    // accessor_pair_deduced&, so at() returns const int&.
    const auto& cref = const_obj.at();
    static_assert(std::is_same_v<decltype(cref), const int&>);
    assert(cref == 2);

    // Called through an rvalue (a temporary): Self deduces to
    // accessor_pair_deduced BY VALUE (exactly like a forwarding-reference
    // template parameter would), which makes Self&& an rvalue reference
    // to that same temporary -- no copy is made. at() then returns
    // int&&, letting the caller move the value out instead of copying it.
    auto&& moved = cpp::accessor_pair_deduced(3).at();
    static_assert(std::is_same_v<decltype(moved), int&&>);
    assert(moved == 3);
}

int main()
{
    show_deducing_this_collapses_overload_family();
    return 0;
}

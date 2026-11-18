#include <cassert>
#include <iostream>

namespace cpp
{

class SimpleClass
{
public:
    SimpleClass(char* const cp1) : cp1(cp1) {}

    // A static data member is not part of any particular object's state,
    // so a const member function -- which only promises not to mutate
    // *this -- is free to change one. The object genuinely hasn't moved.
    void set_static(int i) const
    {
        a_stat = i;
    }

private:
    friend class do_not_exist; // a friend declaration of a non-existent class is simply ignored

    static int a6;

    // Pre-C++11 stand-in for an in-class integer constant: consumes no
    // storage, needs no out-of-class definition. Prefer `static constexpr`
    // in new code; recognize this when reading older code.
    enum { c1 = 5, c2 = 7, c3 = 12 };

    mutable int a8 = 0;

    // "Read backward" settles any ambiguity about which part is const:
    char const* cp2 = nullptr; // pointer (*) to 'const' 'char'   -- pointee is const
    const char* cp3 = nullptr; // pointer (*) to 'char' 'const'   -- identical to cp2, different spelling
    char* const cp1;           // 'const' pointer (*) to 'char'   -- the pointer itself is const
    static int a_stat;
};

int SimpleClass::a_stat = 0;

} // namespace cpp

// Const and non-const overloads are two DIFFERENT functions. Overload
// resolution picks between them using only the constness of the object
// expression the call is made through -- not the constness of anything
// the function happens to return, and not any property of the caller.
namespace cpp
{

class accessor_pair
{
public:
    explicit accessor_pair(int v) : value_(v) {}

    const int& at() const
    {
        last_call_was_const = true;
        return value_;
    }

    int& at()
    {
        last_call_was_const = false;
        return value_;
    }

    static inline bool last_call_was_const = false;

private:
    int value_;
};

} // namespace cpp

void show_const_overload_resolution()
{
    cpp::accessor_pair mutable_obj(1);
    const cpp::accessor_pair const_obj(2);

    mutable_obj.at();
    assert(!cpp::accessor_pair::last_call_was_const); // non-const object -> non-const overload

    const_obj.at();
    assert(cpp::accessor_pair::last_call_was_const); // const object -> const overload

    // And a fact that is easy to forget: declaring one non-const
    // operator= does NOT make the compiler synthesize a const-qualified
    // one alongside it. Const-correctness of hand-written members has to
    // be earned explicitly, every time you write one by hand.
}

// Bitwise constness is not the same thing as conceptual constness. The
// compiler enforces only the former.
namespace cpp
{

struct Point
{
    int x {};
    int y {};
};

class Polygon
{
public:
    explicit Polygon(int num_points) : points_(new Point[num_points]) {}
    ~Polygon() { delete[] points_; }

    void set_point(int index, int x, int y)
    {
        points_[index].x = x;
        points_[index].y = y;
    }

    // This function is bitwise const: it never reseats `points_` itself
    // (points_ is `Point* const`, a const POINTER). But `points_` is not
    // `const Point*` -- what it points to is not const -- so nothing
    // stops this "const" function from mutating the pointee. It compiles
    // cleanly while blatantly violating conceptual constness.
    Point get_point_violating_constness(int index) const
    {
        points_[index].x = 0; // mutates through a const member function
        return points_[index];
    }

private:
    Point* const points_;
};

} // namespace cpp

void show_bitwise_vs_conceptual_constness()
{
    const cpp::Polygon poly(3);
    cpp::Point before = poly.get_point_violating_constness(0);
    cpp::Point after = poly.get_point_violating_constness(0);
    // Both calls ran through a const Polygon, yet the object's observable
    // state changed between them -- "x" was overwritten to 0 as a side
    // effect of a function that promised, by its signature, not to
    // mutate *this.
    (void)before;
    (void)after;
}

int main()
{
    char buf[] = "example";
    cpp::SimpleClass sc(buf);
    sc.set_static(5);

    show_const_overload_resolution();
    show_bitwise_vs_conceptual_constness();
    return 0;
}

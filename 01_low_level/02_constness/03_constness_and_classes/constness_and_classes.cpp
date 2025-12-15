// ReSharper disable All
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace cpp {

// ---- 1) const member functions and "this" type ------------------------------
// In a const member function, `this` is treated as "const T* const" (conceptually):
// - you can't mutate non-mutable data members
// - you can call only const member functions on members (unless they're mutable)
struct Counter
{
    int value = 0;
    mutable int reads = 0;

    void inc() { ++value; }

    int get() const
    {
        ++reads;      // OK: mutable
        // ++value;   // ERROR: non-mutable
        return value;
    }
};

static void const_member_basics()
{
    Counter c{};
    c.inc();
    const Counter& cc = c;
    assert(cc.get() == 1);
    assert(cc.reads == 1);
}

// ---- 2) Returning references: const and encapsulation ------------------------
// Exposing non-const references breaks invariants. Use const ref or value return.
struct VecWrap
{
    std::vector<int> v{1,2,3};

    // Provide both const and non-const access.
    int& operator[](std::size_t i) { return v[i]; }
    const int& operator[](std::size_t i) const { return v[i]; }

    // Sometimes: offer a const-only view even on non-const object.
    const std::vector<int>& view() const { return v; }
};

static void reference_return_design()
{
    VecWrap w{};
    w[0] = 10;

    const VecWrap& cw = w;
    assert(cw[0] == 10);
    // cw[0] = 11; // ERROR
}

// ---- 3) Const-correct overloads for APIs: prefer "T const&" input -------------
// Taking by const& accepts both const and non-const lvalues without copying.
// But for small trivially-copyable types, pass-by-value is fine (and often better).
static int sum3(const std::vector<int>& v)
{
    int s = 0;
    for (int x : v) s += x;
    return s;
}

// ---- 4) iterator constness: "const iterator" vs "iterator to const" ----------
// For standard containers:
// - container::iterator: can modify elements
// - container::const_iterator: cannot modify elements (points-to-const element)
//
// "const iterator" (top-level const) is just an iterator object you can't reseat;
// it does not automatically become const_iterator.
static void iterator_constness()
{
    std::vector<int> v{1,2,3};

    std::vector<int>::iterator it = v.begin();
    *it = 10; // OK

    std::vector<int>::const_iterator cit = v.cbegin();
    // *cit = 11; // ERROR

    const std::vector<int>::iterator const_it = v.begin(); // top-level const iterator object
    *const_it = 12; // OK: still iterator, still mutable element
    // ++const_it;  // ERROR: cannot modify iterator object itself
    assert(v[0] == 12);
}

// ---- 5) "logical constness" and caching without breaking API -----------------
struct LazySum
{
    std::vector<int> data;

    // cache:
    mutable bool cached = false;
    mutable int cached_sum = 0;

    int sum() const
    {
        if (!cached)
        {
            cached_sum = 0;
            for (int x : data) cached_sum += x;
            cached = true;
        }
        return cached_sum;
    }

    void push(int x)
    {
        data.push_back(x);
        cached = false; // OK, non-const member function
    }
};

static void logical_constness_cache()
{
    LazySum s{};
    s.push(1);
    s.push(2);
    const LazySum& cs = s;
    assert(cs.sum() == 3);
    assert(cs.sum() == 3); // cached
}

// ---- 6) const in inheritance: overriding must match cv/ref qualifiers --------
// Signature must match exactly (except covariant returns).
struct Base
{
    virtual ~Base() = default;
    virtual int value() const = 0;
};

struct Derived final : Base
{
    int x = 7;
    int value() const override { return x; } // must be const to override
};

static void virtual_const_override()
{
    Derived d{};
    const Base& b = d;
    assert(b.value() == 7);
}

// ---- 7) Ref qualifiers (& / &&) + const: fine-grained API control ------------
// Common pattern: provide lvalue accessors returning references,
// but on rvalues return values (or moved values) to avoid dangling refs.
struct NameBox
{
    std::vector<int> payload;
    std::optional<int> cached_first;

    int& first() & { return payload.front(); }
    const int& first() const & { return payload.front(); }
    int first() && { return payload.front(); } // by value on temporary
};

static void ref_qualifier_pattern()
{
    NameBox b{{1,2,3}, std::nullopt};
    assert(b.first() == 1);

    int tmp = NameBox{{9,8,7}, std::nullopt}.first(); // calls && overload
    assert(tmp == 9);
}

// ---- 8) Perfect forwarding and const: forwarding preserves constness ---------
// Template param deduction rules matter: T&& is forwarding ref only when T is deduced.
template<class T>
constexpr bool is_const_v = std::is_const_v<std::remove_reference_t<T>>;

template<class T>
void observe(T&& x)
{
    // If caller passes const int&, T deduces to const int&, preserving const.
    std::cout << "observe: const? " << (is_const_v<T> ? "yes" : "no") << "\n";
    (void)x;
}

static void forwarding_constness()
{
    int x = 1;
    const int cx = 2;

    observe(x);   // non-const
    observe(cx);  // const
    observe(3);   // non-const prvalue
}

// ---- 9) Common API pitfall: const on by-value parameters does nothing --------
// This const is top-level and only affects local copy; callers see no difference.
static void pointless_const_param(const int x) // same as "int x"
{
    (void)x;
}

// ---- 10) "const" and "deep immutability": not automatic ---------------------
// const only prevents mutation through that interface. If you store pointers,
// const class can still mutate pointed-to objects unless you encode deep constness.
struct Holder
{
    int* p = nullptr;

    void set(int* q) { p = q; }

    // const member function can still mutate *p (deep mutation) because p points to non-const.
    void poke() const
    {
        if (p) *p += 1; // legal
    }
};

static void deep_constness_demo()
{
    int x = 10;
    Holder h{};
    h.set(&x);

    const Holder& ch = h;
    ch.poke();
    assert(x == 11);
}

} // namespace cpp

int main()
{
    cpp::const_member_basics();
    cpp::reference_return_design();
    cpp::iterator_constness();
    cpp::logical_constness_cache();
    cpp::virtual_const_override();
    cpp::ref_qualifier_pattern();
    cpp::forwarding_constness();
    cpp::deep_constness_demo();

    std::cout << "constness_and_classes.cpp: OK\n";
    return 0;
}

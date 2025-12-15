// ReSharper disable All
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace cpp {

// ---- 1) Top-level const vs low-level const ----------------------------------
// Top-level const: const applies to the object itself (e.g. "const int x").
// Low-level const: const applies to what a pointer/reference refers to (e.g. "const int* p").
//
// Rule of thumb:
// - "const T" (by value) affects the object itself -> top-level const.
// - "T const*" affects the pointee -> low-level const.
// - "T* const" affects the pointer itself -> top-level const on pointer object.
static void pointer_constness()
{
    int x = 10;
    int y = 20;

    const int* p_to_const = &x;     // low-level const: cannot modify *p_to_const
    // *p_to_const = 11;            // ERROR
    p_to_const = &y;               // OK: pointer can change

    int* const const_ptr = &x;     // top-level const: pointer cannot change
    *const_ptr = 11;               // OK: pointee mutable
    // const_ptr = &y;             // ERROR

    const int* const const_ptr_to_const = &x; // both: neither pointer nor pointee modifiable
    (void)const_ptr_to_const;
}

// ---- 2) const and references -------------------------------------------------
// References are not reseatable, so "T& const" is ill-formed / meaningless.
// The key constness is: "const T&" = reference to const.
static int& ref_identity(int& r) { return r; }
static const int& cref_identity(const int& r) { return r; }

static void reference_constness_and_temps()
{
    int x = 5;
    const int cx = 6;

    int& rx = ref_identity(x);
    rx = 7;
    (void)rx;

    const int& rcx = cref_identity(cx);
    (void)rcx;

    // Binding a const reference to a temporary extends the temporary's lifetime
    // to the lifetime of the reference (but only for that reference).
    const std::string& tmp_ref = std::string("hello");
    assert(tmp_ref.size() == 5);

    // Pitfall: returning a const& to a temporary is still dangling.
    // (lifetime extension does NOT cross function boundaries).
}

// ---- 3) const objects, initialization, and "runtime constness" ---------------
// const does not imply compile-time constant; it can be initialized at runtime.
static int read_runtime()
{
    return 42; // pretend: env var, file, RNG, etc.
}

static void runtime_constness()
{
    const int max_size = 100;          // const, but not necessarily compile-time used
    const int value = read_runtime();  // const, value known only at runtime
    (void)max_size;
    (void)value;

    // enum values are integral constant expressions
    enum { num_students = 30 };
    static_assert(num_students == 30);

    // This is a constant expression if all operands are constant expressions
    const int sum = 5 + 7 * 9;
    (void)sum;
}

// ---- 4) const and function overloading --------------------------------------
// You can overload on const for member functions and for reference qualifiers.
struct Buffer
{
    std::vector<int> data;

    // const member function: implicit this is "const Buffer*"
    std::size_t size() const { return data.size(); }

    // non-const member function: implicit this is "Buffer*"
    int& at(std::size_t i) { return data.at(i); }
    const int& at(std::size_t i) const { return data.at(i); }

    // Ref-qualifiers control overload based on value category of *this
    int& front() & { return data.front(); }
    const int& front() const & { return data.front(); }
    int&& front() && { return std::move(data.front()); } // rare but real
};

static void member_const_overloads()
{
    Buffer b{{1,2,3}};
    const Buffer cb{{4,5,6}};

    b.at(0) = 10;          // calls non-const at()
    // cb.at(0) = 10;       // ERROR: calls const at() returning const int&
    assert(cb.at(0) == 4);

    // ref-qualified overloads
    int& lref = b.front();
    (void)lref;
    int&& rref = Buffer{{7,8,9}}.front(); // calls && overload
    (void)rref;
}

// ---- 5) mutable: "logically const" vs "bitwise const" ------------------------
// const member function promises not to modify *observable state*,
// but C++ enforces "bitwise const" unless you mark fields mutable.
struct Cache
{
    int expensive_value = 0;
    mutable bool computed = false;   // can change even in const functions
    mutable int hits = 0;

    int get() const
    {
        ++hits; // allowed due to mutable
        if (!computed)
        {
            // This is often used for memoization / caches.
            expensive_value_computation(); // will mutate via const_cast below
        }
        return expensive_value;
    }

private:
    void expensive_value_computation() const
    {
        // "mutable" would be the better way for fields that are truly cache-only.
        // But expensive_value itself is non-mutable -> cannot assign directly here.
        // We'll demonstrate a controlled const_cast pattern.
        auto* self = const_cast<Cache*>(this);
        self->expensive_value = 123;
        self->computed = true; // mutable anyway
    }
};

static void mutable_demo()
{
    const Cache c{};
    int v1 = c.get();
    int v2 = c.get();
    assert(v1 == 123 && v2 == 123);
    assert(c.hits == 2);
}

// ---- 6) const_cast: what is legal vs Undefined Behavior ----------------------
// const_cast can remove const/volatile, but *modifying a truly const object is UB*.
// Legal scenario: original object is non-const, accessed via const pointer/reference.
// UB scenario: original object is actually const (or in read-only storage).
static void const_cast_rules()
{
    int x = 1;
    const int* px = &x;

    // Removing const is fine if the original object is non-const.
    int* hack = const_cast<int*>(px);
    *hack = 2;
    assert(x == 2);

    // UB example (DO NOT DO THIS):
    // const int cx = 3;
    // const int* pcx = &cx;
    // int* bad = const_cast<int*>(pcx);
    // *bad = 4; // UB: modifies a const object
}

// ---- 7) "const doesn't mean thread-safe" ------------------------------------
// const only restricts mutation through that interface; it says nothing about races.
// Even reading can race if another thread writes (data race => UB).
//
// Also: "const std::shared_ptr<T>" does not make *T const*. It only makes the smart
// pointer object itself immutable (cannot reseat), not the pointee.
static void const_is_not_deep_const()
{
    auto sp = std::make_shared<std::string>("hello");

    const std::shared_ptr<std::string> csp = sp; // top-level const on the shared_ptr
    // csp = nullptr; // ERROR: cannot reseat
    *csp += "!"; // OK: modifies the string, because std::string is non-const
    assert(*sp == "hello!");
}

// ---- 8) auto, decltype, and const-stripping ---------------------------------
// auto by value drops top-level const. References preserve.
static void auto_deduction_const()
{
    const int cx = 10;
    auto a = cx;               // int (top-level const dropped)
    static_assert(std::is_same_v<decltype(a), int>);

    const int& rcx = cx;
    auto b = rcx;              // int (still drops top-level const: b is by value)
    static_assert(std::is_same_v<decltype(b), int>);

    auto& c = rcx;             // const int& (reference keeps const)
    static_assert(std::is_same_v<decltype(c), const int&>);

    // decltype preserves exact type rules (incl. references depending on expression form).
    decltype(rcx) d = cx;      // const int&
    static_assert(std::is_same_v<decltype(d), const int&>);
}

// ---- 9) const on return-by-value is almost always useless --------------------
// "const T f()" prevents moving from the returned temporary in some contexts and
// provides little-to-no safety. Prefer "T f()".
static const std::string bad_api()
{
    return "abc";
}

static void const_return_by_value_pitfall()
{
    // You can still copy it, but "const" here mostly harms optimizations/usability.
    auto s = bad_api();
    assert(s == "abc");
}

// ---- 10) Containers and "const view" pitfalls -------------------------------
// const container => you cannot mutate elements through it (via its interface).
// But the element type might still own mutable graphs / pointers.
static void const_container_behavior()
{
    std::vector<int> v{1,2,3};
    const std::vector<int>& cv = v;

    // cv[0] = 9; // ERROR: operator[] returns const int& for const vector

    // But if vector holds pointers, const vector doesn't make pointed objects const:
    std::vector<int*> vp;
    int x = 7;
    vp.push_back(&x);

    const std::vector<int*>& cvp = vp;
    *cvp[0] = 8; // OK: mutates x (deep const not implied)
    assert(x == 8);
}

// ---- 11) string_view: constness doesn't own lifetime -------------------------
// std::string_view is a non-owning view; constness doesn't help with dangling.
static std::string_view dangling_view()
{
    // Returning view to a temporary string -> dangling after return.
    return std::string("temporary"); // BAD
}

static void string_view_pitfall()
{
    // Don't use the result; it dangles.
    auto v = dangling_view();
    (void)v;
}

} // namespace cpp

int main()
{
    cpp::pointer_constness();
    cpp::reference_constness_and_temps();
    cpp::runtime_constness();
    cpp::member_const_overloads();
    cpp::mutable_demo();
    cpp::const_cast_rules();
    cpp::const_is_not_deep_const();
    cpp::auto_deduction_const();
    cpp::const_return_by_value_pitfall();
    cpp::const_container_behavior();
    // cpp::string_view_pitfall(); // intentionally dangerous; left off by default

    std::cout << "runtime_constness.cpp: OK\n";
    return 0;
}

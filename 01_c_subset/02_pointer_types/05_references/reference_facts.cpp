// ReSharper disable All
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


/*
reference_facts.cpp
===================

A compact, example-driven reference on C++ references:
- lvalue references (C++98)
- rvalue references (C++11)
- reference collapsing and forwarding references
- lifetime extension + dangling pitfalls
- technical/logical differences with pointers
- notes on what changed from C++98 to C++17
*/

// ----------------------------------------------------------------------------
// 1) Basics: lvalue references (C++98)
// ----------------------------------------------------------------------------
static void lvalue_reference_basics()
{
    std::puts("\n== 1) lvalue references (C++98) ==");

    int x = 10;
    int& rx = x;                // binds to an lvalue
    rx += 5;                    // modifies x

    std::printf("x=%d, rx=%d (rx aliases x)\n", x, rx);

    // References are aliases: no reseating.
    int y = 77;
    rx = y;                     // assigns THROUGH rx, does NOT rebind (x becomes 77)
    std::printf("After `rx = y`: x=%d (rx still aliases x), y=%d\n", x, y);

    // You can't make a "null reference" in well-formed code.
    // (A reference must bind to some object.)
    // int& bad = *static_cast<int*>(nullptr); // UB if executed (don't do this)

    // Arrays of references are ill-formed:
    // int& arr_refs[2]; // error

    // Pointers to references are ill-formed:
    // int&* p; // error

    // References are not objects: you can't take sizeof(reference-type).
    // sizeof(int&) is ill-formed. But sizeof a reference variable yields sizeof the referred-to type:
    std::printf("sizeof(rx) = %zu (same as sizeof(int))\n", sizeof(rx));
}

// ----------------------------------------------------------------------------
// 2) const references and lifetime extension
// ----------------------------------------------------------------------------
static const std::string& dangerous_ref()
{
    // Returns a reference to a temporary created inside the function => dangling.
    // This compiles because the temporary is destroyed at the end of the full-expression
    // that creates it (inside this function). The returned reference dangles.
    return std::string("dangling");
}

static void const_reference_lifetime_extension()
{
    std::puts("\n== 2) const references: binding + lifetime extension ==");

    // A const lvalue reference can bind to a temporary (prvalue).
    const std::string& r = std::string("hello");
    std::printf("const std::string& bound to temporary: '%s'\n", r.c_str());

    // Lifetime extension rule: the temporary's lifetime is extended to match `r`
    // because r is a reference initialized directly from the temporary.

    // Pitfall: lifetime extension does NOT apply across function boundaries like this:
    const std::string& bad = dangerous_ref();

    // Accessing `bad` is UB. We'll avoid using it.
    std::puts("Avoided using `bad`: it's a dangling reference returned from function.");
}

// ----------------------------------------------------------------------------
// 3) Rvalue references (C++11): binding rules & why they exist
// ----------------------------------------------------------------------------
static std::string make_string()
{
    return "made";
}

static void rvalue_reference_basics()
{
    std::puts("\n== 3) rvalue references (C++11) ==");

    std::string s = "keep";
    std::string&& rr = make_string(); // binds to a temporary (rvalue)
    std::printf("rr='%s'\n", rr.c_str());

    // Important: rr is a named variable => it is an lvalue expression!
    // So to treat it as an rvalue again, you must std::move(rr).
    std::string t = rr;               // copies (rr is lvalue here)
    std::string u = std::move(rr);    // moves
    std::printf("t='%s', u='%s'\n", t.c_str(), u.c_str());

    // After move, rr is valid but in unspecified state.
    std::puts("After move, rr is valid but its value is unspecified.");
}

// ----------------------------------------------------------------------------
// 4) Overload resolution: & vs && (C++11)
// ----------------------------------------------------------------------------
static void sink(const std::string& s)
{
    std::printf("sink(const&): '%s'\n", s.c_str());
}
static void sink(std::string&& s)
{
    std::printf("sink(&&): '%s'\n", s.c_str());
}

static void overload_resolution_refs()
{
    std::puts("\n== 4) overload resolution: const& vs && ==");

    std::string a = "lvalue";
    sink(a);                  // calls const&
    sink(std::string("tmp")); // calls &&
    sink(std::move(a));       // calls &&
}

// ----------------------------------------------------------------------------
// 5) Reference collapsing & forwarding references (C++11)
// ----------------------------------------------------------------------------
template <class T>
static void show_T_ref_kind()
{
    // prints a tiny compile-time characterization using type traits
    std::printf("  is_lvalue_reference<T>=%d, is_rvalue_reference<T>=%d\n",
                std::is_lvalue_reference<T>::value,
                std::is_rvalue_reference<T>::value);
}

template <class T>
static void forwarding_probe(T&& param)
{
    // T&& here is a *forwarding reference* (aka universal reference) only when:
    //  - T is a deduced template parameter, and
    //  - the parameter is of the form T&& (not e.g. std::vector<T>&&)
    //
    // The magic is reference collapsing:
    //   - if T is U&, then T&& becomes U& (because U& && collapses to U&)
    //   - if T is U,  then T&& is U&&
    (void)param;
    std::puts("forwarding_probe:");
    show_T_ref_kind<T>();
    show_T_ref_kind<decltype(param)>();
}

static void reference_collapsing_and_forwarding()
{
    std::puts("\n== 5) reference collapsing + forwarding references (C++11) ==");

    int x = 1;
    forwarding_probe(x);         // T is int&, param is int&
    forwarding_probe(123);       // T is int,  param is int&&

    // In real code you almost always pair this with std::forward<T>(param)
    // to preserve value category.
}

// ----------------------------------------------------------------------------
// 6) auto&& (C++11) and decltype((x)) quirks
// ----------------------------------------------------------------------------
static void auto_and_decltype_reference_gotchas()
{
    std::puts("\n== 6) auto&& and decltype gotchas (C++11) ==");

    int x = 10;

    auto a1 = x;        // int (copy)
    auto& a2 = x;       // int&
    auto&& a3 = x;      // int&   (because x is lvalue => forwarding behavior)
    auto&& a4 = 123;    // int&&  (temporary)

    (void)a1; (void)a2; (void)a3; (void)a4;

    // decltype:
    //  decltype(x)   => int
    //  decltype((x)) => int&   (because (x) is an lvalue expression)
    static_assert(std::is_same<decltype(x), int>::value, "decltype(x) is int");
    static_assert(std::is_same<decltype((x)), int&>::value, "decltype((x)) is int&");

    std::puts("decltype((x)) yields a reference type because (x) is an lvalue expression.");
}

// ----------------------------------------------------------------------------
// 7) References vs pointers: technical & logical differences
// ----------------------------------------------------------------------------
static void pointers_vs_references()
{
    std::puts("\n== 7) references vs pointers (differences) ==");

    int x = 5;
    int y = 9;

    int* px = &x; // pointer: can be null, can be reseated
    *px += 1;

    int& rx = x;  // reference: cannot be null (in well-formed code), cannot be reseated
    rx += 1;

    std::printf("x=%d, y=%d\n", x, y);

    // reseat pointer:
    px = &y;
    *px += 10;
    std::printf("After reseating px to y: y=%d\n", y);

    // "reseat" reference? no. `rx = y;` assigns through rx.
    rx = y;
    std::printf("After `rx = y`: x=%d (assigned), rx still aliases x\n", x);

    // pointer arithmetic exists; reference arithmetic doesn't (references are not objects you can increment).
    int arr[3] = {1,2,3};
    int* p = arr;
    ++p; // OK
    std::printf("pointer arithmetic: *(arr+1)=%d\n", *p);

    // But a reference can be "reseated" only by rebinding a new reference variable.
    int& r2 = y;
    std::printf("new reference r2 aliases y=%d\n", r2);

    std::puts("Rule of thumb:");
    std::puts("  - Use references for required, non-null aliases in APIs.");
    std::puts("  - Use pointers for optionality (nullable), reseating, or pointer arithmetic.");
}

// ----------------------------------------------------------------------------
// 8) Obscure reference pitfalls
// ----------------------------------------------------------------------------
static void obscure_reference_pitfalls()
{
    std::puts("\n== 8) obscure reference pitfalls ==");

    // (1) Reference members make a type non-assignable by default semantics.
    struct HasRefMember
    {
        int& r;
        // compiler-generated copy assignment is deleted because you can't reseat `r`
    };
    int x = 1;
    int y = 2;
    HasRefMember a{x};
    HasRefMember b{y};
    (void)a; (void)b;
    std::puts("Reference members often delete copy assignment (because references can't be reseated).");

    // (2) You can't bind non-const lvalue reference to a temporary:
    // int& bad = 123; // error

    // (3) You can bind const& to a temporary, but beware of returning it (dangling).
    const int& ok = 123;
    std::printf("const int& to temporary ok=%d\n", ok);

    // (4) Reference-to-bitfield is ill-formed.
    struct Bits { unsigned a : 3; } bits{5};
    // unsigned& rb = bits.a; // error: can't bind reference to bit-field
    (void)bits;

    // (5) Proxy references: std::vector<bool>::reference is not bool&.
    std::vector<bool> vb(1);
    vb[0] = true;
    auto proxy = vb[0];
    // bool& br = vb[0]; // error: cannot bind bool& to proxy
    std::printf("vector<bool> element as proxy converted to bool=%d\n", static_cast<bool>(proxy));

    std::puts("Obscure: vector<bool> uses proxy references; many templates expecting T& break.");
}

// ----------------------------------------------------------------------------
// 9) std::reference_wrapper: "references you can store" (C++11)
// ----------------------------------------------------------------------------
static void reference_wrapper_demo()
{
    std::puts("\n== 9) std::reference_wrapper (C++11) ==");

    int x = 10;
    std::reference_wrapper<int> rw = x; // copyable, assignable
    rw.get() += 1;

    std::printf("x=%d\n", x);

    int y = 50;
    rw = y; // reseats wrapper to refer to y (unlike a true reference)
    rw.get() += 2;
    std::printf("x=%d, y=%d (rw now refers to y)\n", x, y);

    std::puts("reference_wrapper gives 'reference semantics' with reseating and copyability.");
}

// ----------------------------------------------------------------------------
// 10) Timeline: C++98..C++17 (reference-relevant highlights)
// ----------------------------------------------------------------------------
static void standards_timeline()
{
    std::puts("\n== 10) C++98..C++17 timeline (reference highlights) ==");

    std::puts("C++98:");
    std::puts("  - lvalue references (T&), const references (const T&)");
    std::puts("  - reference binding rules; pass-by-reference idiom");

    std::puts("C++11:");
    std::puts("  - rvalue references (T&&), move semantics, std::move/std::forward");
    std::puts("  - reference collapsing rules (U& && -> U&, etc.)");
    std::puts("  - auto/decltype enable reference-heavy generic code");
    std::puts("  - std::reference_wrapper, perfect forwarding patterns");

    std::puts("C++14:");
    std::puts("  - generic lambdas (auto parameters) make forwarding patterns ubiquitous");
    std::puts("  - return type deduction (auto) + decltype(auto) (C++14) helps preserve references");

    std::puts("C++17:");
    std::puts("  - guaranteed copy elision (reduces the need to 'std::move' returns)");
    std::puts("  - structured bindings: can bind by value or by reference (auto& / auto&&)");
    std::puts("  - constexpr lambdas (C++17) improve compile-time reference-heavy code");
}

// ----------------------------------------------------------------------------
// 11) Structured bindings reference flavor (C++17)
// ----------------------------------------------------------------------------
static void structured_bindings_demo()
{
    std::puts("\n== 11) structured bindings (C++17) ==");

#if __cplusplus >= 201703L
    std::pair<int, int> p{1, 2};

    auto  [a, b] = p;    // copies
    auto& [ra, rb] = p;  // references to p.first/p.second

    ra += 10;
    std::printf("a=%d, b=%d (copies)\n", a, b);
    std::printf("p.first=%d, p.second=%d (modified via ra)\n", p.first, p.second);
#else
    std::puts("Structured bindings require C++17.");
#endif
}

int main()
{
    lvalue_reference_basics();
    const_reference_lifetime_extension();
    rvalue_reference_basics();
    overload_resolution_refs();
    reference_collapsing_and_forwarding();
    auto_and_decltype_reference_gotchas();
    pointers_vs_references();
    obscure_reference_pitfalls();
    reference_wrapper_demo();
    standards_timeline();
    structured_bindings_demo();

    return 0;
}

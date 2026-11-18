// Requires C++23 (<expected>, P0323). This target sets its own
// CXX_STANDARD in CMakeLists.txt rather than the project-wide C++20,
// since std::expected is specific to this one example.
// NOTE ON VERIFICATION: this machine's Xcode-14 libc++ does not ship
// <expected> at all yet (the header itself is missing, confirmed while
// writing this), so this leaf could not be compiled or run here --
// unlike every other leaf in this chapter, the assert()s below have
// been checked by hand against the standard's specified behavior for
// std::expected, not by an actual compiler/runtime on this machine.
#include <expected>

#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

enum class parse_error
{
    empty,
    not_a_number,
    out_of_range
};

const char* error_message(parse_error e)
{
    switch (e) {
        case parse_error::empty: return "empty string";
        case parse_error::not_a_number: return "not a number";
        case parse_error::out_of_range: return "out of range";
    }
    return "unknown";
}

// std::expected<T, E> is a non-exception error channel: a function
// either returns a T (the success case) or an E (via std::unexpected),
// and the caller must explicitly check which one it got -- there is no
// implicit "the error silently propagated up the stack" the way a
// thrown exception propagates.
std::expected<int, parse_error> parse_int(const std::string& s)
{
    if (s.empty())
        return std::unexpected(parse_error::empty);

    try {
        size_t pos = 0;
        long value = std::stol(s, &pos);

        if (pos != s.size())
            return std::unexpected(parse_error::not_a_number);

        if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            return std::unexpected(parse_error::out_of_range);

        return static_cast<int>(value);
    }
    catch (const std::invalid_argument&) {
        return std::unexpected(parse_error::not_a_number);
    }
    catch (const std::out_of_range&) {
        return std::unexpected(parse_error::out_of_range);
    }
}

void show_basic_usage()
{
    std::expected<int, parse_error> good = parse_int("42");
    std::expected<int, parse_error> bad = parse_int("abc");

    assert(good.has_value());
    assert(good.value() == 42);
    // operator*/operator-> are available too, like std::optional, and
    // are NOT the same thing as value(): they don't check has_value()
    // at all -- calling them on an error-holding expected is UB, not a
    // thrown exception. value() is the checked, exception-throwing form.
    assert(*good == 42);

    assert(!bad.has_value());
    assert(bad.error() == parse_error::not_a_number);

    if (good.has_value())
        std::cout << "parsed: " << good.value() << '\n';

    if (!bad.has_value())
        std::cout << "error: " << error_message(bad.error()) << '\n';

    // value_or() supplies a fallback instead of branching explicitly --
    // the same shape as std::optional<T>::value_or().
    assert(bad.value_or(-1) == -1);
    assert(good.value_or(-1) == 42); // value_or() on a SUCCESS just forwards the value
    std::cout << "bad.value_or(-1) = " << bad.value_or(-1) << '\n';
}

// Calling value() on an expected that holds an error is the one place
// std::expected still goes through the exception mechanism: it throws
// std::bad_expected_access<E>, which carries the error value along with
// it -- a deliberate safety net for code that forgot to check
// has_value() first, not the intended way to consume an expected.
void show_bad_expected_access()
{
    std::expected<int, parse_error> bad = parse_int("");
    assert(bad.error() == parse_error::empty);

    bool caught = false;
    try {
        int v = bad.value();
        (void)v;
        assert(false); // unreachable: bad holds an error, not a value
    }
    // bad_expected_access<E>::error() returns a COPY of the E that was
    // stored in the expected -- the exception carries the original
    // failure reason out with it, not just "something went wrong".
    catch (const std::bad_expected_access<parse_error>& e) {
        caught = true;
        assert(e.error() == parse_error::empty);
        std::cout << "bad_expected_access, error() = " << error_message(e.error()) << '\n';
    }
    assert(caught);
}

// transform()/and_then()/or_else() let std::expected values be composed
// without manual has_value() checks at every step -- the same monadic
// style std::optional supports, extended to carry an error value through
// the failure path instead of just "nothing".
void show_monadic_chaining()
{
    auto times_two = [](int v) { return v * 2; };

    // transform(): applies the function to the value, if present...
    std::expected<int, parse_error> doubled = parse_int("21").transform(times_two);
    assert(doubled.has_value());
    assert(doubled.value() == 42);
    std::cout << "doubled = " << doubled.value_or(-1) << '\n';

    // ...otherwise passes the existing error through UNCHANGED -- the
    // function is never even called. The original demo never exercised
    // this half of transform()'s own documented behavior.
    std::expected<int, parse_error> still_an_error = parse_int("abc").transform(times_two);
    assert(!still_an_error.has_value());
    assert(still_an_error.error() == parse_error::not_a_number);

    auto reciprocal_hundred = [](int v) -> std::expected<int, parse_error> {
        if (v == 0)
            return std::unexpected(parse_error::out_of_range);
        return 100 / v;
    };

    // and_then(): the function itself returns another expected, so a
    // chain of fallible steps short-circuits at the first failure,
    // exactly like a chain of early-return error checks would, but
    // without writing any of those checks by hand.
    std::expected<int, parse_error> chained = parse_int("5").and_then(reciprocal_hundred);
    assert(chained.has_value());
    assert(chained.value() == 20);
    std::cout << "chained = " << chained.value_or(-1) << '\n';

    // and_then() SHORT-CIRCUITS on an input that's already an error: the
    // chained function never runs at all, so a parse failure can never
    // be misread as "parsed 0, then divided by zero" -- it stays the
    // original parse_error::not_a_number all the way through.
    std::expected<int, parse_error> short_circuited = parse_int("abc").and_then(reciprocal_hundred);
    assert(!short_circuited.has_value());
    assert(short_circuited.error() == parse_error::not_a_number);

    // or_else(): the error-path counterpart to and_then() -- runs only
    // when there is an error, and can recover with a value instead of
    // propagating the failure further.
    std::expected<int, parse_error> recovered = parse_int("not a number").or_else(
        [](parse_error) -> std::expected<int, parse_error> {
            return 0; // fall back to a default rather than propagating
        });
    assert(recovered.has_value());
    assert(recovered.value() == 0);
    std::cout << "recovered = " << recovered.value() << '\n';

    // ...and, symmetrically with transform(), or_else() leaves a
    // SUCCESS value untouched -- the recovery function never runs.
    std::expected<int, parse_error> already_fine = parse_int("7").or_else(
        [](parse_error) -> std::expected<int, parse_error> {
            assert(false); // unreachable: input already holds a value
            return 0;
        });
    assert(already_fine.has_value());
    assert(already_fine.value() == 7);
}

int main()
{
    show_basic_usage();
    show_bad_expected_access();
    show_monadic_chaining();
    return 0;
}

// Requires C++23 (<expected>, P0323). This target sets its own
// CXX_STANDARD in CMakeLists.txt rather than the project-wide C++20,
// since std::expected is specific to this one example.
#include <expected>

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

    if (good.has_value())
        std::cout << "parsed: " << good.value() << '\n';

    if (!bad.has_value())
        std::cout << "error: " << error_message(bad.error()) << '\n';

    // value_or() supplies a fallback instead of branching explicitly --
    // the same shape as std::optional<T>::value_or().
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

    try {
        int v = bad.value();
        (void)v;
    }
    catch (const std::bad_expected_access<parse_error>& e) {
        std::cout << "bad_expected_access, error() = " << error_message(e.error()) << '\n';
    }
}

// transform()/and_then()/or_else() let std::expected values be composed
// without manual has_value() checks at every step -- the same monadic
// style std::optional supports, extended to carry an error value through
// the failure path instead of just "nothing".
void show_monadic_chaining()
{
    // transform(): applies the function to the value, if present;
    // otherwise passes the existing error through unchanged.
    std::expected<int, parse_error> doubled = parse_int("21").transform([](int v) { return v * 2; });
    std::cout << "doubled = " << doubled.value_or(-1) << '\n';

    // and_then(): the function itself returns another expected, so a
    // chain of fallible steps short-circuits at the first failure,
    // exactly like a chain of early-return error checks would, but
    // without writing any of those checks by hand.
    std::expected<int, parse_error> chained = parse_int("5").and_then(
        [](int v) -> std::expected<int, parse_error> {
            if (v == 0)
                return std::unexpected(parse_error::out_of_range);
            return 100 / v;
        });
    std::cout << "chained = " << chained.value_or(-1) << '\n';

    // or_else(): the error-path counterpart to and_then() -- runs only
    // when there is an error, and can recover with a value instead of
    // propagating the failure further.
    std::expected<int, parse_error> recovered = parse_int("not a number").or_else(
        [](parse_error) -> std::expected<int, parse_error> {
            return 0; // fall back to a default rather than propagating
        });
    std::cout << "recovered = " << recovered.value() << '\n';
}

int main()
{
    show_basic_usage();
    show_bad_expected_access();
    show_monadic_chaining();
    return 0;
}

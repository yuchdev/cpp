// ReSharper disable All
#include <cassert>
#include <iostream>
#include <type_traits>
#include <utility>

namespace cpp {

// ---- 1) const controls modification, not when evaluation happens -------------
static int read_runtime()
{
    return 40;
}

static void const_is_not_a_compile_time_promise()
{
    const int from_constant_expression = 6;
    const int from_runtime = read_runtime();

    // An integral const object initialized by a constant expression can itself
    // be used as a constant expression in the appropriate contexts.
    static_assert(from_constant_expression * 7 == 42);

    // This is const too, but its value isn't available during constant evaluation.
    // template<int> struct size_tag {};
    // size_tag<from_runtime> invalid{};
    assert(from_runtime == 40);
}

// ---- 2) constexpr variables and functions -----------------------------------
constexpr int increment(int value)
{
    return value + 1;
}

static void constexpr_can_be_evaluated_either_way()
{
    constexpr int fixed = increment(41);
    static_assert(fixed == 42);

    const int runtime_input = read_runtime();
    const int runtime_result = increment(runtime_input); // ordinary runtime call
    assert(runtime_result == 41);

    // A constexpr variable is const, but constexpr doesn't mean deeply immutable.
    static int mutable_object = 1;
    constexpr int* pointer_to_mutable = &mutable_object;
    static_assert(std::is_const_v<decltype(pointer_to_mutable)>);
    *pointer_to_mutable = 2; // the pointer is const; the pointed-to int is not
    assert(mutable_object == 2);
}

// ---- 3) consteval declares an immediate function -----------------------------
constexpr int double_value(int value)
{
    return value * 2;
}

consteval int make_compile_time_value(int value)
{
    return double_value(value);
}

static void consteval_requires_constant_evaluation()
{
    constexpr int value = make_compile_time_value(21);
    static_assert(value == 42);

    const int runtime_input = read_runtime();
    (void)runtime_input;
    // make_compile_time_value(runtime_input); // ERROR: immediate call needs a constant expression
}

// ---- 4) Detecting the evaluation mode ----------------------------------------
constexpr int evaluation_mode()
{
    return std::is_constant_evaluated() ? 1 : 0;
}

// Static/thread-local initializers may get a trial constant evaluation.
const int static_initialization_mode = evaluation_mode();

static void evaluation_mode_is_contextual()
{
    constexpr int compile_time = evaluation_mode();
    static_assert(compile_time == 1);

    int runtime = evaluation_mode();
    assert(runtime == 0);
    assert(static_initialization_mode == 1);
}

// ---- 5) Common pitfalls -------------------------------------------------------
static void common_pitfalls()
{
    // `constexpr` on a function makes constant evaluation possible, not mandatory.
    const int result = increment(read_runtime()); // legal runtime evaluation
    assert(result == 41);

    // `consteval` is an API constraint, not a performance hint: callers cannot
    // fall back to a runtime call. Prefer constexpr when both modes are valid.

    // `std::is_constant_evaluated()` reports the evaluation context. Static and
    // thread-local initializers may be trial-evaluated; don't use it to predict
    // whether initialization will ultimately be constant.
}

} // namespace cpp

int main()
{
    cpp::const_is_not_a_compile_time_promise();
    cpp::constexpr_can_be_evaluated_either_way();
    cpp::consteval_requires_constant_evaluation();
    cpp::evaluation_mode_is_contextual();
    cpp::common_pitfalls();

    std::cout << "const_expr_eval.cpp: OK\n";
    return 0;
}

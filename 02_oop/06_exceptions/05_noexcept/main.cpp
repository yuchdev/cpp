#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#if !defined(_WIN32)
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace std;

/*

C++11 features (the noexcept specifier and operator are both C++11, not C++14)

* Preventing exception propagation: the noexcept specifier
* Testing for the possibility of a throw in an expression: the noexcept operator

Examples:
1. Finally (RAII substitute for a missing `finally` keyword)
2. noexcept specifier: a genuinely non-throwing function, and one that lies
3. Conditional noexcept + the noexcept operator, exercising both branches
4. What actually happens when a noexcept function throws anyway (forked,
   so it doesn't take this test binary down with it)
5. set_terminate: installing a handler that controls how that abandonment looks

(Vector's own use of noexcept move constructors to pick a stronger
reallocation strategy is demonstrated in 01_classes/02_constructors/main.cpp
-- show_noexcept_move_changes_vector_growth_strategy() -- rather than
repeated here.)
*/

namespace cpp
{

// 1. `finally` implementation
// The 'finally action' is provided as an argument to the constructor
template<typename F>
struct final_action
{
    final_action(F f) : clean { f }
    {
    }

    ~final_action()
    {
        clean();
    }

    F clean;
};

// we define a function that conveniently deduces the type of action
template<class F>
final_action<F> finally(F f)
{
    return final_action<F>(f);
}

} // namespace cpp

void show_finally()
{
    int* i = new int[100];
    bool cleaned_up = false;

    {
        // the cleanup runs on ANY exit from this scope -- normal or
        // exceptional -- because it is riding on ordinary destructor
        // semantics, not a dedicated language feature
        auto guard = cpp::finally([&] {
            delete[] i;
            cleaned_up = true;
        });
    } // guard destroyed here

    assert(cleaned_up);
}

// 2/3. noexcept specifier, conditional noexcept, the noexcept operator
namespace cpp
{

// genuinely non-throwing: new[]/delete[] of ints and memset do not throw
void check_noexcept1() noexcept
{
    int* i = new int[1000];
    std::memset(i, 0, sizeof(int) * 1000);
    delete[] i;
}

// noexcept is a promise the compiler does NOT verify: this compiles, and
// throws anyway. See show_noexcept_violation_terminates_by_default() below
// for what that actually does at runtime -- it is NOT the same as an
// ordinary throw that a caller's catch block can see.
void check_noexcept2() noexcept
{
    throw std::runtime_error("boom");
}

// std::is_pod was deprecated in C++20. The direct modern replacement for
// "is copying this with memcpy-like semantics safe and non-throwing" is
// is_trivially_copyable, not the old std::is_pod category.
template<typename T>
void copy_function(T& x) noexcept(std::is_trivially_copyable_v<T>)
{
    T tmp(x); // non-throwing only when T is trivially copyable
}

// zero memory is noexcept
template <typename T>
void func(T t) noexcept
{
    std::memset(&t, 0, sizeof(T));
}

// noexcept operator - returns true if operand is nothrow. Conditional
// noexcept specifications and the noexcept() operator are how generic
// standard-library operations (container swap, move construction, etc.)
// propagate "doesn't throw" guarantees through template code without the
// author hand-writing it for every instantiation:
//
//   template<class T, size_t N>
//   void swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(swap(*a, *b)));
template <typename T>
void call_func(std::vector<T>& v) noexcept(noexcept(func(v[0])))
{
    for (auto x : v)
        func(x);
}

} // namespace cpp

void show_check_noexcept1_is_actually_safe()
{
    cpp::check_noexcept1(); // keeps its promise; nothing to catch
}

void show_conditional_noexcept_both_branches()
{
    struct trivial
    {
        int i {};
    };
    struct non_trivial
    {
        std::string s; // non-trivial copy constructor
    };

    trivial t;
    non_trivial nt;

    // The noexcept OPERATOR never evaluates its operand -- like
    // decltype, it only inspects the expression's type/overload --
    // so asking it about a non-constexpr local is perfectly fine, and
    // the result is itself a compile-time constant usable in
    // static_assert. The original demo only ever instantiated
    // copy_function<T> with a POD/trivial T, so the conditional
    // noexcept's "false" branch had never actually been exercised.
    static_assert(noexcept(cpp::copy_function(t)));
    static_assert(!noexcept(cpp::copy_function(nt)));

    cpp::copy_function(t);
    cpp::copy_function(nt); // still perfectly legal to call; just not noexcept

    std::vector<int> v { 1, 2, 3 };
    static_assert(noexcept(cpp::call_func(v)));
    cpp::call_func(v);
}

#if !defined(_WIN32)

// Runs f() in a fresh child process and returns the child's wait status.
// We need a real child here, not just a try/catch in this process: a
// noexcept violation calls std::terminate(), which (by default) aborts
// the process outright -- there is no way to observe that from inside
// the very process it kills, short of ending this test binary too.
int run_in_child(void (*f)())
{
    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        f();
        _exit(42); // unreachable if f() really doesn't return normally
    }

    int status = 0;
    waitpid(pid, &status, 0);
    return status;
}

// Proof, not just a comment: a noexcept function that throws anyway does
// NOT produce a catchable exception at the call site. std::terminate()
// runs instead, and the default terminate_handler calls std::abort(),
// which the OS reports to the parent as "killed by SIGABRT" -- a signal,
// not a clean exit, and nothing resembling a reachable catch block.
void show_noexcept_violation_terminates_by_default()
{
    int status = run_in_child(&cpp::check_noexcept2);

    assert(WIFSIGNALED(status));
    assert(WTERMSIG(status) == SIGABRT);
}

// Same violation, but now with our own terminate_handler installed (see
// main()): std::terminate() calls OUR handler instead of the default
// abort() path, so the child exits cleanly with the status my_handler
// chose -- proving set_terminate() genuinely changes how abandonment
// looks, not just that it compiles.
void show_custom_terminate_handler_runs_instead_of_abort()
{
    int status = run_in_child(&cpp::check_noexcept2);

    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 1);
}

#endif // !_WIN32

// 5. set_terminate

using terminate_handler = void (*)();

[[noreturn]]
void my_handler()
{
    // handle termination my way -- exit() (unlike abort()) still runs
    // global/static destructors and atexit-registered functions on the
    // way out, at the cost of running on program state that may be
    // arbitrarily broken by whatever got us here.
    exit(1);
}

int main()
{
#if !defined(_WIN32)
    // exercise the DEFAULT terminate_handler first, before installing
    // our own -- fork() copies whichever handler is active at the time
    show_noexcept_violation_terminates_by_default();
#endif

    // the standard guarantees some handler is always installed (the
    // implementation-default one, typically calling abort()), so the
    // previous handler returned here is never null
    terminate_handler old_handler = ::set_terminate(&my_handler);
    assert(old_handler != nullptr);

#if !defined(_WIN32)
    show_custom_terminate_handler_runs_instead_of_abort();
#endif

    show_finally();
    show_check_noexcept1_is_actually_safe();
    show_conditional_noexcept_both_branches();

    ::set_terminate(old_handler);
    return 0;
}

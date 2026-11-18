#include <cstdlib>
#include <iostream>

/*
 * Abandoning a process abruptly, but with a chance to flush something
 * specific first: quick_exit() / at_quick_exit() (both C++11).
 *
 * NOTE ON VERIFICATION: this target is standards-conformant C++11 and
 * builds on mainstream GCC/Clang-on-Linux and MSVC, but it does not
 * LINK on this repo's macOS/Xcode-14 toolchain -- Apple's libc++
 * declares quick_exit()/at_quick_exit() as `using_if_exists`, and the
 * platform's own C library doesn't actually provide the symbols (see
 * the section README for detail). It could not be compiled or run
 * while writing this, so unlike every other leaf in this chapter there
 * is no assert() here backing the ordering claim below at runtime --
 * it is the documented standard behavior, checked against cppreference,
 * not something this specific build has verified.
 */

// at_quick_exit() callbacks run in REVERSE registration order -- the
// same LIFO rule as atexit(). Registering only one callback (as the
// original version of this example did) can't actually exercise that
// rule; two callbacks, printing something that names their own
// registration order, can.
void cleanup_second_registered()
{
    std::cout << "cleanup: second registered, runs FIRST\n";
}

void cleanup_first_registered()
{
    std::cout << "cleanup: first registered, runs SECOND\n";
}

// quick_exit() sits between exit() and abort(): like exit(), it hands
// the OS a clean status code; like abort(), it does NOT run
// automatic-storage destructors or atexit()-registered functions --
// those are skipped entirely, not merely deferred. Only callbacks
// registered specifically via at_quick_exit() run at all.
[[noreturn]]
void abandon_process()
{
    std::quick_exit(0);
}

int main()
{
    if (std::at_quick_exit(&cleanup_first_registered) != 0) {
        std::cout << "Too many at_quick_exit() functions\n";
        return 1;
    }
    if (std::at_quick_exit(&cleanup_second_registered) != 0) {
        std::cout << "Too many at_quick_exit() functions\n";
        return 1;
    }

    // Expected stdout, in this order, on a toolchain whose C library
    // actually provides quick_exit()/at_quick_exit():
    //   cleanup: second registered, runs FIRST
    //   cleanup: first registered, runs SECOND
    abandon_process();

    return 0; // unreachable: quick_exit() never returns
}

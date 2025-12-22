/*
================================================================================
04_noexcept_is_part_of_type.cpp

Advanced topic: noexcept is part of the function type (C++17+).

Consequences:
  - void(*)() and void(*)() noexcept are distinct types.
  - Assignments across them are ill-formed (can't drop/add noexcept).
  - Overloads can differ only by noexcept and overload resolution can prefer noexcept.
  - Generic wrappers should propagate noexcept with `noexcept(expr)`.

This file demonstrates:
  1) Pointer type differences
  2) Overload resolution preferring noexcept
  3) noexcept propagation in a `call()` helper
  4) `noexcept` as a compile-time query tool
================================================================================
*/

#include <iostream>
#include <type_traits>
#include <utility>
#include <functional>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

void may_throw() { println("may_throw()"); }
void no_throw() noexcept { println("no_throw() noexcept"); }

void pick() { println("pick()"); }
void pick() noexcept { println("pick() noexcept"); }

template <class F, class... Args>
constexpr bool is_noexcept_invoke_v =
    noexcept(std::invoke(std::declval<F>(), std::declval<Args>()...));

template <class F, class... Args>
decltype(auto) call(F&& f, Args&&... args) noexcept(is_noexcept_invoke_v<F, Args...>) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

static void demo_pointer_types() {
    println("== noexcept in pointer types ==");

    using P1 = void(*)();
    using P2 = void(*)() noexcept;

    P1 p1 = &may_throw;
    P2 p2 = &no_throw;

    p1();
    p2();

    println("P1 == P2? ", std::is_same_v<P1, P2> ? "yes" : "no");
}

static void demo_overload_resolution() {
    println("\n== overload resolution prefers noexcept ==");
    pick();
}

static void demo_noexcept_queries() {
    println("\n== noexcept queries in generic code ==");

    println("is_noexcept_invoke(may_throw) = ",
            is_noexcept_invoke_v<decltype(&may_throw)> ? "true" : "false");
    println("is_noexcept_invoke(no_throw)  = ",
            is_noexcept_invoke_v<decltype(&no_throw)> ? "true" : "false");

    println("noexcept(call(&may_throw)) = ", noexcept(call(&may_throw)) ? "true" : "false");
    println("noexcept(call(&no_throw))  = ", noexcept(call(&no_throw)) ? "true" : "false");

    call(&may_throw);
    call(&no_throw);
}

int main() {
    demo_pointer_types();
    demo_overload_resolution();
    demo_noexcept_queries();
    println("\nTakeaway: noexcept participates in type identity and generic design.");
    return 0;
}

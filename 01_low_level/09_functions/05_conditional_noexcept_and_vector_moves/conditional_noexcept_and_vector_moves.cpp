/*
================================================================================
05_conditional_noexcept_and_vector_moves.cpp

Advanced topic: conditional noexcept drives standard library algorithm choices.

Classic:
  std::vector reallocation must preserve strong exception guarantee.
  Many implementations will:
    - move elements if move ctor is noexcept
    - otherwise copy elements (if copy is available)

This file demonstrates:
  1) Observing vector's copy vs move on reallocation
  2) A conditional-noexcept "Box<T>" pattern
================================================================================
*/

#include <iostream>
#include <vector>
#include <type_traits>
#include <utility>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

struct MoveMayThrow {
    MoveMayThrow() = default;
    MoveMayThrow(const MoveMayThrow&) { println("MoveMayThrow: copy"); }
    MoveMayThrow(MoveMayThrow&&) noexcept(false) { println("MoveMayThrow: move (may throw)"); }
};

struct MoveNoThrow {
    MoveNoThrow() = default;
    MoveNoThrow(const MoveNoThrow&) { println("MoveNoThrow: copy"); }
    MoveNoThrow(MoveNoThrow&&) noexcept(true) { println("MoveNoThrow: move (noexcept)"); }
};

template <class T>
struct Box {
    T value;

    Box() = default;
    Box(const T& v) : value(v) {}

    Box(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value(std::move(v)) {}

    Box(const Box&) = default;
    Box(Box&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
};

static void demo_vector_realloc_may_throw() {
    println("== vector<MoveMayThrow> reallocation ==");

    std::vector<MoveMayThrow> v;
    v.reserve(1);

    println("push #1");
    v.push_back(MoveMayThrow{});

    println("push #2 (growth -> relocation)");
    v.push_back(MoveMayThrow{});
}

static void demo_vector_realloc_no_throw() {
    println("\n== vector<MoveNoThrow> reallocation ==");

    std::vector<MoveNoThrow> v;
    v.reserve(1);

    println("push #1");
    v.push_back(MoveNoThrow{});

    println("push #2 (growth -> relocation)");
    v.push_back(MoveNoThrow{});
}

static void demo_conditional_noexcept_types() {
    println("\n== conditional noexcept pattern ==");

    println("Box<MoveMayThrow> nothrow move? ",
            std::is_nothrow_move_constructible_v<Box<MoveMayThrow>> ? "yes" : "no");
    println("Box<MoveNoThrow>  nothrow move? ",
            std::is_nothrow_move_constructible_v<Box<MoveNoThrow>> ? "yes" : "no");
}

int main() {
    demo_vector_realloc_may_throw();
    demo_vector_realloc_no_throw();
    demo_conditional_noexcept_types();

    println("\nTakeaway: noexcept affects STL behavior and performance.");
    return 0;
}

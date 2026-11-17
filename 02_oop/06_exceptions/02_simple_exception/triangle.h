#pragma once

#include <exception>

// A small exception hierarchy nested inside the class that throws them --
// a common, deliberate design: the exceptions are part of triangle's
// public interface and conceptually belong to it, not to free scope.
class triangle
{
public:
    class triangle_exception : public std::exception
    {
    public:
        explicit triangle_exception(const char* msg) : _msg(msg) {}
        const char* what() const noexcept override { return _msg; }

    private:
        const char* _msg;
    };

    // Thrown when an individual side length is not even a valid length.
    class wrong_param : public triangle_exception
    {
    public:
        wrong_param(const char* msg, int param)
            : triangle_exception(msg), _param(param)
        {
        }

        int get_wrong_param() const { return _param; }

    private:
        int _param;
    };

    // Thrown when all three lengths are individually valid but cannot
    // form a triangle (the triangle inequality fails).
    class non_triangle_exception : public triangle_exception
    {
    public:
        non_triangle_exception(const char* msg, int a, int b, int c);

        // The (non-positive) amount by which the triangle inequality
        // failed, as a concrete, inspectable number rather than just a
        // message: 0 means degenerate (a straight line), negative means
        // genuinely impossible.
        int wrong_size() const { return _deficit; }

    private:
        int _deficit;
    };

    triangle(int a, int b, int c);

    // Throws a type that is deliberately NOT named in the catch clauses
    // that demonstrate this class, caught only through the public
    // triangle_exception base -- exactly the scenario where catching by
    // base class (not by exact type) matters.
    void throw_test() const;

    void test_nothrow() const noexcept;
    void test_throw_some1() const;
    void test_throw_some2() const;

private:
    class unspecified_defect : public triangle_exception
    {
    public:
        explicit unspecified_defect(const char* msg) : triangle_exception(msg) {}
    };

    int _a, _b, _c;
};

#include <iostream>

// Demonstrates the "main is the real entry point" difference.
//
// In C++:
// - Global objects can have constructors/destructors.
// - Such constructors run BEFORE `main()`.
// - Such destructors run AFTER `main()` (during termination).
//
// In C:
// - No constructors/destructors for objects exist in the language.
// - Therefore there is no user-defined "work before main" in the same sense.
//   (There is still CRT startup, but not user constructors.)

struct Tracer
{
    explicit Tracer(const char* name) : name_(name)
    {
        std::cout << "[ctor] " << name_ << "\n";
    }
    ~Tracer()
    {
        std::cout << "[dtor] " << name_ << "\n";
    }
private:
    const char* name_;
};

// Constructed before main in C++.
static Tracer g_before_main("global object (constructed before main)");

static Tracer& safe_singleton()
{
    // Function-local static: constructed on first use.
    // Since C++11: thread-safe initialization.
    static Tracer t("function-local static (constructed on first use)");
    return t;
}

int main()
{
    std::cout << "[main] begin\n";

    (void)safe_singleton();

    std::cout << "[main] end\n";
    return 0;
}

// NOTE: Avoid relying on global initialization order across translation units.
// Use function-local statics or explicit init APIs to avoid the static initialization order fiasco.

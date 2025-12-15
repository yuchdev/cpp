// Chapter 2
// Simple function templates

// Function max value of two

// It's possible to oveload function template with other templates
// so as plain functions

// Plain function overload (we need it just to show how overload works)
// It should be defined before the template for correct overload
inline const int max(const int a, const int b)
{
    return (a > b) ? a : b;
}

// Compare strings (lexigraphcal predicate for string literals)
inline const char* const& max(const char* const& a, const char* const& b)
{
    return (std::strcmp(a, b) < 0) ? a : b;
}

// It's possible to replace 'typename' with 'class'
// however better not to
// typename T is a well-established naming convention
// T is template type, a, b - function arguments
template <typename T>
inline const T& max(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

// C++14 auto return type deduction (impossible in C++11)
template <typename T>
auto add_things(T a, T b) -> decltype(a + b)
{
    const auto ret = a + b;
    return ret;
}

/*
 * template<class T, class U>
 * auto operator+(const Matrix<T>& a, const Matrix<U>& b) -> Matrix<decltype(T{}+U{})>;
 * We use the suffix return type syntax to be able to express the return type in terms of the arguments
 * Important note: decltype has new behavior in C++14
*/


// It's important to understand, template is not some kind of object that magically accept ant type
// Instead a new class of function is created for every type (so called instantiation)

// Template function can be inline

Questions:

New C++14 features:
* Generalized and guaranteed constant expressions: constexpr (2.2.3, 10.4, 12.1.6)
* Suffix return type syntax (12.1.4)
* A syntax for attributes and two standard attributes
* C99 features: extended integral types (i.e., rules for optional longer integer types; 6.2.4); 
  concatenation of narrow/wide strings; __func__ and __STDC_HOSTED__ (12.6.2);
  _Pragma(X) (12.6.3); vararg macros and empty macro arguments (12.6)

Advices:

* The C++ standard algorithms (e.g., find, sort, and iota) provide a good start for functions design
* The most basic advice is to keep a function of a size so that you can look at it in total on a screen
* The cost of a function call is not a significant factor
* To preserve C compatibility, a const is ignored at the highest level of an argument type
* For example, this is two declarations of the same function
void f(int); // type is void(int) 
void f(const int); // type is void(int)
* we can indicate that an argument is unused in a function definition by not naming it
* Constructors are technicallly not functions
* The following two declarations are equivalent
string to_string(int a); // prefix return type 
auto to_string(int a) -> string; // suffix return type
* Prefix auto indicates that the return type is placed after the argument list
* Use for a suffix return type comes in function template declarations in which the return type depends on the arguments
* A call of a void function may be used as the return value of a void function
return g(p); // OK: equivalent to g(p); return;
* A constexpr function cannot have side effects, so writing to nonlocal objects is not possible
* A construct [[...]] is called an attribute and can be placed just about anywhere in the C++ syntax
* Placing [[noreturn]] at the start of a function declaration indicates that the function is not expected to return
* The effect of initializing a local static recursively is undefined
* If an array is used as a function argument, a pointer to its initial element is passed
* These three declarations are equivalent and declare the same function:
void odd(int* p);
void odd(int a[]);
void odd(int buf[1020]);
* The size of an array is not available to the called function
* For some functions, it is not possible to specify the number and type of all arguments expected in a call. 
  To implement such interfaces, we have three choices:
    * Use a variadic template (28.6)
    * Use an initializer_list as the argument type (12.2.3)
    * Terminate the argument list with the ellipsis (...), which means 'and maybe some more arguments'
* If I didn’t have to mimic C style, I would further simplify the code by passing a container as a single argument
* A {}-delimited list can be used as an argument to a parameter of:
    * Type std::initializer_list<T>, where the values of the list can be implicitly converted to T
    * A type that can be initialized with the values provided in the list
    * A reference to an array of T, where the values of the list can be implicitly converted to T
* If there is a possible ambiguity, an initializer_list parameter takes priority
* Default arguments may be provided for trailing arguments only

Automatic Overload Resolution
To approximate our notions of what is reasonable, a series of criteria are tried in order:
* Exact match; that is, match using no or only trivial conversions (for example, array name
to pointer, function name to pointer to function, and T to const T)
* Match using promotions; that is, integral promotions (bool to int, char to int, short to int,
and their unsigned counterparts; 10.5.1) and float to double
* Match using standard conversions (e.g., int to double, double to int, double to long double,
Derived* to Base* (20.2), T* to void* (7.2.1), int to unsigned int (10.5))
* Match using user-defined conversions (e.g., double to complex<double>; 18.4)
* Match using the ellipsis ... in a function declaration (12.2.4)

* Return types are not considered in overload resolution
  The reason is to keep resolution for an individual operator or function call context-independent
* A pointer to function does not allow the code to be modified
* A pointer to a noexcept function can be declared noexcept
* A pointer to function must reflect the linkage of a function (15.2.6) and calling convention
* Neither linkage specification nor noexcept may appear in type aliases:
using Pc = extern "C" void(int); // error: linkage specification in alias 
using Pn = void(int) noexcept; // error: noexcept in alias

* A string can be created by concatenating two strings using the ## macro operator
* A single # before a parameter name in a replacement string means a string containing the macro argument

* Macros can even be variadic. For example:
#define err_print(...) fprintf(stderr,"error: %s %d\n", __VA_ARGS__) 
err_print("The answer",54);

* A few macros are predefined by the compiler 
	* __cplusplus: defined in a C++ compilation (and not in a C compilation). Its value is 201103L
	  in a C++11 program; previous C++ standards have lower values
	* __DATE__: date in ‘‘yyyy:mm:dd’’ format.
	* __TIME__: time in ‘‘hh:mm:ss’’ format.
	* __FILE__: name of current source file.
	* __LINE__: source line number within the current source file.
	* __FUNC__: an implementation-defined C-style string naming the current function.
	* __STDC_HOSTED__: 1 if the implementation is hosted (6.1.1); otherwise 0.
	In addition, a few macros are conditionally defined by the implementation:
	* __STDC__: defined in a C compilation (and not in a C++ compilation)
	* __STDC_MB_MIGHT_NEQ_WC__: 1 if, in the encoding for wchar_t, a member of the basic
	character set (6.1) might have a code value that differs from its value as an ordinary charac-
	ter literal
	* __STDCPP_STRICT_POINTER_SAFETY__: 1 if the implementation has strict pointer safety
	(34.5); otherwise undefined.
	* __STDCPP_THREADS__: 1 if a program can have more than one thread of execution; otherwise undefined

Book advices:
* 'Package' meaningful operations as carefully named functions; 12.1.
* A function should perform a single logical operation; 12.1.
* Keep functions short; 12.1.
* Don’t return pointers or references to local variables; 12.1.4.
* If a function may have to be evaluated at compile time, declare it constexpr; 12.1.6.
* If a function cannot return, mark it [[noreturn]]; 12.1.7.
* Use pass-by-value for small objects; 12.2.1.
* Use pass-by-const-reference to pass large values that you don’t need to modify; 12.2.1.
* Return a result as a return value rather than modifying an object through an argument
* Use rvalue references to implement move and forwarding; 12.2.1.
* Pass a pointer if 'no object' is a valid alternative (and represent 'no object' by nullptr)
* Use pass-by-non-const-reference only if you have to; 12.2.1.
* Use const extensively and consistently; 12.2.1.
* Assume that a char* or a const char* argument points to a C-style string; 12.2.2.
* Avoid passing arrays as pointers; 12.2.2.
* Pass a homogeneous list of unknown length as an initializer_list<T> (or as some other container); 12.2.3.
* Avoid unspecified numbers of arguments (...); 12.2.4.
* Use overloading when functions perform conceptually the same task on different types; 12.3.
* When overloading on integers, provide functions to eliminate common ambiguities; 12.3.5.
* Specify preconditions and postconditions for your functions; 12.4.
* Prefer function objects (including lambdas) and virtual functions to pointers to functions; 12.5
* Avoid macros; 12.6.
* If you must use macros, use ugly names with lots of capital letters; 12.6.
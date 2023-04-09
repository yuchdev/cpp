Functions

* Function pointer types could be defined with typedef or using
* Explicit inline makes sense if
   - its definition is in *.cpp file
   - in template functions (std::max)
* inline could be cancelled by the recursion, cycle, virtual methos or taking function address
* std lib has function pointers callback: new_handler
* If you put the inline function's definition into a .cpp file, and if it is called from some other .cpp file, 
  you'll get an "unresolved external" error from the linker
* Function default argument value may be function call 
* The C++ standard algorithms (e.g., find, sort, and iota) provide a good start for functions design
* The most basic advice is to keep a function of a size so that you can look at it in total on a screen
* The cost of a function call is not a significant factor
* To preserve C compatibility, a const is ignored at the highest level of an argument type
* For example, this is two declarations of the same function
```
void f(int); // type is void(int) 
void f(const int); // type is void(int)
```
* we can indicate that an argument is unused in a function definition by not naming it
* Constructors are technicallly not functions
* The following two declarations are equivalent
string to_string(int a); // prefix return type 
auto to_string(int a) -> string; // suffix return type
* Prefix auto indicates that the return type is placed after the argument list
* Use for a suffix return type comes in function template declarations in which the return type depends on the arguments
* A call of a void function may be used as the return value of a void function
return g(p); // OK: equivalent to g(p); return;
* A `constexpr` function cannot have side effects, so writing to nonlocal objects is not possible
* A construct `[[...]]` is called an attribute and can be placed just about anywhere in the C++ syntax
* Placing `[[noreturn]]` at the start of a function declaration indicates that the function is not expected to return
* The effect of initializing a local static recursively is undefined
* If an array is used as a function argument, a pointer to its initial element is passed
* These three declarations are equivalent and declare the same function:
```
void odd(int* p);
void odd(int a[]);
void odd(int buf[1020]);
```
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

## Automatic Overload Resolution

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

## Macros

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

## Initializer List

* The type of a {}-list can be deduced (only) if all elements are of the same type
* Unfortunately, we do not deduce the type of an unqualified list for a plain template argument
```
template<typename T> void f(T);
f({}); // error: type of initializer is unknown
f({1}); // error: an unqualified list does not match 'plain T'
```

The implementation model for {}-lists comes in three parts:

* If the {}-list is used as constructor arguments, the implementation is just as if you had used a ()-list. List elements are not copied except as by-value constructor arguments.
* If the {}-list is used to initialize the elements of an aggregate (an array or a class without a constructor), each list element initializes an element of the aggregate. List elements are not copied except as by-value arguments to aggregate element constructors.
* If the {}-list is used to construct an initializer_list object each list element is used to initialize an element of the underlying array of the initializer_list. Elements are typically copied from the initializer_list to wherever we use them

A unqualified list is used where an expected type is unambiguously known. It can be used as an expression only as:

* A function argument `f({0})`
* A return value `return {0};`
* The right-hand operand of an assignment `operator (=, but not +=, *=, etc.) v = {0};`
* An array subscript `[{0}]`


## Lambda

* The body of the lambda simply becomes the body of the operator()()
* lambda is called a closure object (or simply a closure)
* If a lambda potentially captures every local variable by reference (using the capture list [&]), the
  closure may be optimized to simply contain a pointer to the enclosing stack frame
* Naming the lambda is often a good idea
* If you need to capture a variadic template (28.6) argument, use ...
* A lambda might outlive its caller. Use copy capture then [=]
* We don't need to 'capture' namespace variables (including global variables) because they are always accessible
* We can include class members in the set of names potentially captured by adding [this]
* Members are always captured by reference
* In the unlikely event that we want to modify the state, we can declare the lambda mutable
* If a lambda body does not have a return-statement, the lambda's return type is void
* To allow for optimized versions of lambda expressions, the type of a lambda expression is not defined
* so no two lambdas have the same type
* we can use it to initialize a variable declared auto or std::function<R(AL)> where R is the lambda's return type and AL is its argument list
* Truncation of floating-point numbers is not 'well behaved'
* If rounding is desirable, we can use the standard-library function round()
* make type conversion more visible and to allow the programmer to express the intent of a cast


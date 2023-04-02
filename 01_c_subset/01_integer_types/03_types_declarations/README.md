### Types and defenitions

* Many important things are deemed implementation-defined by the standard
* Other behaviors are unspecified. The exact value returned by new is unspecified
* Undefined behavior is nastier. A construct is deemed undefined by the standard 
  if no reasonable behavior is required by an implementation
* A C++ implementation can be either hosted or freestanding
	* A hosted implementation includes all the standard-library facilities as described in the standard
	* Freestanding implementations are meant for code running with only minimal operating system support
* The Boolean, character, and integer types are collectively called integral types
* In arithmetic and logical expressions, bools are converted to ints
* A pointer can be implicitly converted to a bool
* There are many character sets and character set encodings in use. C++ provides a variety of character types
  char, signed/unsigned char, whar_t, char16_t, char32_t
* The notation int{c} gives the integer value for a character c 
* The choice of signed or unsigned for a plain char is implementation-defined
* A few characters have standard names that use the backslash, \ , as an escape character
* Literals of such larger character preceded by a U or a u
* If you need more detailed control over integer sizes, you can use aliases from <cstdint>
* In general, the type of an integer literal depends on its form, value, and suffix
* The reason for providing more than one integer type, more than one unsigned type, 
  and more than one floating-point type is to allow the programmer 
  to take advantage of hardware characteristics
* This is what is guaranteed about sizes of fundamental types:
    * 1 = sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)
    * 1 <= sizeof(bool) <= sizeof(long)
    * sizeof(char) <= sizeof(wchar_t) <= sizeof(long)
    * sizeof(float) <= sizeof(double) <= sizeof(long double)
    * sizeof(N) == sizeof(signed N) == sizeof(unsigned N)
* Some implementation-defined aspects of fundamental types can be found by a simple use of sizeof, 
  and more can be found in <limits>
* On some machine architectures, the bytes used to hold it must have proper alignment
  for the hardware to access it efficiently
* The alignof() operator returns the alignment of its argument expression
* Sometimes, we have to use alignment in a declaration, such as alignof(x+y) is not allowed. 
  Instead, we can use the type specifier alignas()
* Some type names don't even look much like names, such as decltype(f(x)) - the return type of a call f(x)
* An initializer can use one of four syntactic styles:
X a1 {v}; 
X a2 = {v}; 
X a3 = v; 
X a4(v);
* Initialization using {}, list initialization, does not allow narrowing 
* decltype(expr) for deducing the type of something that is not a simple initializer
* We can decorate auto type with specifiers and modifiers (6.3.1), such as const and & 
* Better to use the = syntax with auto, because the {}-list syntax might surprise someone:
auto v1 {12345}; // v1 is a list of int
* The 'using' keyword can also be used to introduce a template alias
* We cannot apply type specifiers, such as unsigned, to an alias
using Uchar = unsigned Char; // error 
using Uchar = unsigned char; // OK

#### Scopes
* Statement scope: A name is in a statement scope if it is defined within the  part of a for-, while-, if-, or switch-statement
* Local scope: A name declared in a function  is called a local name
* Class scope: A name is called a member name  if it is defined in a class outside any function
* Namespace scope: A name is called a namespace member name if it is defined in a namespace  
  outside any function, lambda , or other namespace
* Global scope: A name is called a global name if it is defined outside any function, class , or namespace

#### Using vs typedef
* Using can be templated
* using produces very clean function pointers
* while using template alias does not need typename keyword
* typedef allows redeclaration, which could be a source of errors
* using is more strict in terms of place declaring new type (e.g. can't declare template inside the class)

### New C++14 features
* Uniform and general initialization using {}-lists (2.2.2, 6.3.5)
* Type deduction from initializer: auto (2.2.2, 6.3.6.1)
* Prevention of narrowing (2.2.2, 6.3.5)
* Type and template aliases (3.4.5, 6.5, 23.6)
* Unicode characters (6.2.3.2, 7.3.2.2)
* long long integer type (6.2.4)
* Alignment controls: alignas and alignof(6.2.9)
* The ability to use the type of an expression as a type in a declaration: decltype 
* C99 features (long long)
* Integer type names, such as int16_t, uint32_t, and int_fast64_t (6.2.8)

#### TODOs
* alisnof and alignas + pragma pack()
* bitwise repr of cstdint types
* relate to [alognof()](https://en.cppreference.com/w/c/language/object)
* TODO: why fp multiplication faster than integer?
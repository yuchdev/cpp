Questions:
Ensure about func_ptr -> void* (Standard)

New C++14 features:

* Null pointer keyword (7.2.2)
* Rvalue references (enabling move semantics)(7.7.2)
* Unicode characters (6.2.3.2, 7.3.2.2)
* Raw string literals (7.3.2.1)
* Scoped and strongly typed enums: enum class (2.3.3, 8.4.1)
* Generalized POD (8.2.6)
* Generalized unions (8.3.1)
* A fixed-sized contiguous sequence container: array (8.2.4, 34.2.1)
* Generalized and guaranteed constant expressions: constexpr (2.2.3, 10.4, 12.1.6)
* Range-for-statement (2.2.5, 9.5.1)

Advices

* The logical operators && (and), || (or), and ! (not) take operands of arithmetic and pointer types, convert them to bool, and return a bool result
* The && and || operators evaluate their second argument only if necessary, so they can be used to control evaluation order
* A plain enum can be implicitly converted to an integer type and used as an operand to bitwise logical operations
* int size could be checked with static assert so that work with masks
* Whenever possible, use standard-library facilities in preference to fiddling with pointers and bytes
* Wherever possible, have that manager object being a scoped variable
* Usually two or more words per allocation are used for free-store management


* A pointer to any type of object can be assigned to a variable of type `void*`, but a pointer to member (20.6) cannot
* The standard conversions allow 0 to be used as a constant of pointer or pointer-to-member type
* In C, NULL is typically `(void*)0`, which makes it illegal in C++
* A string literal is statically allocated
* The backslash convention for representing nongraphic characters
* Raw string literals use the `R"(ccc)"` notation for a sequence of characters "ccc"
* Unless you work with regular expressions, raw string literals are probably just a curiosity
* Similarly, a string with the prefix LR, such as `LR"(angst)"`
* Basically, constexpr's role is to enable and ensure compile-time evaluation, 
  whereas const's primary role is to specify immutability in interfaces
* To reflect the lvalue/rvalue and const/non-const distinctions, there are three kinds of references:
  * lvalue references: to refer to objects whose value we want to change
  * const references: to refer to objects whose value we do not want to change (a constant)
  * rvalue references: to refer to objects whose value we do not need to preserve after we have used it (a temporary)
* We use rvalue references to implement a 'destructive read' for optimization
* Since move(x) does not move x (it simply produces an rvalue reference to x), 
  it would have been better if move() had been called rval()
* Rvalue references can also be used to provide perfect forwarding
* Reference to reference is just a reference. This is sometimes known as reference collapse
* Use of multiple access specifiers (i.e., public, private, or protected) can affect layout (20.5)
* Sometimes, we want to treat an object as just 'plain old data' (a contiguous sequence of bytes in memory)
* Bit field saves data space, but the size of the code needed to manipulate these variables increases
* The language doesn't keep track of which kind of value is held by a union
* On some machines, an int and an int* do not occupy the same amount of space
* Union that holds current type info is often called a tagged union or a discriminated union
* An anonymous union is an object, not a type, and its members can be accessed without mentioning an object name
* In general, prefer the enum classes
* The underlying type must be one of the signed or unsigned integer types (6.2.4)
  the default is int. We could be explicit about that
* enum is a user-defined type, so we can define the | and & operators
* A plain enum can be unnamed
* The expression in the case labels must be a constant expression of integral or enumeration type
* The compiler uses v and v+N as begin(v) and end(v) for a built-in array `T v[N]`
* C++ provides assignment operators for the binary operators
* The order of evaluation of subexpressions within an expression is undefined
* A constant expression is an expression that a compiler can evaluate
* Symbolic names should be used systematically to avoid 'magic numbers' in code
* In many cases, plain enumerators (8.4) are another alternative to consts
* A class with a constexpr constructor is called a literal type
* To be simple enough to be constexpr, a constructor must have an empty body 
  and all members must be initialized by potentially constant expressions
* Note that we can have constexpr arrays and also access array elements and object members
* The address of a statically allocated object (6.4.2), such as a global variable, is a constant
* Value of fp conversion bigger->smaller is undefined


The main problems with free store are:
* Leaked objects: People use new and then forget to delete the allocated object.
* Premature deletion: People delete an object that they have some other pointer to and later
use that other pointer
* Double deletion

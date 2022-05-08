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

Advices:
* A pointer to any type of object can be assigned to a variable of type void*, 
  but a pointer to member (20.6) cannot
* The standard conversions allow 0 to be used as a constant of pointer or pointer-to-member type
* In C, NULL is typically (void*)0, which makes it illegal in C++
* A string literal is statically allocated
* The backslash convention for representing nongraphic characters
* Raw string literals use the R"(ccc)" notation for a sequence of characters "ccc"
* Unless you work with regular expressions, raw string literals are probably just a curiosity
* Similarly, a string with the prefix LR, such as LR"(angst)"
* Basically, constexpr's role is to enable and ensure compile-time evaluation, 
  whereas const's primary role is to specify immutability in interfaces
* To reflect the lvalue/rvalue and const/non-const distinctions, there are three kinds of references:
	* lvalue references: to refer to objects whose value we want to change
	* const references: to refer to objects whose value we do not want to change (a constant)
	* rvalue references: to refer to objects whose value we do not need to preserve after we have used it (a temporary)
* We use rvalue references to implement a 'destructive read' for optimization
* Since move(x) does not move x (it simply produces an rvalue reference to x), 
  it would have been better if move() had been called rval()
* Rvalue references can also be used to provide perfect forwarding (23.5.2.1, 35.5.1)
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
* The compiler uses v and v+N as begin(v) and end(v) for a built-in array T v[N]
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

Book advices:
* Keep use of pointers simple and straightforward
* Avoid non-trivial pointer arithmetic
* Take care not to write beyond the bounds of an array
* Avoid multidimensional arrays
* Use nullptr rather than 0 or NULL
* Use containers (e.g., vector, array, and valarray) rather than built-in (C-style) arrays
* Use string rather than zero-terminated arrays of char
* Use raw strings for string literals with complicated uses of backslash
* Prefer const reference arguments to plain reference arguments
* Use rvalue references (only) for forwarding and move semantics
* Keep pointers that represent ownership inside handle classes
* Avoid void* except in low-level code
* Use const pointers and const references to express immutability in interfaces
* Prefer references to pointers as arguments, except where 'no object' is a reasonable option
* When compactness of data is important, lay out structure data members with larger members before smaller ones
* Use bit-fields to represent hardware-imposed data layouts
* Don�t naively try to optimize memory consumption by packing several values into a single byte
* Use unions to save space (represent alternatives) and never for type conversion
* Use enumerations to represent sets of named constants
* Prefer class enums over 'plain' enums to minimize surprises
* Define operations on enumerations for safe and simple use
* Don�t declare a variable until you have a value to initialize it with
* Prefer a switch-statement to an if-statement when there is a choice
* Prefer a range-for-statement to a for-statement when there is a choice
* Prefer a for-statement to a while-statement when there is an obvious loop variable
* Prefer a while-statement to a for-statement when there is no obvious loop variable
* Avoid do-statements
* Avoid goto
* Keep comments crisp
* Don�t say in comments what can be clearly stated in code
* State intent in comments
* Maintain a consistent indentation style
* Prefer the standard library to other libraries and to 'handcrafted code'
* Use character-level input only when you have to
* When reading, always consider ill-formed input
* Prefer suitable abstractions (classes, algorithms, etc.) to direct use of language features (e.g.,
ints, statements)
* Avoid complicated expressions
* If in doubt about operator precedence, parenthesize
* Avoid expressions with undefined order of evaluation
* Avoid narrowing conversions
* Define symbolic constants to avoid 'magic constants'

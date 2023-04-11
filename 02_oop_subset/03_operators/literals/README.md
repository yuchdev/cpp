## TODOs
* ternary digits example
* complex type example
* string type example

## New C++14 features
* Explicit conversion operators (18.4.2)
* User-defined literals (19.2.6)

## Operators
* Note that the C++ grammar is written so that the {} notation can only be used 
  for initializers and on the right-hand side of an assignment
* The following operators cannot be defined by a user:
	* :: scope resolution
	* . member selection
	* .* member selection through pointer to member
* It is not possible to define new operator tokens
* A unary operator, whether prefix or postfix, can be defined by either a non-static member function 
  taking no arguments or a nonmember function taking one argument
* The operators operator=, operator[] , operator() , and operator-> must be non-static member functions
* The default meaning of &&, ||, and , (comma) involves sequencing: the first operand is evaluated before the second 
  (and for && and || the second operand is not always evaluated)
* The operators= (assignment), &(address-of), and , (sequencing) have predefined meanings when applied to class objects
  These predefined meanings can be eliminated (deleted)
* It is not possible to define an operator function that operates exclusively on pointers
* Enumerations are user-defined types so that we can define operators for them
* For small objects, say, one to four words, call-by-value is typically a viable alternative
* Performance of argument passing and use depends on machine architecture, compiler interface conventions 
  (Application Binary Interfaces; ABIs), and the number of times an argument is accessed 
  (it almost always is faster to access an argument passed by value than one passed by reference)
* For large objects, such as a Matrix, define move operations to make such transfers of values efficient
* Note that in operator lookup no preference is given to members over nonmembers. This differs from lookup of named functions
* Prefer to minimize the number of functions that directly manipulate the representation of an object. 
  This can be achieved by defining only operators that inherently modify the value of their first argument, 
  such as +=, in the class itself
* It is possible to go further and introduce a user-defined literal
* Type conversion can be accomplished by
	* A constructor taking a single argument
	* A conversion operator
* In either case the conversion can be
	* Explicit; that is, the conversion is only performed in a direct initialization, ie, an initializer not using a =
	* Implicit; that is, it will be applied wherever it can be used unambiguously, e.g., as a function argument
* These problems can be handled by defining a conversion operator
* It is not a good idea to define an implicit conversion in such a way that information is lost in the conversion
* The best idea is initially to do conversions by named functions, such as X::make_int()
* It is possible to declare a conversion operator explicit and have it apply only for direct initialization
* In some cases, a value of the desired type can be constructed in more than one way; such cases are illegal
* The call operator() is also known as the application operator
* Note that a lambda expression is basically a syntax for defining a function object
* Function call operators are often templates
* Operator -> must be a non-static member function. Its return type must be a pointer or an object of a class to which you can apply ->
* The pre-increment operator can return a reference to its object. The post-increment operator must make a new object to return
* Member operator new()s and operator delete()s are implicitly static members
* How does a compiler know how to supply the right size to operator delete()?
  The type specified in the delete operation matches the type of the object being deleted. 
  If we delete an object through a pointer to a base class, that base class must have a virtual destructor 
  for the correct size to be given
* User-defined literals are supported through the notion of literal operators that map literals 
with a given suffix into a desired type
* The basic (implementation) idea is that after parsing what could be a literal, the compiler always checks for a suffix
* It is not possible to redefine the meaning of a built-in literal suffix or to augment the syntax of literals
* There are four kinds of literals that can be suffixed to make a user-defined literal
	* An integer literal: unsigned long long or a const char* argument or by a template literal operator, 
	for example, 123m or 12345678901234567890X
	* A floating-point literal: long double or a const char* argument or by a template literal operator, 
	for example, 12345678901234567890.976543210x or 3.99s
	* A string literal: (const char*, size_t) pair of arguments, for example, "string"s and R"(Foo\bar)"_path
	* A character literal: type char, wchar_t, char16_t, or char32_t, for example, 'f'_runic or u'BEEF'_w
* A literal operator converting numerical values to strings could be quite confusing
* A template literal operator is a literal operator that takes its argument as a template parameter pack, 
  rather than as a function argument. For example:
template<char...>
constexpr int operator"" _b3(); // base 3, i.e., ternary

* The variadic template techniques can be disconcerting, but it is the only way 
  of assigning nonstandard meanings to digits at compile time
* The standard library reserves all suffixes not starting with an initial underscore, 
  so define your suffixes starting with an underscore or risk your code breaking in the future

* An ordinary member function declaration specifies three logically distinct things:
	1. The function can access the private part of the class declaration.
	2. The function is in the scope of the class.
	3. The function must be invoked on an object (has a this pointer).
  By declaring a member function static, we can give it the first two properties only. 
  By declaring a nonmember function a friend, we can give it the first property only
* A friend declaration can be placed in either the private or the public part of a class declaration
* A member function of one class can be the friend of another
* It is possible to make a template argument a friend:
friend T;
friend class T; // redundant class

* A friend must be previously declared in an enclosing scope or defined in the non-class scope 
  immediately enclosing the class that is declaring it to be a friend. 
  Scopes outside the innermost enclosing namespace scope are not considered for a name first declared as a friend (iso.7.3.1.2)
  (see examples)
* A friend function should be explicitly declared in an enclosing scope or take an argument of its class or a class derived from that
* An operation modifying the state of a class object should therefore be a member or a function taking a non-const reference argument 
  (or a non-const pointer argument)
* Binary operators are the most common source of friend functions
* The member function call syntax makes it clear to the user that the object may be modified; a reference argument is far less obvious

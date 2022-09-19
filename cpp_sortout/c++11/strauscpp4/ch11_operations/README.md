Questions:
* emplace {} and copy = {} difference: show_emplace_copy()  (Standard)
* More C++14 lambda examples

New C++14 features:
* Lambdas (3.4.3,11.4)
* Local classes as template arguments (11.4.2, 25.2.1)

Advices:


* The logical operators && (and), || (or), and ! (not) take operands of arithmetic and pointer types,
  convert them to bool, and return a bool result
* The && and || operators evaluate their second argument only if necessary, so they can be used to control evaluation order
* A plain enum can be implicitly converted to an integer type and used as an operand to bitwise logical operations
* int size could be checked with static assert so that work with masks
* Whenever possible, use standard-library facilities in preference to fiddling with pointers and bytes
* Wherever possible, have that manager object being a scoped variable
* Usually two or more words per allocation are used for free-store management
* The type of a {}-list can be deduced (only) if all elements are of the same type
* Unfortunately, we do not deduce the type of an unqualified list for a plain template argument
template<typename T> void f(T);
f({}); // error: type of initializer is unknown
f({1}); // error: an unqualified list does not match 'plain T'

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
* If a lambda body does not have a return-statement, the lambda�s return type is void
* To allow for optimized versions of lambda expressions, the type of a lambda expression is not defined
* so no two lambdas have the same type
* we can use it to initialize a variable declared auto or std::function<R(AL)>
  where R is the lambda�s return type and AL is its argument list
* Truncation of floating-point numbers is not 'well behaved'
* If rounding is desirable, we can use the standard-library function round()
* make type conversion more visible and to allow the programmer to express the intent of a cast

The main problems with free store are:
* Leaked objects: People use new and then forget to delete the allocated object.
* Premature deletion: People delete an object that they have some other pointer to and later
use that other pointer
* Double deletion

The implementation model for {}-lists comes in three parts:
* If the {}-list is used as constructor arguments, the implementation is just as if you had used a
()-list. List elements are not copied except as by-value constructor arguments.
* If the {}-list is used to initialize the elements of an aggregate (an array or a class without a constructor),
 each list element initializes an element of the aggregate. 
 List elements are not copied except as by-value arguments to aggregate element constructors.
* If the {}-list is used to construct an initializer_list object
 each list element is used to initialize an element of the underlying array of the initializer_list. 
 Elements are typically copied from the initializer_list to wherever we use them

A unqualified list is used where an expected type is unambiguously known. It can be used as an expression only as :
* A function argument f({0})
* A return value return {0};
* The right-hand operand of an assignment operator (=, but not +=, *=, etc.) v = {0};
* An array subscript [{0}]

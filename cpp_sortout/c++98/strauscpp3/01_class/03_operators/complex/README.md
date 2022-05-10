# Operators overload

#### Content  

* Operators that can or cannot be overloaded
* Operators members and non-members of classes
* Operators that can be overloaded only by class members
* Operators that are better to overload with functions
* Ambiguity of operator overloading
* Operators in namespaces
* Implicit '=' transformation in a constructor and prohibiting it
* Type casting operators
* Operators and friends
* Returning by value and by reference


#### General information

* Operators [] () = -> {type} can only be only class members
* You cannot overload operators :: . .* ?
* You cannot use the default parameters in operators, because otherwise you could theoretically make an unary frunction from binary
* The language does not allow for changing operator precedence or associativity
* The language does not allow declaring new operators
* In theory new operations could be declared by implementing a wrapper class over some built-in type.
  E.g. `class Real`, which implements raising to a power using `operator^`
  However, we do not recommend doing so
* The prefix form of the increment/decrement operatora returns reference, the postfix returns value


#### Operators design

* Declare intuitive operators. Naming some accessor function `operator->` "just because it's cool" will do a bad job in future
* Overload commutative operations and input-output operations as functions. Overload the rest as methods.
* Use namespaces to create a unified interface between friend functions and their class
* When overloading operators, implement basic operations and then reuse their code in other operators (+= +, -= -, etc.)
* Use the `operator()` for multidimensional indexing
* Overload 2 forms of `operator()` and `operator[]` for constant and non-constant version 
  (non-constant version must return reference in odeer to be l-value)
* Be careful when overloading type conversion operators, ambiguities or implicit unwanted conversions may occur.
  E.g. `class A` can be created from `class B`, `class B` can be converted to `class A`
* For enumerations you can also define operators
* The operators offering an access to a class state, e.g. `operator+=`, `operator*=`, `operator=` 
  return the current object by reference (in order to make it l-value)
* if `operator=` is defined, don't forget to update it after each addition of a new member
* Don't forget to call explicitly the base class `operator=`, if your class is derived
* This can be forbidden explicitly if necessary, by returning a const reference
* The value is not returned when overloading the type conversion operators
* Friends can be declared in both public or private sections of the class, it does not matter
* The operator -> implements the concept of indirect access, which could be seen in iterators
* Member operators do not overload non-member, members always have priority
* In high-performance environment it is sometimes worth abandoning implicit conversions in operators in order to improve performance
  Such operators could be explicitly overloaded, or better implemented as template functions.
* Overload of logical operators `operator&&`, `operator||`, `operator~`, etc, is extremely rare, so it is hardly expected
* In addition, overloading binary logical operatora `operator&&` and `operator||` would not lead to expected "lazy evaluation", 
  all expressions will be evaluated, and the order of evaluation won't be defined
* Constructors of smart pointers usually declared explicit

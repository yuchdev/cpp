Chapters 2-5 Nicolai Josuttis

Simple function templates. Simple class templates.
Specialization, partial specialization, default arguments.
Keyword typename. The X.template construct.
Using this-> construct
Templates - class members. Template parameters.
Initializing with null. Using literals.

* For template functions, overloading is possible, both with other template functions and with plain functions.
* You can explicitly specify the use of the template with <>
* When preferring one overload over another watch the order of declaration.
* If the template parameter is a type, use typename keyword. typename T is the common form
* Template code is compiled 2 times - when checking for syntactic correctness, and diring the instantiation
* It is necessary that each operation for a template class is defined and valid.
* Tenplate functions can be inline
* Instantiation in classes occurs only for methods that actually being called
* For a specialized template <T> construct is not specified in class declaration, and template <> is not written in the method definition
* Partial specialization when the template has several parameters, but one of them is specialized.
  E.g. "both parameters are the same, the second parameter is int, both parameters are pointers"
  Doing so cerefully watch for possible ambiguities
* The default arguments should be the last in the argument list
* Non-type template parameters could be
  * integer constants (including enums)
  * pointers or references to objects with external linkage
* The keyword typename is used not only to specify parameter-types, but also to indicate that a given construction derived from a parameter is a type
  Applying the typename keyword towards the iterator is a standard pattern.
*  For template methods you need to explicitly specify this-> This could be necessary for achieving virtuality
   The point is that external functions are considered first when resolving names.
   For static methods you must also explicitly specify Base<T>::
*  A template method or class can be a class member (both template and plain class)
   For such methods implemented outside the class, spicifying template <> twice is neccessary
* Another template could be a template param
* Template parameters and partial specialization are only allowed for class templates.
* Template parameters can be zero-initialized (), {}
* The conversion of an array (string literal) into a pointer occurs only if the name of the array is passed by value

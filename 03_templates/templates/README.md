### Templates

* It is good practice to test a class or function before converting it into a template
* Always declare the general form of the template before the specializations
* You will likety need partial specializations for C-strings and C-arrays
* Template arguments could be used to pass algorithm in compile-time
* Template arguments could be used to define possible conversions
* Template arguments could be checked in compile-time thanks to concepts and SFINAE
* Prefer templates over derived classes when runtime efficiency matters
* Differences between a function template over a class template:
  * overloading with regular functions is possible
  * arguments deduction is possible
  * partial specialization is impossible
* You can replace `typename` keyword with `class`, but does not have much sense in modern C++
  If the template parameter is type, `typename T` is the common form
* One specialization is considered more specialized than the other 
  if each its arguments match to the first specialization
* Indirect call of a method by its pointer using `.*` or `->*` operations
* A pointer to a regular method cannot be assigned to a function pointer
* A pointer to a static method in fact is a function pointer
* It is handy to improve the class readability by introducing aliaces,
  using keywords `typedef` for pre-C++11, and `using` starting from C++11
* Templates can be friends and templates can have friends
* The friend functions of a template can be defined in the template itself
* It usually happens when the friend function parameters depend on the template argument
  If they do not, it would lead to ODR violation and compilation error
* Non-type template parameters can be: integer constants, pointers and references to externally linked objects
* The template argument can be used to define other types later
* Example of deriving template from a class is a static counter of object instances
* From the point of C++ two classes generated from the same template are not connected by any inheritance relations
* Generic algorithms are usually inplemented using templates
* Algorithm itself can be a template argument
* Template class can be abstract
* Template constructors and copy operators never replace default constructors and operators
* Inetesting pattern of template programming: inheriting partial from full specialization to reduce code duplication 
* info about passed types (traits, strategies) could be fetched from compiler info

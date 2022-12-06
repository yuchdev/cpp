Questions:
http://habrahabr.ru/post/264157/
http://habrahabr.ru/post/157961/
http://habrahabr.ru/post/226229/
http://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c
Perfect Forwarding, Universal References
Friend template operators example

Links:
http://habrahabr.ru/post/242639/

New C++14 features:
* Type and template aliases (23.6)
* Simpler and more general SFINAE rule (23.5.3.2)
* Reference forwarding (23.5.2.1)

Advices:
* When designing a class template, it is usually a good idea to debug a particular class before turning it into a template
* We call template param check predicate a concept
* A concept is not (yet) a language construct in C++; it is a notion that we can use to reason about requirements on template arguments
* Aliases do not introduce new types, so String<Uchar> and String<uchar> are the same type as String<unsigned char>
* Errors that relate to the use of template parameters cannot be detected until the template is used
* The implementation is allowed to postpone essentially all checking until the program is linked, 
  and for some errors link time is also the earliest point when complete checking is possible
* In addition, a class template can declare friends, just as an ordinary class
* Non-static data members can be const, but unfortunately not constexpr
* Similarly, a member function of a template can be virtual or not
* However, a virtual member function cannot also be a member function template
* Type names as member aliases are often referred to as associated types (iterator, value_type)
* If a class is missing a desired member alias, a trait can be used to compensate
* As for non-template classes, a const or conexpr static data member of literal type can be initialized in-class 
* A template constructor is never used to generate a copy constructor, so without the explicitly declared copy constructor, 
  a default copy constructor would have been generated
* Similarly, copy assignments, move constructors, and move assignments must be defined as non-template
* Avoid nested types in templates unless they genuinely rely on every template parameter
  This is a special case of the general rule to avoid unnecessary dependencies in code
* Naturally, the interesting cases are those where the friend depends on a template argument
* As ever, friendship is neither inherited nor transitive
* There is no way of saying that Link<X> should only be a friend of List<X> (hm, specialization?)
* The template arguments are deduced from the function arguments
* Class template parameters are never deduced. The reason is that the flexibility 
  provided by several constructors for a class would make such deduction impossible
* Instead, specialization provides a mechanism for implicitly choosing between alternative definitions of a template
* If we need to create an object of a deduced type, we can often do that by calling a function to do the deduction (and creation)
  For example, consider a simple variant of the standard library make_pair()
* If a template parameter can be deduced from more than one function argument, the same type must be the result of each deduction
  Otherwise, the call is an error. Ex:
template<typename T> 
void f(T i, T* p);
// error, ambiguous: T is int or T is const char?
f(i,"Remember!");

* We apply the usual function overload resolution rules to these specializations and all ordinary functions:
	1. Find the set of function template specializations
	2. If two function templates can be called and one is more specialized than the other, consider only the most specialized template function
	3. Do overload resolution for this set of functions, plus any ordinary functions, as for ordinary functions
	4. If a function and a specialization are equally good matches, the function is preferred
	5. If no match is found, the call is an error
* We don't apply promotions and standard conversions until after template parameters have been uniquely determined
* There is no rule telling the compiler to prefer one resolution over the other
* using an explicit specialization of the template is an easy way of defining such resolution functions
* Considering this possible declaration is not by itself an error
  There is a language rule (iso.14.8.2) that says that such a substitution failure is not an error
  It simply causes the template to be ignored; that is, the template does not contribute a specialization to the overload set
* This rule gives us a general tool for selecting among templates
  In particular, the standard library provides enable_if to simplify conditional definition of templates
* A template specialization is considered nonsensical if it would lead to a type error
  In this, we consider only a declaration; template function definitions
  and the definition of class members are not considered (or generated) unless they are actually used
* If the argument has to be deduced from an actual argument of a function call, conversions are not applicable to the first argument
* Notation with f<T>() is sometimes called explicit specialization
* The 'using' syntax is more general in the important sense that it can be used 
  to define an alias for a template with some of its arguments bound
* The equivalence of the alias and the original template implies that if you specialize the template, 
  you (correctly) get the specializations when you use the alias
* As ever, non-inline, non-template functions and static members must have a unique definition in some compilation unit
  This implies that such members are best not used for templates that are otherwise included in many translation units
* The rules for linkage of templates are the rules for linkage of the generated classes and functions
* It is possible to minimize the exposure to changes in complicated template libraries 
  by encapsulating their use in functions with non-template interfaces
* It could be useful complex, esoteric, or frequently changing template libraries


Book advices: 
Questions:
TODO: tuple example
TODO: SI units example
TODO: traits
especially regex_traits
TODO: enable_if example

New C++14 features:
* Variadic templates (28.6)
* Tuples (28.5)
* Predicates


Advices:
* There are two main reasons for using metaprogramming techniques:
	* Improved type safety
	* Improved run-time performance
* An infinite compile-time recursion will be caught by running out of some compile-time resource, 
  such as the number of recursive constexpr calls, the number of nested classes, 
  or the number of recursively nested template instantiations
* Four levels of implementation complexity:
	1. No computation (just pass type and value arguments)
	2. Simple computation (on types or values) not using compile-time tests or iteration, for example, 
	   && of Booleans (24.4) or addition of units (28.7.1)
	3. Computation using explicit compile-time tests, for example, a compile-time if (28.3)
	4. Computation using compile-time iteration (in the form of recursion)
* Calling a constexpr function hiding a metaprogram or extracting the type from a template type function 
  is not in itself metaprogramming. It just uses a metaprogram
* STD function, thread, and tuple, are examples of relatively simple application of metaprogramming techniques
* The standard-library template conditional is a compile-time selector between two alternatives
* Compile time selection (stack or heap?) example is not hypothetical. For example, the C++ standard contains 
  the following comment in its definition of the function type
* When the standard provides a type function, such as conditional, I define a corresponding type alias:
template<typename C, typename T, typename F>
using conditional = typename std::conditional<C,T,F>::type;
* The std::is_pod predicate is one of the many provided by the standard library
* Add functions to provide the conventional notation in all contexts (see example)
* We can define our own type predicates (see example)
* Note the parentheses around condition with < or >; without those, we would have gotten a syntax error 
  because the compiler would have interpreted the > as the end of the template argument list
* A trait is used to associate properties with a type
* You can see a trait as a type function with many results or as a bundle of type functions
* The standard library provides allocator_trait, char_traits, iterator_traits, regex_traits, pointer_traits
* In addition, it provides time_traits and type_traits, which confusingly are simple type functions
* Consider how to write the previous example without any mention of traits or other type functions:
using value_type = decltype(*p); 
using difference_type = decltype(q-p);
* We need a trait (or equivalent, such as decltype()) to associate a type with another type, such as a value_type with a T
* When a trait is used simply to provide a name for something that already has a perfectly good name, such as pointer for value_type*,
  the utility is less clear and the potential for confusion greater. Don't blindly define traits for everything 'just in case'
* How would you know that you have gone 'too far' with metaprogramming? One warning sign that I use is an urge to use macros
* We can write a somewhat curious type function to make the definition of operator->() conditional
  The standard library (in <type_traits>) provides enable_if for that
* The language itself defines -> only for pointers to class objects 
* I introduced the enable_if as a default template argument because that is the most general solution
template<typename Iter, typename = enable_if<Input_iterator<Iter>(),Iter>>
vector(Iter b, Iter e);

* we could alternatively apply it to the constructor argument type
template<typename Iter>
vector(enable_if<Input_iterator<Iter>(),Iter>> b, Iter e);

* Question 'Can we call f(x) if x is of type X?' Defining has_f to answer that question gives an opportunity
  to demonstrate some of the techniques used (see example)
* Note that decltype() does not evaluate its operand
* I use decltype() partly to show how in general to derive the return type from a previously defined operator
auto operator!=(const T& a, const T& b) -> decltype(!(a==b))
// operator == for T could be overloaded!

* The compiler optimizes away the empty base classes. This is called the empty-base optimization and is guaranteed by the language
* The Args... defines what is called a parameter pack. A parameter pack is a sequence of (type/value) pairs from which you can 
  'peel off' arguments starting with the first
* The standard library provides std::is_integral and std::is_floating_point
* For std::tuple the memory layout is not specified by the C++ standard. For example, it might be the reverse
* Expansion of a parameter pack into its elements is not restricted to function calls. For example:
template<typename... Bases> 
class X : public Bases... {

* We can use the ellipsis to mean 'zero or more elements of something' in most places where a list of elements is required:
	* A template argument list
	* A function argument list
	* An initializer list
	* A base specifier list
	* A base or member initializer list
	* A sizeof... expression
* A sizeof... expression is used to obtain the number of elements in a parameter pack
enable_if<sizeof...(Types)==2>

* One of the major uses of variadic templates is forwarding from one function to another
* std::tuple does not provide head() and tail() functions
* The standard library does not offer << or >> for tuple. Worse, writing a << for std::tuple is amazingly complicated 
  because there is no simple and general way of iterating through the elements of a standard-library tuple

Book advices: 
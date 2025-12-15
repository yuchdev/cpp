Questions:
read more about instantiation (ch.26)
extern templates: TODO
repeat templates
`remove_reference<T>::type&`
linearizing hierarchy example (27.4.2)

## New C++14 features
* Local classes as template arguments (25.2.1)
* Default template arguments for function templates (25.2.5.1)
* More explicit control of template instantiation: extern templates (26.2.2)

## C++ Templates

* To be used as a template argument, a type must be in scope and accessible (incl. local classes since C++11)
* A template parameter that is not a type or a template is called a value parameter and an argument passed to it a value argument
* An argument for a template value parameter can be (iso.14.3.2):
	* An integral constant expression (10.4)
	* A pointer or a reference to an object or a function with external linkage (15.2)
	* A nonoverloaded pointer to member (20.6)
	* A null pointer (7.2.2)
* Unfortunately (for no fundamental reason), literal types cannot be used as template value parameters
* The comparison criterion for a map is just an example. The technique used to pass it is general and very widely used
  to parameterize classes and functions with 'policies'
  Examples include actions for algorithms, allocators for containers, and deleters for unique_ptr 
* Just as for default function arguments, the default template arguments can be specified and supplied for trailing arguments only
* Default template arguments can also be useful for function templates (since C++11)
* If all function template arguments are defaulted, the `<>` can be left out (exactly as in function template specializations
* The `vector<void*>` is a complete specialization. That is, there is no template parameter to specify or deduce when we use the specialization
* A specialization with a pattern containing a template parameter is called a partial specialization in contrast to complete specializations
* We refer to the most general template as the primary template. The primary template defines the interface for all specializations
* All specializations of a template must be declared in the same namespace as the primary template
* One specialization is more specialized than another if every argument list that matches its specialization pattern also matches the other
* Specialization is also useful for template functions. However, we can overload functions, so we see less specialization. 
  Furthermore, C++ supports only complete specialization for functions, so we use overloading where we might have tried partial specialization
* As for classes, the `template<>` prefix says that this is a specialization that can be specified without a template parameter
```
template<>
bool less<>(const char* a, const char* b)
```

* How does a specialization differ from overloading? From a technical point of view, they differ because individual functions
   take part in overloading whereas only the primary template takes part in specialization
* There are a few uses of function specializations. For example, we can select among functions taking no arguments
```
template<> constexpr int max_value<int>() { return INT_MAX; } 
template<> constexpr char max_value<char>() { return CHAR_MAX; }
```

* From a template function and a set of template arguments, a function needs to be generated. 
  This process is commonly called template instantiation
* A template class is not instantiated unless its definition is actually needed
* A place where a template is used defines a point of instantiation
* An implementation instantiates a template function only if that function has been used.
  'Used' we mean 'called or had its address taken'
* instantiation of a class template does not imply the instantiation of all of its member functions
* it is provided two mechanisms to help the user take control under instantiation when needed:
	* optimize the compile-and-link process by eliminating redundant replicated instantiations
	* know exactly which point of instantiation is used to eliminate surprises from complicated name-binding contexts
```
template class vector<int>; // class
template int& vector<int>::operator[](int); // member function
template int convert<int,double>(double); // nonmember function
```
* To complement explicit instantiation requests, the language provides explicit requests not to
  instantiate (usually called extern templates)
```
// suppresses implicit instantiation
// explicitly instantiate elsewhere
extern template class MyVector<int>; 
```
* One use of this is to place the explicit instantiation in a shared library
* The general rule of avoiding global names as much as possible should be taken especially seriously in template code
* We try to make template definitions as self-contained as possible and to supply much of what would otherwise 
  have been global context in the form of template arguments (e.g., traits)
  Use concepts to document dependencies on template arguments
* Sometimes, nonlocal functions are the best choice. Typical examples of that are `sort()`, `swap()` and `less()`
  When something needs to be nonlocal, prefer a named namespace to the global scope. Doing so preserves some locality
* The general problem with template name binding is that three contexts are involved in a template instantiation:
	1. The context of the template definition
	2. The context of the argument type declaration
	3. The context of the use of the template
* Dependent names: names that depend on a template parameter. Such names are bound at a point of instantiation
* Nondependent names: names that don't depend on a template parameter. Such names are bound at the point of definition of the template
* By default, a dependent name is assumed to name something that is not a type. 
  So, to use a dependent name as a type, you have to say so, using the keyword `typename`
* We can avoid such awkward use of `typename` by introducing a type alias `using`
```
template<typename T>
using ValueType<T> = typename T::value_type;
```
* When the compiler sees a template definition, it determines which names are dependent
  If a name is dependent, looking for its declaration is postponed until instantiation time 
* For a function template, that point is in the nearest global or namespace scope enclosing its use, 
  just after the declaration that contains that use (after - to enable recursive calls)
* For a template class or a class member, the point of instantiation is just before the declaration containing its use
* The name-binding rules for templates were radically more restrictive than the rules for non-template code, 
  writing template code would be a completely separate skill from writing non-template code. 
  Templates and non-template code would no longer interoperate simply and freely
* The binding of dependent names is done (iso.14.6.4.2) by looking at
	1. the names in scope at the point where the template is defined, plus
	2. the names in the namespace of an argument of a dependent call (ADL)
* ADL can be 'too aggressive' when combined with unconstrained templates
`void copy(const Index&, Index&, int deep); // if Index is from std, it opens std:: and you get a conflict`

* Many such problems can be solved by the use of concepts
* Often, we know which namespace our function should come from, so we can be specific about that. For example:
```
user::copy(x,y,false);
using std::begin;
```
* while inherited from template-dependent classes, the `this->` or `Base::` qualification is required 



Book advices: 
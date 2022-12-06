Chapter 8 Nicolai Josuttis

Union templates. 
Members of templates: functions, nested classes, static members.
Virtual functions. Linking templates. 
Parameters for the templates: types, non-types, template.

*  Template arguments
  * Template arguments can be defined 
    * explicitly <int, int>
    * by the class name (type_t a; X<a>)
    * by default
    * using type deduction
  * Some function template arguments cannot be deducted (e.g. return value type)
  * When overloading template functions, it is not always possible to define a specific one even by fully substituting arguments (abiguity)
  * Unnamed classes and enumerations (except typedefs) cannot be arguments of types
  * Template arguments are equivalent to each other if they are identical pairwise at the compilation stage

* Templates and friends
  * A declaration of friend function could be the only object declaration (i.e. it is not visible anywhere except in the class)
  * A declaration of friend function could be its defenition
  * The declaration of friendly classes cannot be a definition
  * You can declare friend a specific instance of the template
  * The template friend must be visible at the declaration point, unlike the plain class
  * It is possible to define a friend template or a friend template instance
  * It is important to understand, you can not define a friend function in a template class, 
  otherwise it would be a violation of the ODR
* Non-type params
  * Non-type parameters can be:
    * Integer type, including enums
    * Pointers and references with external linking
  *  Can not be static and mutable, const and volatile parameters are ignored. Always rvalue. 
  * The keyword 'typename' can be used with a non-type pointer parameter (typename allocator<T>::alloc*)
* SFINAE
* Template params of templates
  * The template parameter is defined by the 'class' keyword
  * The argument inside the template parameters of template cannot be used in the template itself
  * If the name of the template parameter nowhere to be used, it can be omitted
  * A template parameter that has default values must repeat them in the parameter list
* Template members and members of templates
  * Templates are able to include plain and template methods, plain and template classes, 
  virtual non-template methods, and static members
  * Unlike classes, templates are not allowed to have the same name as other objects
  * Templates can't have extern "C" calling conventions, but can have non-standard extern "X" calling conventions
  * Templates have external linking, except declared as static

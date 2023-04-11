## TODOs
http://blog.httrack.com/blog/2014/05/09/a-basic-glance-at-the-virtual-table/
http://www.phpcompiler.org/articles/virtualinheritance.html

## New C++14 features
* Override controls: override and final (20.3.4)
* Inheriting constructors (20.3.5.1)

## Simple Inheritance

* No memory overhead is implied by deriving a class. The space required is just the space required by the members
* We can, but do not have to, repeat virtual in a derived class
* I don't recommend repeating virtual. If you want to be explicit, use override
* A virtual function invoked from a constructor or a destructor reflects that the object is partially constructed or partially destroyed
* override: The function is meant to override a virtual function in a base class (20.3.4.1)
* final: The function is not meant to be overridden (20.3.4.2)
* The override specifier comes last in a declaration, after all other parts
* An override specifier is not part of the type of a function and cannot be repeated in an out-of-class definition
* override is not a keyword; it is what is called a contextual keyword
* We might want to prevent our users from overriding virtual functions 
  because the only thing such overrides could do would be to change the semantics
* We can make every virtual member function of a class final; just add final after the class name
* adding final to the class not only prevents overriding, it also prevents further derivation from a class
* Use final where it clearly reflects a class hierarchy design that you consider proper
* A final specifier is not part of the type of a function and cannot be repeated in an out-of-class definition
* using-declarations can be used to add a function to a scope
* using with the constructor is exactly equivalent to its use for ordinary functions (14.4.5, 20.3.5)
* A return type of B& may be relaxed to D& (provided B is a public base of D)
  This is sometimes called the covariant return rule
* Because functions such as clone() are virtual and they (indirectly) construct objects, they are often called virtual constructors
* A compiler may reorder sections of a class with separate access specifiers (affects alignment)
* Such reordering could come as a surprise to the programmer and is implementation-dependent
* declaring data members protected is usually a design error
* A using-declaration (14.2.2, 20.3.5) cannot open private data
* A pointer to member can be obtained by applying the address-of operator, &, to a fully qualified class member name
* A pointer to a virtual member can therefore be passed between different address spaces as long as the same object layout is used in both
* Pointers to non-virtual member functions cannot be exchanged between address spaces
* We can safely assign a pointer to a member of a base class to a pointer to a member of a derived class, 
  but not the other way around. This property is often called contravariance
* Inheritance aims to provide one of two benefits:
	* Shared interfaces
	* Shared implementation
	A class can combine aspects of these two styles
* The constructor of every virtual base is invoked (implicitly or explicitly) from the constructor 
  for the complete object (the constructor for the most derived class)
* A dynamic_cast to void* can be used to determine the address of the beginning of an object of polymorphic type
* A dynamic_cast can cast from a polymorphic virtual base class to a derived class or a sibling class. 
  A static_cast does not examine the object it casts from, so it cannot
* From a design perspective, dynamic_cast can be seen as a mechanism for asking an object if it provides a given interface
* That is, typeid() returns a reference to a standard-library type called type_info defined in <typeinfo>
* The type_index is a standard-library type for comparing and hashing type_info objects (35.5.4)
* Use explicit run-time type information only when necessary

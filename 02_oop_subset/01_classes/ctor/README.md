Questions:
* check if ~suppresses copy&move
* declare non-const => delete const

### New C++14 features
* Language mapping of {}-list to std::initializer_list (3.2.1.3, 17.3.4)
* Delegating constructors (17.4.3)
* In-class member initializers (17.4.4)
* Control of defaults: default (17.6) and delete (17.6.4)
* initializer_list constructors for containers (3.2.1.3, 17.3.4, 31.3.2)
* Move semantics for containers (3.3.1, 17.5.2, 31.3.2)


### Class Construction

* New C++11 {} notation is recommended over the () notation for initialization because it is explicit about what is being done (initialization), avoids some potential mistakes, and can be used consistently
* For a set of related update functions, it is often useful to return a reference to the updated object so that the operations can be chained
```
d.add_day(1).add_month(1).add_year(1);
```

* A member class (often called a nested class) can refer to types and static members of its enclosing class
* There are five situations in which an object is copied or moved:
  * As the source of an assignment
  * As an object initializer
  * As a function argument
  * As a function return value
  * As an exception
* In all cases, the copy or move constructor will be applied (unless it can be optimized away)
* In addition constructors are used to initialize temporary objects and to implement explicit type conversion
* Resource management in C++ rely on constructor/destructor pairs
  So do other techniques relying on a pair of actions, such as do/undo, start/stop, before/after, etc.
* The name of a class cannot be used for an ordinary member function, data member, member type, etc, within the class
* Constructor tries to establish the invariant and if it cannot, it throws an exception
* A programmer who declares a destructor for a class must also decide if objects of that class can be copied or moved
* Constructors execute member and base constructors in declaration order (not the order of initializers)
* If two constructors used a different order, the destructor could not
* Memberwise initialization can be used if initializers are needed: `{a,b,c}`
* If declared for a `class X`, a destructor will be implicitly invoked whenever an X goes out of scope or is deleted
* This implies that we can prevent destruction of an X by declaring its destructor `=delete` (17.6.4) or private
* Of the two alternatives, using `private` is the more flexible
  For example, we can create a class for which objects can be explicitly destroyed, but not implicitly
* The default initialization of using `{}` is defined as initialization of each member by `{}`
* Naturally, memberwise initialization works only if we can access the members
* If a class has a private non-static data member, it needs a constructor to initialize it
* For this reason, `{}` initialization is sometimes referred to as universal initialization: the notation can be used everywhere
* No any logical reason to prefer the `(`) notation except in the rare case where you need to distinguish between initialization with a list of elements and a list of constructor arguments
* Constructors for built-in types are most often used for template arguments
* References and consts must be initialized
* Therefore, a class containing such members cannot be default constructed unless the programmer supplies in-class member initializers
* The initializer list can be of arbitrary length but must be homogeneous
  That is, all elements must be of the template argument type, T, or implicitly convertible to T
* For selecting a constructor, default and initializer lists take precedence
* Prefer the default constructor - is basically common sense: pick the simplest constructor when you can
* If you define an initializer-list constructor to do something with an empty list 
  that differs from what the default constructor does, you probably have a design error on your hands
* `initializer_list` doesn't provide subscripting
* `initializer_list<T>` is passed by value
* The elements of an initializer_list are immutable
* Because initializer_list elements are immutable, we cannot apply a move constructor
* A container can have some constructors explicit and some not. 
  For example, `std::vector<int>(int)` is explicit, but `std::vector<int>(initialize_list<int>)` is not
* Prefer to use the member initializer syntax to make it explicit that initialization is being done
* That is, a member-style initializer using the class�s own name (its constructor name) 
  calls another constructor as part of the construction. 
  Such a constructor is called a delegating constructor (and occasionally a forwarding constructor)
```
X() :X{42} { }
X(string s) :X{to<int>(s)} { }
```

* You cannot both delegate and explicitly initialize a member
```
X() :X{42}, a{56} { } // error
```

* An in-class member initializer can use names that are in scope at the point of their use in the member declaration
* Member initialization is done in declaration order
* For a few simple special cases, it is possible to initialize a static member in the class declaration.
  The static member must be a const of an integral or enumeration type, or a constexpr of a literal type,
  and the initializer must be a constant-expression
* For integers, enumerators offer an alternative for defining symbolic constants within a class declaration
* Typically, a move cannot throw, whereas a copy might (because it may need to acquire a resource)
* When you write a move operation, you should leave the source object in a valid but unspecified state
* A virtual base may appear as a base of several classes in a hierarchy. A default copy constructor will correctly copy it
* Meaning of copy:
  * Equivalence: After x=y, operations on x and y should give the same result
  * Independence: After x=y, operations on x should not implicitly change the state of y
* If you don't want slicing, you have two major tools to prevent it:
  * Prohibit copying of the base class: delete the copy operations
  * Prevent conversion of a pointer to a derived to a pointer to a base: make the base class a private or protected base
* The argument of a move operation must always be left in a state that the destructor can cope with
* In a few cases, such as for a return value, the language rules say that it can return `&&`
  (because the next action is defined to destroy the element)
  However, in general we have to tell it by giving an rvalue reference argument: std::move()
* What if we try to swap objects of a type that does not have a move constructor? We copy and pay the price
* By default, a class provides:
  * A default constructor: `X()`
  * A copy constructor: `X(const X&)`
  * A copy assignment: `X& operator=(const X&`)
  * A move constructor: `X(X&&)`
  * A move assignment: `X& operator=(X&&)`
  * A destructor: `~X()`
* If the programmer declares any constructor for a class, the default constructor is not generated for that class
* If the programmer declares a copy operation, a move operation, or a destructor for a class, 
  no copy operation, move operation, or destructor is generated for that class
* Unfortunately, the second rule is deprecated until C++17, and enforced starting from C++20, 
  so in C++17 copy constructors and copy assignments are still generated even if a destructor is defined.
* Using `=default` is always better than writing your own implementation of the default semantics
* Copy, move, and destructors are logically linked
* Declaring destructor suppresses the generation of copy and move operations
* If memberwise copy is appropriate, explicit `=default` and a comment are most likely a good idea
* We defined copy assignment, so we must also define the destructor
* Net effect of these `=defaults` is just to eliminate move operations:
```
Tic_tac_toe(const Tic_tac_toe&) = default;
Tic_tac_toe& operator=(const Tic_tac_toe& arg) = default;
~Tic_tac_toe() = default;
```
* It is common to want to prevent the copying of classes used as bases because such copying easily leads to slicing
* We can delete any function that we can declare
  * we can eliminate a specialization from the set of possible specializations of a function template
```
template<class T>
T* clone(T* p);
Foo* clone(Foo*) = delete; // don't try to clone a Foo
```
  * Another application is to eliminate an undesired conversion
```
Z(double); Z(int) = delete;
```
  * Control where a class can be allocated
```
void* operator new(size_t) = delete;
```

* Note the difference between a deleted function and one that simply has not been declared
  In the former case, the compiler notes that the programmer has tried to use the deleted function and gives an error
  In the latter case, the compiler looks for alternatives, such as not invoking a destructor or using a global operator new()
* It is logically possible to move a source into itself `(s=std::move(s))`, so we have to protect against self-assignment

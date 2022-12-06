#### Content
* Derived classes. Hierarchy of classes
* Implementation inheritance
* Virtual functions. Abstract Classes
* Designing class hierarchies


### Definition of Inheritance

* A hierarchy of classes can be a non-cyclic graph. The depth of the hierarchy is not limited
* Do not use fields storing type information unless you really need it (e.g. for object serialization in database)
* Liskov Substitution Principle idea is following: a function that uses a reference or a pointer to a base class 
  must also be able to work with descendant classes, without checking thir types explicitly
* Do not pass polymorphic objects by value to avoid slicing
* It is common practice to inherit classes from classes (i.e., structures usually are not involved in inheritance, however, perfectly can be)
* The constructor and destructor are never inherited, it is either created by default or implemented explicitly
* Assignment operators and friend functions are not inherited as well
* Destructors are called in an order reverse to construction
* The derived class fields may have same named as the base one, although it is not recommended.
  In this case, the base class member is replaced by the one from derived class.
* You can't initialize base class members in the derived constructor initialization list, 
  you can change them in constructor body at the best (if they are not hidden in `private` section)
* The methods of the base class can be called explicitly
* Public inheritance in most cases declares that is called "is-a" relationship between base and derived class.
  During the design stage, choose the base entity carefully (the classic antipattern here is classic "square-rectangle" inheritance)
* Private inheritance establish "has-a" relationship
* `private` or `protected` inheritance could be used to hide the interface of a base class (Adapter pattern)
* For implementing an Adapter, prefer composition. Use inheritance only when you have to
  (e.g. you have to provide access to protected members or ensure polymorphic behavior)
* In the case of closed inheritance, the casting pointer or reference to a base class does not work either
* You can use `using base::method` directive in order to override the parent method

* When calling virtual functions from constructors and destructors only static binding works
* Declaring data in protected section indicates a design error
* When a function is non-virtual, it is assumed that it will not be overridden
* It is normal to leave a virtual function in derived classes without implementation


### Runtime Polymorphism

* In polymorth classes you should have a virtual destructor
* In derived classes, virtual functions are implemented only if required
* Operators can be virtual
* In the constructor or destructor only static binding could be used (only own virtual methods of the calss are called)
* You can "virtualize" external functions (e.g. friends) by passing a reference to a polymorphic base class
* Upcasting is possible, but widely considered as a design error.
  Nethertheless, implementing upcasting constructor or assignment operator,
  don't forget to call explicitly constructor or operator of the parent class in functional form
* Default values in the virtual function parameter list have static binding
  (there's no such place in VTBL to save them for polymorphic use)
* The return value of a virtual function may have different type in different classes if tey are
  pointers or references, and can be casted to each other (so-called co-variant types)
* Don't make objects virtual unnecessarily, it's just a waste of resources without gains
* `inline` is ignored for virtual functions, but it is not an error, as virtual function must have an address
* Do not use arrays of polymorphic classes by value
* A low-level call of a virtual function looks like `(*class->vptr[i])(class)`
* Once polymorphic object is created, a pointer to VTBL, called a virtual table pointer or vpointer, 
  is added as a hidden member of that object
* Constructor inirialize VTBL as a first step, because the pointer to VTBL is just a hidden member which needs to be initialized
* Usually the compiler creates a separate vtable for each class
* The VTBLs are statically allocated somewhere in your address space
* VTBL format is defined by ABI and may vary between compilers
* E.g. in MSVC++, a vtable pointer stored at the beginning of an object's memory layout, before any member data.
* There are different approaches to its placement in object files
  * a copy in each object file
  * a heuristic to allow generation in the first use
* There also can be multiple VTBL pointers, if your class participate in multiple imheritance from other classes with vtables.
* Class RTTI is also stored in VTBL, usually as a first element
* And non-trivial stuff, with multiple inheritance, VTBL stores shifts from `this` to the beginning of the class where the function belongs
* The "rule of thumb" for choosing between template and polymorphism
  * Polymorphism - when the behavior is different
  * Template - when the stored type is different

### Abstract Classes

* Abstract classes are widely used in design patterns and COM technology
* Usecases of abstract classes:
  * interface declaration
  * contract definition
  * default implementation (although, in some cases it makes sense to separate it as well)
* An abstract class usually does not need a constructor
* You can declare and implement a virtual destructor in an abstract class. It will be normally called and won't be inherited.
  It will play the role of the default destructor, and will be called if derived class do not provide a destructor
* In an abstract class, you can also implement non-pure virtual methods, and call them for derived classes normally.
  However, from the design point it's a rather questionable solution.
* Consider placing constructor and destructor to protected section in abstract classes. 
  This gives to such classes some "abstractness" (you can't create an instance) even without pure virtual functions
* All cases of pure-virtual function implementation:
  * Pure virtual destructor
  * Implementation by default
  * Common partial implementation (protected)
  * Throw an exception if pure virtual call is possible

#### Design Considerations

* If the function is non-virtual, it is likely to be inherited always and for all
* If the function is purely virtual, the interface is inherited
* If a purely virtual class function is defined, the default implementation is inherited
* If the default implementation is not appropriate for everyone, you either need to inherit from proper implementation,
  or implement a non-virtual method called manually, like default(). 
  Then if the overridden method is not implemented, the derived class will not compile.
* Non-terminal classes should be abstract
* Abstract class should have protected operator=()
* Abstract class should not possess data, try to move it into a special storage-class

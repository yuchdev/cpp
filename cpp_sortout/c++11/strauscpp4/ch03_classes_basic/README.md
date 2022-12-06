Questions:

Proof of a statement in C++ Standard
"A move operation is not implicitly generated for a class where the user has explicitly declared a destructor"
Effective since C++20

New C++14 features:

* Language mapping of {}-list to std::initializer_list (3.2.1.3, 17.3.4)
* Rvalue references (enabling move semantics; 3.3.2, 7.7.2)
* Nested template arguments ending with >> (no space between the >s; 3.4.1)
* Lambdas(3.4.3,11.4)
* Variadic templates (3.4.4, 28.6)
* Type and template aliases (3.4.5, 6.5, 23.6)
* initializer_list constructors for containers (3.2.1.3, 17.3.4, 31.3.2)
* Move semantics for containers (3.3.1, 17.5.2, 31.3.2)

Advices:

* The basic idea of concrete classes is that they behave just like built-in types
* The classical user-defined arithmetic type is complex
* The std::initializer_list used to define the initializer-list constructor
* The std::initializer_list is passed by value
* It is possible to return a standard-library unique_ptr
* A move constructor does not take a const argument as is supposed to remove the value from its argument
* A move assignment is defined similarly
* virtual call mechanism can be made almost as efficient as the 'normal function call' (within 25%)
* In very much the same way as new and delete disappear from application code, we can make pointers disappear into resource handles
* The best thing to do is usually to delete the default copy and move operations
* If you need to copy an object explicitly, write some kind of clone() function
* The =delete mechanism is general, that is, it can be used to suppress any operation
* One particularly useful kind of template is the function object (sometimes called a functor)

Book advices:

* Express ideas directly in code
* Define classes to represent application concepts directly in code
* Use concrete classes to represent simple concepts and performance-critical components
* Avoid 'naked' new and delete operations
* Use resource handles and RAII to manage resources
* Use abstract classes as interfaces when complete separation of interface and implementation is needed
* Use class hierarchies to represent concepts with inherent hierarchical structure
* When designing a class hierarchy, distinguish between implementation inheritance and interface inheritance
* Control construction, copy, move, and destruction of objects
* Return containers by value (relying on move for efficiency)
* Provide strong resource safety; that is, never leak anything that you think of as a resource
* Use containers, defined as resource handle templates, to hold collections of values of the same type
* Use function templates to represent general algorithms
* Use function objects, including lambdas, to represent policies and actions
* Use type and template aliases to provide a uniform notation for types that may vary among similar types or among implementations


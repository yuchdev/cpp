Questions:

Proof of a statement in C++ Standard
"A move operation is not implicitly generated for a class where the user has explicitly declared a destructor"
Effective since C++20


Book advices:


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


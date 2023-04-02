### Classes

* Express ideas directly in code, not in documentation
* The basic idea of concrete classes is that they behave just like built-in types
* The classical user-defined arithmetic type is complex
* Avoid 'naked' new and delete operations
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
* Use function templates to represent general algorithms
* Use function objects, including lambdas, to represent policies and actions
* Use type and template aliases to provide a uniform notation for types that may vary among similar types or among implementations

### TODOs

Proof of a statement in C++ Standard
"A move operation is not implicitly generated for a class where the user has explicitly declared a destructor"
Effective since C++20

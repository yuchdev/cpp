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

### Class and structure

* In general, the choice of using a class or a struct depends on your preference and coding style
* Some people prefer to use a struct for simple data containers, while others prefer to use a class for everything 
* Use a class if you want to define a type that has both data and behavior, and you want to control access to the data.
* Use a struct if you want to define a simple data container with no behavior, and you don't care about access control.
* If you are working on a codebase that already has a convention for using classes or structs, it's a good idea to follow that convention for consistency.
* If you are working with a library or framework that requires you to use classes or structs in a certain way, follow those guidelines to ensure compatibility.

### TODOs

Proof of a statement in C++ Standard
"A move operation is not implicitly generated for a class where the user has explicitly declared a destructor"
Effective since C++20

### Content

Exception handling
Derived exceptions. Resource management. Constructors and destructors.
Standard exceptions.
ANSI C exception support
Exception Security
Safeguarding STL exceptions

### Standard Exceptions

Exception | Parent | Throws
---|---|---
| bad_alloc |  `std::exception` | new |
| bad_cast |  `std::exception` | dynamic_cast |
| bad_typeid |  `std::exception` | typeid |
| bad_ios_base::failure |  `std::exception` | ios_base::clear() |
| logic_error |  `std::exception` | logic errors |
| <- domain_error | `std::logic_error` | |
| <- invalid_argument | `std::logic_error` | bitset() |
| <- length_error | `std::logic_error` | |
| <- out_of_range | `std::logic_error` | at(), bitset<>::operator[] |
| runtime_error |  `std::exception`| Any runtime error |
| <- range_error |  `std::runtime_error` |  |
| <- overflow_error |  `std::runtime_error` | bitset::to_ulong() |
| underflow_error |  `std::runtime_error` |  |

### Exceptions Basics

* Use exceptions in `main()`, including `catch(...)`
* In theory, you can throw exception of any type (int, bool, enum)
* Don't use exceptions when C++ control structures are sufficient
* Use the RAII (Resource Assignment is Initialization) principle
* Exception is the only standard way to report an error from constructor
* Don't throw from destructor, or at least don't let exceptions to leave its scope
* In a large project, inherit from a common exception class
* Guarantee the release of all the resources in constructor in case of an exception
* Since C++11 we use noexcept for declaring that the function does not throw
* The library should not terminate the program execution or display any debugging information
* Define clear error handling strategy on the early design stages
* If the exception is thrown by reference and caught by value, slicing is possible
* `terminate()` is called when uncaught exception is thrown
* Standard `terminate()` handler calls `abort()`
* You set a new handler using `set_terminate()` function, e.g. call `exit(0)` instead of `abort()`,
  as it would call destructors of global objets
* Controlled constructor intercept all exceptions in the body and initialization list, and passes them on
* There's a temptation to say it's possible to release some resources in controlled `catch` section,
  but it's not true. We can't make any assumptions, on what stage of instruction exception occured,
  and what resources were already initialized
* Controlled constructor used only for diagnostic purposes
* If one exception throws another, the application will crash by terminate()
* members of exception class could be nothrow only
* Use standard exceptions, but understand the difference between those which STL throws 
  and those that user should throw
* Inherit all exception from `std::exception`, so that they were caught by the final handler `catch(std::exception&)`
* The available amount of memory is guaranteed to be sufficient to throw `bad_alloc`
* Exceptions allocate memory for themselves not in the heap
* If the reference by which exception is caught is non-constant, it is theoretically possible to modify it
  However it's almost always a bad idea.
* Rethrowing exception by `throw` guarantees type being unchanged
* `catch(const void*)` catches all exceptions thrown by pointer
* Exception handling slows down the program. Just using of `try-catch` slows it down by 5-10%.
  Handling slows it down by orders of magnitude. That's why exceptions should be used only for special situations
  and not be an element of regular program logic
* don't throw exception while checking error code is sufficient
* `uncaught_exception()` reports that some exception is thrown and not processed yet
* If inheriting from `std::exception` privately, it won't be possible to cast reference to the base class
  It menans inaccessible `what()`
* don't add too many nested `catch` blocks, it will reduce performance dramatically
* If you catch `bad_alloc`, you may try to release memory or at least shut down application correctly 
  rather than crash it (TODO: example with nodes)
* Some exceptions are just does not mean to be caught, they should not to inherit from `std::exception` (e.g. `boost::thread::tread_interrupted`)
* Note, by catching an exception in the constructor and trying to create the object again,
  you are actually trying to work with a completely different object
* An object's lifetime begins when its constructor is complete
  * Consequence: An object whose constructor has not completed never existed
  * Consequence: The only way to report a failed constructor 
    is to throw an exception from constructor
* In general, `catch(...)` handler is undesirable for the following reasons:
  * it is impossible to retreive a particular error message without re-throwing
  * depending on compilation options, it may catch SEH exceptions, which should not be caught
  * it may catch exceptions that shouldn't be caught by user code 
    e.g. `boost::tread_interrupted`, which is intentionally not a descendant of `std::exception`


### Exceptions Design Considerations

* When you start considering exceptions as your error handling policy, 
  you have to understand the importance of them. 
  You have to understand that your decisions will have a potentially large impact on those 
  who will use your code. 
  Your exceptions are part of your interface contract; you should fully understand 
  which exceptions your interface throws, when it throws, and why it throws.
* A good idea is to include the file name and code line number 
  in your exceptions description, using `__FILE__` and `__LINE__` macroses
* Exceptions put special requirements on C++ templates.
  In particular, we cannot be sure that a passed type `T` does not throw exceptions,
  especially during construction, copying, passing or returning by value
* There can be several general recommendations:
  * passing type `T` only by pointers or references
  * suppressing all exceptions in `catch(...)` section
  * catching all exceptions, and wrapping in your own exception type
  * using `shared_pth` `nothrow`
  * exception transparency in templates; rethrowing everything by `throw`
* The same applies to the PImpl idiom; in the general we don't know 
  what exceptions the implementation will throw


### Structured Exceptions

* The SEH mechanism is not portable from the Windows platform and does not call destructors while the stack unwinding
* `catch(...)` handles all SE unconditionally
* It is possible to translate SE into C++ exception by setting `_set_se_translator()`
  The program must be compiled with `/EHa`

### Floating Point Exceptions

* [A good article on IEEE FP format and exceptions](http://www.gamasutra.com/view/news/169203/Exceptional_floating_point.php)

### C pseudo-exceptions

* ANSI C offers a limited support for pseudo-exceptions
  * local jump `goto`
  * non-local jumps `setjmp`, `longjmp`
  * The C library also provides a unified (although quite primitive)
    error handling package, `raise()` and `signal()` functions from `signal.h`
    Handlers can interact with the outer world through objects, e.g. `sig_atomic_t`
    You are free to choose to call the `abort()`, `exit()`, or `longjmp()` function 
    from the exception handler, thereby interpreting the signal 
    as a terminating exception
  * If you prefer the polling method, the standard library provides
    such a tool through declarations in the `errno.h` header file. 
    This header defines `errno` global flag plus a few values that are the most popular. 
    The standard defines three such values: `EDOM`, `ERANGE`, and `EILSEQ`
    for domain, range, and multibyte stream errors respectively

### Exception internals

* For each exception handler the compiler generates a unique named descriptor inside the
  `.data` segment. Each descriptor defines a mangled type name corresponding to the type
  type of the handler exception
* The compiler also generates references to these type descriptors in `xdata$x` segment. 
  For each type, multiple key-value pairs are created, where key is type, 
  and value is the address of a handler. The resulting set of pairs forms a dispatch table 
  used by the application to redirect exceptions.
* In general, the compiler generates one table preamble for a function that contains a try block,
  plus one table entry for each handler in that try block.
  Fortunately, the type descriptors are shared between all the dispatch tables in the program,
  e.g., all of the handlers catch(long) in a program refer to the same descriptor
* The moral of a story: to reduce the overhead of exception handling on the size of your code and performance,
  you should minimize the number of functions include exception handling, 
  the number of `catch` handlers in those functions, and the number of types these handlers process.

### Exception safety

* Exception safety is a term that is often used in the C++ standard, however, it is not clearly defined
* Among C++ community, different authors tend to use the term for different things

* If an object throw or catch an exception, and still executes its public semantic guarantees, 
  then such an object offers _interface security_
  Depending on the strength of semantic guarantees, 
  this object may not allow an exception to leave the object's scope 
* If the exception does not result in any resource leak or undefined behavior, 
  then such object offers the _security of the implementation_
  Implementation security is generally the easiest to enforce.
  Fortunately, if you make your implementation secure, 
  you often also secure the the interface as well
* Interface security is like the correctness of the `const` modifier: 
  a well-designed class  must contain it from the very beginning. 
  Interface security cannot be implemented afterwards
* When a constructor throws an exception, it kills its object without calling its destructor. 
  Such behavior in this way put in danger implementation security.
  If the the constructor that throws the exception is allocating resources, 
  you cannot rely on destructor anymore to release them
  (in general, RAII is a solution of this problem)
* Some offer to make constructor `nothrow()`, 
  and move all the complex initialization into `init()` protected method
* Arguments against `init()`:
  * Using a trivial constructor + `init()` can cause a number of maintenance problems:
    forgetting to call `init()`, forgetting that `init()` throws, forgetting to check `init()`
  * The default trivial state makes the invariant complicated, 
    because you constantly need to check for it
  * Often calling `if` and loops operators you forbid many useful compilation optimization
  * Safe state should be a natural class invariant, not an implementation artifact.


### Levels of Exception Safety

* The standard C++ library provides both exception safety guarantees and requirements
  for container elements, compatible containers, iterators, allocators
* The operation considered exception-safe, when it leaves an object in a consistent state,
  or destroys it completely and correctly; in other words, enforces the invariant
* Types of STL Exception Safety Levels:
  * basic: class invariants is ensured, and object remains in a consistent state
  * strong: operation either successful, or complete rollback to previous state performed
  * `noexcept`: exceptions are not being thrown
* When you design your own library, the minimum level you should provide is (1),
  provide (2) and (3) where possible
* (1) and (2) highly depend on the implementation of the container elements
  (they must also be consistent)
* All containers transparently throw use exceptions from elements,
  so that the user instantly became aware of the problem
* Opposite approach is appropriate when implementing a high-level module:
  all exceptions must be c internally or wrapped in the module's standard exceptions
* Designing reliable library functions for mass user base, 
  sometimes it is necessary to analyze each line of code
  for the possibility of exceptions being thrown and their consequences,
  and describe the provided guarantee in the API documentation
* However, don't overengineer it, often such attempts to implement 
  an absolute safety complicate the invariant
* Almost all STL containers offer a strong exception guarantee, with the exception of:
  * insertion in the middle of vector
  * insertion of several elements in a map
  * string operations
  * all algorithms except uninitialized* family
* Requirements for STL container elements to ensure the basic or strong guarantee:
  * operators <> == != swap do not leave the element in a non-consistent state
  * swap must not generate exceptions
  * copying and assigning iterators must not throw exceptions
  * uninitialized* algorithms family require iterator's `operator++` and `--` to be noexcept
  * predicates and lambdas must not throw exceptions
  * `char_traits` must not throw exceptions
  * locales and facets do not throw
  * algorithms do not throw
  * the C library does not throw except for `qsort()` and `bsearch()` 
    as they accept pointers to functions which potentially may throw
  * C++ sorting algorithms may throw as well
  * containers of pointers to dynamically allocated memory are unsafe
  * predicates for associative containers must be copied
  * the constructor should set an invariant
* How to implementing exception-safe container element:
  * do not destroy the old representation until the new one is created
  * use RAII principle where possible
  * check object consistency before throwing an exception, except for constructors

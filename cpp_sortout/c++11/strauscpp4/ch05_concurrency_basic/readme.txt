Questions:
more real example std::async

New C++14 features:

* Resource management pointers: unique_ptr, shared_ptr, and weak_ptr (5.2.1)
* Concurrency support: thread (5.3.1), mutexes (5.3.4), locks (5.3.4), and condition variables (5.3.4.1)
* Higher-level concurrency support: packaged_thread, future, promise, and async()(5.3.5)
* tuples (5.4.3)
* Regular expressions: regex (5.5)
* Random numbers

Advices:

* unique_ptr is moved, shared_ptr is copied
* We do not need to use a pointer to return a collection of objects, use containers instead
* Concurrency support facilities: threads, mutexes, lock() operations, packaged_tasks, and futures
* The initialization with {f,some_vec} uses a thread variadic template constructor
* The unique_lock's constructor acquires the mutex
* The standard library offers help in the form of an operation for acquiring several locks simultaneously
* Locking and unlocking are relatively expensive operations
* The simplest 'event' is simply time passing
* The standard library provides a few facilities to allow programmers to operate at the conceptual level of tasks 
  (work to potentially be done concurrently) rather than directly at the lower level of threads and locks:
	* future and promise for returning a value from a task spawned on a separate thread
	* packaged_task to help launch tasks and connect up the mechanisms for returning a result
	* async() for launching of a task in a manner very similar to calling a function
* When a task wants to pass a value to another, it puts the value into a promise
* The main purpose of a promise is to provide simple 'put' operation
* A packaged_task provides wrapper code to put the return value or exception from the task into a promise
* The move() operations are needed because a packaged_task cannot be copied
* Treat a async() as a function that may happen to run concurrently with other tasks
* Basically, async() separates the 'call part' of a function call from the 'get the result part'
* Using async(), you don't have to think about threads and locks
* It is usually a good idea to convert a duration into a known unit
* A type function is a function that is evaluated at compile-time given a type as its argument or returning a type
* The standard library provides a mechanism, iterator_traits that allows us to check which kind of iterator is supported
* iterator_traits constructs a 'tag' value indicating the kind of iterator provided
* A standard-library type predicate is a simple type function that answers a fundamental question about types
  Examples are is_class, is_pod, is_literal_type, has_virtual_destructor, and is_base_of
* To express a regexp pattern, use a raw string literal
* A random number generator consists of two parts:
    * an engine that produces a sequence of random or pseudo-random values.
    * a distribution that maps those values into a mathematical distribution in a range

Book advices:
 
* Use resource handles to manage resources (RAII)
* Use unique_ptr to refer to objects of polymorphic type
* Use shared_ptr to refer to shared objects
* Use type-safe mechanisms for concurrency
* Minimize the use of shared data
* Don’t choose shared data for communication because of 'efficiency' without thought and preferably not without measurement
* Think in terms of concurrent tasks, rather than threads
* A library doesn't have to be large or complicated to be useful
* Time your programs before making claims about efficiency
* You can write code to explicitly depend on properties of types
* Use regular expressions for simple pattern matching
* Don't try to do serious numeric computation using only the language, use libraries
* Properties of numeric types are accessible through numeric_limits


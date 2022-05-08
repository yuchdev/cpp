Questions:
* exception-safe modern vector impl
* is exception copied or moved

New C++14 features:
* Preventing exception propagation: the noexcept specifier (13.5.1.1) Section 44.2.1 Language Features 1269
* Testing for the possibility of a throw in an expression: the noexcept operator (13.5.1.1)

Advices:
* Alternatives to exceptions
	* terminate program
	* error code
	* errno
	* error-handler function
	* return pair error-value
* An exception terminates a program only if a programmer allows it to terminate
* A library that asks the (possibly absent) user/operator for help is unacceptable
* Asynchronous events require mechanisms fundamentally different from exceptions
* Many systems offer mechanisms, such as signals, to deal with asynchrony
* practical and historical reasons cannot use exceptions:
	* A time-critical component of an embedded system (use init(), errno, pair value-code etc.)
	To mimic RAII, give every class with a constructor an invalid() operation that returns some error_code
	* A large old program in which resource management is an ad hoc mess (error codes, plain new etc)
* The strategy must be simple (relative to the complexity of the total program) and explicit
* Occasionally, it is necessary to convert from one style of error reporting to another
	* on error code throw exception
	* catch exception and return error code
* Exception handling can be implemented so that there is no run-time overhead when no exception is thrown
* The noexcept specifier (13.5.1.1) can be most helpful in improving generated code
* Most C functions can be declared noexcept
* only a few standard C library functions (such as atexit() and qsort()) can throw
* Before a throw, a function must place all constructed objects in valid states
* The library provides one of the following guarantees for every library operation:
	* The basic guarantee for all operations: 
	The basic invariants of all objects are maintained, and no resources, such as memory, are leaked. 
	In particular, the basic invariants of every built-in and standard-library type guarantee that 
	you can destroy an object or assign to it after every standard-library operation (iso.17.6.3.1)
	* The strong guarantee for key operations: in addition to providing the basic guarantee, 
	either the operation succeeds, or it has no effect. This guarantee is provided for key operations
	such as push_back(), single-element insert() on a list, and uninitialized_copy()
	* The nothrow guarantee for some operations: in addition to providing the basic guarantee, 
	some operations are guaranteed not to throw an exception. 
	This guarantee is provided for a few simple operations, such as swap() of two containers and pop_back()
* Both the basic guarantee and the strong guarantee are provided on the condition that
	* user-supplied operations (such as assignments and swap() functions) do not leave container elements in invalid states
	* user-supplied operations do not leak resources, and
	* destructors do not throw exceptions (iso.17.6.5.12)
* 'finally' could be implemented as a generic RAII functor
* Systematically checking preconditions can lead to repeated tests of logically unnecessary conditions
* The second approach tends to be used in systems where recovery from a precondition failure is considered infeasible
* The standard offers two assert mechanisms: cassert and static_assert
* The __FILE__ and __LINE__ are macros that expand at their point of appearance in the source code
* We can throw an exception of any type that can be copied or moved
* This temporary may be further copied several times before it is caught
* The process of passing the exception up the stack from the point of throw to a handler is called 'stack unwinding'
* Because an exception is potentially copied several times we don't usually put huge amounts of data in it
* Exceptions containing a few words are very common
* noexcept condition is not completely checked by the compiler and linker
* It is possible to declare a function to be conditionally noexcept. For example:
template<typename T>
void my_fct(T& x) noexcept(Is_pod<T>());
* The noexcept() operator takes an expression as its argument and returns true 
  if the compiler 'knows' that it cannot throw and false otherwise. For example:
template<typename T>
void call_f(vector<T>& v) noexcept(noexcept(f(v[0]))
* Conditional noexcept specifications and the noexcept() operator are common in standard-library operations
template<class T, size_t N>
void swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(swap(*a, *b)));
* The handler throw E/catch H is invoked:
[1] If H is the same type as E
[2] If H is an unambiguous public base of E
[3] If H and E are pointer types and [1] or [2] holds for the types to which they refer 
[4] If H is a reference and [1] or [2] holds for the type to which H refers
* The body of a function can be a try-block
* We cannot 'repair' the object and return normally as if the exception had not happened:
  An exception from a member constructor means that the member may not be in a valid state
* The specific rules for calling terminate() are (iso.15.5.1)
	* When no suitable handler was found for a thrown exception
	* When a noexcept function tries to exit with a throw
	* When a destructor invoked during stack unwinding tries to exit with a throw
	* When code invoked to propagate an exception tries to exit with a throw
	* When someone tries to rethrow (throw;) when there is no current exception being handled
	* When a destructor for a statically allocated or thread-local object tries to exit with a throw
	* When an initializer for a statically allocated or thread-local object tries to exit with a throw
	* When a function invoked as an atexit() function tries to exit with a throw
* The user can provide a terminate handler function by a call std::set_terminate()
* For example, a terminate handler could be used to abort a process or maybe to reinitialize a system
* Note that abort() indicates abnormal exit from the program
* The function exit() can be used to exit a program with a return value
* If an exception is not caught on a thread, std::terminate() (13.5.2.5) is called
* We can transfer an exception thrown on one thread to a handler on another thread the function current_exception()
try {
	// ... do the work ...
}
catch(...) {
	prom.set_exception(current_exception()); 
}
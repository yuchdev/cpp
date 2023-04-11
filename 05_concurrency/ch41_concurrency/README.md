## New C++14 features
* A syntax for attributes and two standard attributes: [[carries_dependency]] (41.3) and [[noreturn]]
* Memory model
* Low-level concurrency support: atomics

## Concurrency

* Prefer a `future` over a `mutex` for exchanging information, and a `mutex` over an `atomic` for anything but simple counters and the like
* Please also note that as long as you don't pass pointers to your local data to other threads, your local data is free of the sharing problems
* In contrast to the C-style POSIX facilities and to many older C++ thread-support libraries, the standard-library thread support is type-safe. There is no longer any reason to mess around with macros or `void*` to pass information among threads
* The C++ concurrency mechanisms are mostly supplied as standard-library components
* These components rely on a set of language guarantees known as the memory model. A memory model is the result of discussions between machine architects and compiler writers about how best to represent computer hardware. The memory model, as specified in the ISO C++ standard, represents a contract between the implementers and the programmers to ensure that most programmers do not have to think about the details of modern computer hardware
* Operations on an object in memory are never directly performed on the object in memory. Instead, the object is loaded into a processor register, modified there, and then written back. Worse still, an object is typically first loaded from the main memory into a cache memory and from there to a register
* The C++ memory model guarantees that two threads of execution can update and access separate memory locations without interfering with each other
* Why must programmers suffer all this complexity abour memory ordering? An alternative would be to provide only a simple, sequentially consistent model with minimal (or no) opportunities for data races. I can offer two reasons:
  * That is not the way the world is. The complexities of machine architectures are real, and a systems programming language, such as C++, must provide the tools for programmers to live with them
  * The C++ standards committee seriously considered that. They would have liked to provid a memory model that was an improved version of what Java and C# provide
* A synchronization operation is something that determines when a thread sees the effects of another thread; it determines what is considered to have happened before something else
* A synchronization operation on one or more memory locations is a consume operation, an acquire operation, a release operation, or both an acquire and release operation
  * For an acquire operation, other processors will see its effect before any subsequent operation's effect
  * For a release operation, other processors will see every preceding operation's effect before the effect of the operation itself
  * A consume operation is a weaker form of an acquire operation. For a consume operation, other processors will see its effect before any subsequent operation's effect, except that effects that do not depend on the consume operation's value may happen before the consume operation
* The enumerations represent:
  * `memory_order_relaxed`: No operation orders memory
  * `memory_order_release`, `memory_order_acq_rel`, and `memory_order_seq_cst`: A store operation performs a release operation on the affected memory location
  * `memory_order_consume`: A load operation performs a consume operation on the affected memory location
  * `memory_order_acquire`, `memory_order_acq_rel`, and `memory_order_seq_cst`: a load operation performs an acquire operation on the affected memory location
* If two threads really do not directly share data, some machine architectures deliver significant performance improvements through the use of a relaxed memory model at the cost of complexity in the implementation of message-passing primitives
* There are no copy or move operations for atomics
* The initialization of an `atomic` variable is not an atomic operation, so an initialization may have a data race with an access from another thread
* The difference between compare_exchange_strong() and compare_exchange_weak() is that the weak version can fail for 'spurious reasons'. That is, some peculiarity of the hardware or the implementation of `x.compare_exchange_weak(rt,t)` may cause a failure even if `x.val==rt`
* Consider the popular double-checked locking idiom. The basic idea is that if initializing some x must be done under a lock, you may not want to incur the cost of acquiring that lock every time you access x to see if the initialization has been done. The double-checked locking idiom is represented in the standard library by `once_flag` and `call_once()`
* To allow the C standard library to be compatible, the `atomic` member function types have freestanding equivalents
* A `fence`, also known as a memory barrier, is an operation that restricts operation reordering according to some specified memory ordering

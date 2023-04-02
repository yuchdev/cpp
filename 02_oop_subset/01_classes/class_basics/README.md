### Classes

Advices:

* Do not add many c-tors to a class, just because you think they could be useful for someone
* Start declaring class members from public, to make emphasis on interface
* Elements of member array could be constructed only by default. This effectively makes constant array members useless
* Constructors for static objects are being called during first access, destructors in an order opposite to creation
* Member of union can't have c-tors or destructors, as it is unclear which one to call
* Default c-tor may have arguments with default values
* If one constructor is defined, the default constructor is not created
* The default assignment operator does not work with references or constant members (TODO: example)
* forward declaration decrease compilation time
* Implement clone() as "virtual copy c-tor"
* use "lazy evaluation" of not all result dataset could be used
* use "pre-evaluation" is results could be used too often
* use ref-count and copy-on-write on big datasets in concurrent environment
* all function prototype params and return values could be declared via "forward declaration". Even passed by value!
* friend declarations of non-existent classes are ignored
* Forward declarations could be used also for function prototypes params (including by value) and returning values (including by value)
* inline is not a directive, but rather advice to a compiler. It could be canceled by cycles, recursion, de-referencing
* If copy, move and assignment are OK by default, use =default
* You can disable copying and assignment if necessary
* Deep copying methods are the copy constructor, assignment operator, and a virtual destructor
* Declare constructors with a single argument as explicit if you want to forbid implicit conversions (e.g. if that argument is size)
* The object is implicitly copied when passing or returning by value, and while throwing an exception
* Note that the copy constructor parameter cannot be passed by value, otherwise it will result in infinite recursion
* Uniform initialization is more efficient than assignment in the constructor body
* In some cases, you can put complex initialization in the private `init()` function
* In order to avoid warning on some compilers, the order of initialization in the constructor must be the same with the order of declaration

* Use "lazy evaluation" for classes which results are not always needed, or not all of them needed
* For classes which results are needed often, use "greedy evaluation"

Consistency means that the values of all variables of an object completely match to its state
and do not conflict with each other
For example, a string object has two variables: a pointer to memory block, and an integer string size.
Assigning a longer string to an object, we can try the following sequence (incorrect, in fact)
* free an old memory block
* allocate a new one
* copy the string
* change the pointer value and the length value

Before performing the first of these operations, the object data is consistent, after the last one is done, too.
Between the first and last operations, consistency is broken.

* Rule 1: Before returning from any public method, object data must be consistent.
* Rule 2: Ensure object data is consistent before making an outgoing call.
* Rule 3: Consider that during the outgoing call the object's state can be changed.

TODO: class invariant

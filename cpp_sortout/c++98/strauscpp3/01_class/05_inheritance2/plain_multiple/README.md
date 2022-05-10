### Content

Hierarchies of classes. Multiple inheritance
Ambiguity resolution. Using-directive
Virtual inheritance. Virtual base classes
Access control. Protected members. Access to base classes
Polymorphism, dynamic casting, type information and RTTI
Virtual Constructor pattern
Multimethods. Visitor Pattern
Pointers to members
Free memory

### Multiple Inheritance Defenition

* Multiple inheritance could be used to separate implementation from interface, with some tricky approaches
* If a class defines `operator delete`, it is necessary to define a virtual destructor
* Ambiguity in multiple inheritance could be resolved by explicit override or using-directive (`using Base::method`)
* Ambiguity during the function call can be resolved with the explicit call using class name (`Base::method()`)
* Declaring data in protected section indicates a design error
* Constructors are called in the order declared during inheritance
* Multimethods are not part of C++ langiage
  > "I rejected multi-methods with regret, because I liked the idea, but couldn't find an acceptable form under which to accept it"
  Bjarne Stroustrup, The Design and Evolution of C++
* However, multimethods could be implemented with Visitor or Double-Dispatching pattern
* Changing the access modifier should not change the logic of the program,
  therefore, even private members participate in ambiguities resolution

### Virtual Inheritance
* Multiple inheritance is well managed in case of an abstract base class
* In multiple inheritance the object contains several VPTR 
* In virtual inheritance, the constructor of the base class is called by the last derived class in the hierarchy
* The dominance principle is as follows:
  The virtual `print()` method is inherited from all three base classes
  but it will be executed only if it is explicitly overridden in the most "derived" class of the available
* For upcasting or crosscasting `dynamic_cast<>` requires polymorphism
* If conversion is forbidden, `dynamic_cast<Ptr*>` returns nullptr
* If conversion is forbidden, `dynamic_cast<Ref&>` throws `bad_cast`

### RTTI and `typeid`


### Pointer to Class Member

* Member pointer could be treated os an OFFSET from the class beginning (or its polymorph part)
* pointers to method is not a regular pointers
* Pointer template partial spec does not recogrize them
* Pointer to a non-static function-class member is a special case
  It cannot be assigned to a regular pointer or to a function pointer.
* Usually it is inpmeneted as a struct, because, besides the function pointer itself, 
  it can contain offset of `this` pointer (especially in case of multiple inheritance)
* In addition, if the function is virtual, instead of the address it stores nullptr, 
  but stores the index of the virtual function in VTBL instead
* Member pointer could store an offset from the class beginning (or its polymorph part)
* Thus, a structure is used to store a member pointer (not defined by the standard, but used in practice)
  * function address or NULL if method is virtual
  * offset of the `this` pointer
  * index, if the function is virtual
* A pointer to a member can be obtained by applying the dereferencing (&) operation 
  to a fully qualified class name (e.g. `char* Window::* pwCaption = &Window::szWinCaption`)

#### Links 

* http://blogs.msdn.com/b/oldnewthing/archive/2004/02/09/70002.aspx
* http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible

#### Notable Implementations

##### Borland and Watcom

struct BorlandMFP { 
   CODEPTR m_func_address;
   int delta;
   int vindex; // or 0 if no virtual inheritance
};
if (vindex==0) adjustedthis = this + delta; 
else adjustedthis = *(this + vindex -1) + delta
CALL funcadr

##### GNU g++ 

g++ uses a tricky space optimisation, also adopted by IBM's VisualAge and XLC.
struct GnuMFP {
   union {
     CODEPTR funcadr; // always even
     int vtable_index_2; //  = vindex*2+1, always odd
   };
   int delta;
};
adjustedthis = this + delta
if (funcadr & 1) CALL (* ( *delta + (vindex+1)/2) + 4)
else CALL funcadr

##### Microsoft and Intel

Microsoft and Intel use this for the 'Unknown' case.
Microsoft also use it when the /vmg option is used

struct MicrosoftUnknownMFP{
   FunctionPointer m_func_address; // 64 bits for Itanium.
   int m_delta;
   int m_vtordisp;
   int m_vtable_index; // or 0 if no virtual inheritance
};
if (vindex=0) adjustedthis = this + delta
else adjustedthis = this + delta + vtordisp + *(*(this + vtordisp) + vindex)
CALL funcadr

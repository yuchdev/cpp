### Constaness of classes and members
* Method which does not change anything in class state must be declared const
* A non-constant method cannot be called on a constant instance.
* Unnamed enum is the analog of static const member
* Compiler does not generate operator=(const A&) in non-const version explicitly declared
* static members COULD be changed in const methods
* Correctly define a constant pointer - 'char *const p' and a pointer to a constant 'char const* p' (or 'const char* p')
* If the class contains uninitialized const members of references, default c-tor can't be called

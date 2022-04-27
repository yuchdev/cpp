#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

// Pro-argument for data accessors and mutators
// * distinctive read-write access
// * support of functional abstractness - class member could be replaced with calculation, or even moved outside the class
// * value evaluation, checking the consistency
// * logging
// * if we see open class member in method, we expect it to be this class member


// It is allowed to declare structure and non-structure with the same name
// (same for class, enum, union)
void struct_one_name()
{
    struct A{
        int a;
    };

    A aa;
    aa.a = 2;
    int A = 1;
}

class A
{
public:

    A(char *const cp1) : cp1(cp1) {}

    // Static members COULD be changed in const members, as it does not change class state
    void set_static(int i) const{
        a_stat = i;
    }

    // TODO: const and non-const methods with same name
    // TODO: overload rules
    // TODO: function signature

    // Static method coule be called with the class instance and with the class name
    // construct::out_static_array() and c.construct::out_static_array();

    // You can't set array size with the other class member, even with const static
    // int arr1[sz];

private:
    // friend declarations of non-existent classes are ignored
    friend class do_not_exists;
    
    static int a6;

    // unnamed enum could be considered const class member
    // could be seen in pre-C++11 code, before in-place initialization was introduced
    enum{ c1 = 5, c2 = 7, c3 = 12 };

    // mutable class member could always be changed 
    // even being part of const or static object 
    // Design may require this, if we deal with so-called "conceptual constness" 
    // For example, cache in constant object could be mutable

    mutable int a8;
    // Opposite situation is also possible
    // when class contains pointer, which is being changed inside the const method
    // such method is const formally, but not conceptually

    // constant pointer or pointer to constant?
    // to determine it quickly, you can use "read backward" trick
    char const* cp2; // pointer (*) to 'const' 'char'
    const char* cp3; // pointer (*) to 'char' 'const'
    char *const cp1; // 'const' pointer (*) to 'char'
    static int a_stat;
};

#pragma clang diagnostic pop

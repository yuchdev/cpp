#include <iostream>

construct g_construct;

// It is allowed to define a structure and non-structure with the same name
// (same for class, enum, union)
void struct_one_name() 
{
    struct A {
        int a;
    };

    A aa;
    aa.a = 2;
    int A = 1;
}

void f1(A a) {}
void f2(A& a) {}

// This function demonstrates initialization and operation
// with constant objects and mutable members
void show_const() 
{
    A a;

    bool b = false;
    size_t s = sizeof(b); // 1

    const A aa;

    f1(aa); // non-const copy
    // f2(aa); compile error

    //aa.set_int1(5); - This is not allowed, calling a non-constant method in a constant class

    // But this is allowed, a mutable member of a constant class can be modified
    aa.set_a6_mutable(5);

    // static class member could be changed in const method
    aa.set_static(6);
}

// This function demonstrates initialization and operation
// with constant pointers and pointers to constants
void show_const_pointers() {
    // Pointer to constant
    const int* pp1 = new const int(3);
#if 0
    // Explicit error - constant not initialized,
    // but the compiler remains silent
    // Clang & GCC detect it!
    const int* pp2 = new const int;

    // It is no longer possible to initialize it legally
    // Not even const_cast will work
    //const_cast<int>(*pp2) = 1;
    // It is possible to create an uninitialized constant dynamic array
    // It won't even be zeroed, and nothing more can be done with it.
    const int* pp3 = new const int[100];
#endif

    // It is more logical to create a dynamic array through a constant pointer
    int* const pp4 = new int[100];

    // Beware of applying typedef to arrays
    typedef good_weekday_t week_array[7];
    good_weekday_t* s = new week_array;
    delete[] s;

    //X* x = new Y[2];
    // delete[] x;
    // It invokes undefined behavior.
    // This is correct.
    // When deleting an array, the dynamic and the static type of the object must be the same,
    // or the behavior is undefined (C++ Standard 5.3.5/3).
}

// Ways to call the constructor of an object:
// 1. Named automatic object (by default, copy).
// 2. Object in free memory (new/delete).
// 3. Non-static member object of a class.
// 4. Object - element of an array.
// 5. Local static object.
// 6. Global object.
// 7. Temporary object.
// 8. Object constructed with a user-defined allocator.
// 9. Union member
void show_construct_destruct() 
{
    cout << endl << "Create automatic objects" << endl;
    // 1. Named automatic object (by default, copy).
    // If the object has constants or references, it cannot be created
    // with the default constructor
    {
        construct c;
        construct d;
        {
            construct c1(c);
        }
        noncopyble nc;
        // But this object cannot be copied!
        // noncopyble nc1(nc);
    }
    // Created in the order c-d-c1-nc, destroyed c1-nc-d-c

    // 2. Object in free memory (new/delete).
    cout << endl << "Create dynamic objects" << endl;

    // If the class only has a compiler-generated constructor,
    // this form of invocation, with (), will zero the memory...
    construct* pc1 = new construct();
    // ... while this one won't
    construct* pc2 = new construct;

    // Always nullify unused pointers!
    delete pc1;
    pc1 = 0;
    delete pc2;
    pc2 = 0;

    // 3. Non-static member object of a class.
    cout << endl << "Create non-static member" << endl;
    // The default constructor intern() will be called
    {
        construct c1;
    }

    // 4. Object - element of an array.
    cout << endl << "Create objects array " << endl;
    // Elements of the array can be default constructed
    construct arr_c[5];
    // Initialization list can be used as constructors
    construct arr_1[] = { construct(2), construct(g_construct) };
    // Even implicit conversions can be used in construction
    construct arr_2[] = { 1, 2 };

    // 5. Local static object.
    cout << endl << "Create local static objects " << endl;
    create_static(0);
    create_static(1);
    create_static(2);

    // For simple objects, the function representation can be used
    int st = return_static();

    // 6. Global object.
    // The constructors of global objects are called in one module - in the order of definition
    // in different modules - the order is undefined. The same applies to object destruction (see g_construct, g_intern)
    // It is useful to use singletons as well as initialization wrappers (see construct.h)

    // 7. Temporary object.
    // It gets destroyed immediately
    cout << endl << "Create temporary object " << endl;
    construct(5);
    // Implicit copying also occurs when passing and returning by value
    // and when throwing exceptions

    // 8. Object constructed with a user-defined allocator.
    {
        // see show_user_alloc()
    }

    // 9. Union member
    // A union cannot have members with constructors/destructors
    // because it is unclear which one to call
}

void show_placement_delete() 
{
    struct X {
        X() {
            throw std::runtime_error("X object never be created");
        }

        // Custom placement new
        static void* operator new(std::size_t sz, bool b) {
            std::cout << "custom placement new called, b = " << b << '\n';
            return ::operator new(sz);
        }

        // Custom placement delete
        // Can only be called in functional form
        // However, pair delete can be found in case of exception
        static void operator delete(void* ptr, bool b) {
            std::cout << "custom placement delete called, b = " << b << '\n';
            ::operator delete(ptr);
        }
    };

    try {
        X* p1 = new (true) X;
    } catch (const std::exception& e) {
        cout << e.what();
    }
}

void test_placement_new_handler() 
{
    printf("No more memory!\n");
    throw bad_alloc();
}

void show_user_alloc() 
{
    // Different versions of overloaded new() and delete()

    // Simple new overload
    {
        cout << "Test simple new overload" << endl;
        user_alloc* x = new user_alloc();
        x->test();
        delete x;
    }

    // Array new overload
    {
        cout << "Test array new overload" << endl;
        user_alloc* x = new user_alloc[10];
        x[1].test();
        delete[] x;
    }

    // Placement new overload
    {
        cout << "Test placement new overload" << endl;
        user_alloc* x = new user_alloc();
        x->test();
        x->~user_alloc();

        user_alloc* y = new (x) user_alloc();
        // Can be directly called in functional form only!
        // Will be implicitly called in case of exception
        user_alloc::operator delete(y, x);
        delete x;
    }

    // new_handler replacement new overload
    {
        cout << "Test new_handler replacement new overload" << endl;
        user_alloc* x = new (test_placement_new_handler) user_alloc();
        x->test();
        delete x;
    }
}

void show_new_handler() 
{
    // Demonstrate how to use the mixture class from Meyers 1-7,
    // replacing the global new_handler with its own

    // Try with a very large object
    try {
        LargeObject::set_new_handler(NewHandlerSupport<LargeObject>::no_more_memory);
        LargeObject* x = new LargeObject();
        delete x;
    } 
    catch (const std::bad_alloc& e) {
        cerr << "Lack of memory: " << e.what() << '\n';
    }

    // Try with a regular object
    try {
        SmallObject::set_new_handler(NewHandlerSupport<SmallObject>::no_more_memory);
        SmallObject* x = new SmallObject();
        delete x;
    } 
    catch (const std::bad_alloc& e) {
        cerr << "Lack of memory: " << e.what() << '\n';
    }
}

void show_memory_pool() 
{
    // Allocated from the pool, addr = 0x003789d0
    memory_pool_item* mpi1 = new memory_pool_item();
    mpi1->test();

    // Allocated from the pool, addr = 0x003789d8 (prev + sizeof memory_pool_item)
    memory_pool_item* mpi2 = new memory_pool_item();
    mpi2->test();

    // Deallocated both from the pool
    delete mpi1;
    delete mpi2;

    // Allocated from the pool again, should have the same addr = 0x003789d0
    // (block 0 free again)
    memory_pool_item* mpi3 = new memory_pool_item();
    mpi3->test();
    delete mpi3;
}

class class_with_non_trivial_destructor 
{
private:
    size_t s;
    int* data;

public:
    class_with_non_trivial_destructor(size_t new_s = 10) : s(new_s) {
        printf("class_with_non_trivial_destructor\n");
        data = new int[s];
    }

    ~class_with_non_trivial_destructor() {
        printf("~class_with_non_trivial_destructor\n");
        if (data) delete[] data;
    }

private:
    class_with_non_trivial_destructor(const class_with_non_trivial_destructor&);
    class_with_non_trivial_destructor& operator=(const class_with_non_trivial_destructor&);
};

void crash() 
{
    class_with_non_trivial_destructor* bad_array = new class_with_non_trivial_destructor[10];

    // Debug mode triggers assert _ASSERTE(_BLOCK_TYPE_IS_VALID(pHead->nBlockUse));
    // Release mode crashes
    delete bad_array;
}

struct base 
{
    int data;

    base() {
        cout << "ctor of base\n";
    }

    ~base() {
        cout << "dtor of base\n";
    }
};

struct derived : base 
{
    derived() {
        cout << "ctor of derived\n";
    }

    virtual ~derived() {
        cout << "dtor of derived\n";
    }
};

void crash_2() 
{
    base* p = new derived;

    std::cout << p << '\n';
    base* b = p;
    std::cout << b << '\n';

    // Trying to delete data, but in fact this is a vtable pointer
    delete p;
}


int main() 
{
    return 0;
}

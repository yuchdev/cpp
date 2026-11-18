#pragma once

// Nine places an object can come into existence:
// 1. Named automatic object (stack)
// 2. Free-store object (new/delete)
// 3. Non-static class member
// 4. Element of a C array
// 5. Local static object
// 6. Global/namespace-scope object
// 7. Temporary object
// 8. Object created by a user-defined allocator
// 9. (Non-case) union member -- a union cannot have members with
//    constructors/destructors, because the compiler has no way to know
//    which member's constructor to run

/** @brief A plain, non-aggregate class: it has a user-declared constructor,
    so `new T()` and `new T` behave identically for it -- see AggregateOfInts
    in this header for the case where they genuinely differ. */
struct SimpleStruct
{
    int a;
    SimpleStruct();
    SimpleStruct(int);
    ~SimpleStruct();
};

class A;

/** @brief A class containing another class as a member. */
class SimpleClass
{
public:
    SimpleClass();
    SimpleClass(int i);
    SimpleClass(const SimpleClass& c);
    ~SimpleClass();

    static void out_static_array();

    // A forward-declared, incomplete type is legal here, by value, because
    // this is only a DECLARATION -- the compiler does not need A's layout
    // to parse a function signature. The moment anyone tries to actually
    // CALL get_a with a real A object, A must be complete at the call
    // site; see show_forward_declared_by_value() in construct.cpp.
    A get_a(A a);

private:
    SimpleStruct intrnal;
    int b {};

    // A C-array member can only ever be default-constructed element by
    // element (no per-element arguments possible), which is most of why
    // std::array/std::vector displaced raw array members.
    int arr[5];

    static const char ch_array[];
    static const int int_array[];

    // Shared by every object of the class, inherited as the SAME variable
    // by every derived class -- not a per-object counter at all.
    static int _counter;
};

/** @brief An aggregate: no user-declared constructors, no private/protected
    non-static data members, no virtual functions, no base classes. This is
    the type `new T()` vs. `new T` actually behaves differently for. */
struct AggregateOfInts
{
    int a;
    int b;
};

/** @brief Pre-C++11 non-copyable idiom: private, declared-but-not-defined
    copy operations. Shown only as a historical footnote -- in new code,
    delete the operations instead (see 02_constructors/main.cpp for =delete),
    which gives a clearer compiler diagnostic and documents intent directly
    in the member list rather than by omission. */
class NonCopyClass
{
private:
    int d;
    NonCopyClass(const NonCopyClass& n);
    NonCopyClass& operator=(const NonCopyClass& n);
public:
    NonCopyClass() : d(5) {}
};

/** @brief A class holding a pointer to another heap object, and a
    (dangerous if misused) pointer to its own type. */
class ContainsPointer
{
public:
    ContainsPointer(SimpleClass* pc = new SimpleClass) : _pc(pc) {}
    ~ContainsPointer() { delete _pc; }

private:
    SimpleClass* _pc = nullptr;
    ContainsPointer* _p_cp = nullptr;
};

void show_forward_declared_by_value();

void create_static(int i);

/** @brief Meyers' singleton: a function-local static, initialized exactly
    once on first call, in a way the standard guarantees is thread-safe
    without any explicit locking by the caller (since C++11). */
int& return_static();

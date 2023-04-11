#include <vector>
#include <list>
#include <string>
#include <iostream>
using namespace std;

// Chapter 3 N.Josuttis
// Simple class templates

// Declaration
template <typename T>
class stack
{
public:
    stack();
    void push(const T& p);
    void pop();
    const T& top() const;
    bool empty() const;
private:
    std::vector<T> _stack;
};


// Implementation
template <typename T>
stack<T>::stack() {}

template <typename T>
void stack<T>::push(const T& p)
{
    _stack.push_back(p);
}

template <typename T>
void stack<T>::pop()
{
    _stack.pop_back();
}

template <typename T>
const T& stack<T>::top() const
{
    return _stack.back();
}

template <typename T>
bool stack<T>::empty() const
{
    return _stack.empty();
}

// ------------------- Template specialization -------------------
// Declaration of a template specialized for std::string
// Note, we do not specify the 'typename' keyword anymore
template <>
class stack<std::string>
{
public:
    stack();
    void push(const std::string& p);
    void pop();
    const std::string& top() const;
    bool empty() const;
private:
    // In this specialization we change the vector to a list
    std::list<std::string> _stack;
};

// We don't write template <> in the methods implementation
// In fact, it's a simple class
stack<std::string>::stack() {}

void stack<std::string>::push(const std::string& p)
{
    _stack.push_back(p);
}

void stack<std::string>::pop()
{
    _stack.pop_back();
}

const std::string& stack<std::string>::top() const
{
    return _stack.back();
}

bool stack<std::string>::empty() const
{
    return _stack.empty();
}

// ------------------- Partial specialization -------------------
// When there is more than one parameter, but one of them is specialized
// Possible only for template classes
template <typename T1, typename T2>
class double_stack
{
public:
    double_stack()
    {
        cout << "double_stack" << endl;
    }
    // ...
private:
    std::vector<T1> _stack1;
    std::vector<T2> _stack2;
};

// The following specializations are possible

// 1. Second argument is int
template <typename T1>
class double_stack<T1, int>
{
public:
    double_stack()
    {
        cout << "double_stack<T1, int>" << endl;
    }
    // ...
private:
    std::vector<T1> _stack1;
    std::vector<int> _stack2;
};

// 2. Arguments are the same
template <typename T>
class double_stack<T, T>
{
public:
    double_stack()
    {
        cout << "double_stack<T, T>" << endl;
    }
    // ...
private:
    std::vector<T> _stack1;
    std::vector<T> _stack2;
};

// 3. Arguments are pointers
template <typename T1, typename T2>
class double_stack<T1*, T2*>
{
public:
    double_stack()
    {
        cout << "double_stack<T1*, T2*>" << endl;
    }
    // ...
private:
    std::vector<T1*> _stack1;
    std::vector<T2*> _stack2;
};

// With partial specialization, it is not necessary to write in the implementation
// complete template form with <> brackets (ClassType<T>::method())
// Be careful about ambiguities implementing partial specializations!

// ------------------- Template default arguments -------------------
// Also a stack, but with the option to pass a custom underlying container, like in STL
// which is std::vector by default
// Arguments by default should be the last in the list of arguments
template <typename T, typename CONTAINER = std::vector<T> >
class custom_stack
{
public:
    custom_stack();
    void push(const T& p);
    void pop();
    const T& top() const;
    bool empty() const;
private:
    CONTAINER _custom_stack;
};

// Implementation
template <typename T, typename CONTAINER>
custom_stack<T, CONTAINER>::custom_stack() {}

template <typename T, typename CONTAINER>
void custom_stack<T, CONTAINER>::push(const T& p)
{
    _custom_stack.push_back(p);
}

template <typename T, typename CONTAINER>
void custom_stack<T, CONTAINER>::pop()
{
    _custom_stack.pop_back();
}

template <typename T, typename CONTAINER>
const T& custom_stack<T, CONTAINER>::top() const
{
    return _custom_stack.back();
}

template <typename T, typename CONTAINER>
bool custom_stack<T, CONTAINER>::empty() const
{
    return _custom_stack.empty();
}

// 4.
struct Pet{};

struct Dog : public Pet{};

template <typename T>
struct PrintMe
{
    static void print()
    {
        std::cout << "PrintMe::print()\n";
    }
};

template <>
struct PrintMe<Pet>
{
    static void print()
    {
        std::cout << "PrintMe<Pet>::print()\n";
    }
};

void test_pet()
{
    // Would print "PrintMe::print()\n"
    // Specialization knows nothing about inheritance
    PrintMe<Dog>::print();
}

// 5.
class HoldRaii
{
    static size_t object_count = 0;
public:
    HoldRaii() = default;
    HoldRaii(const HoldRaii& other)
    {
        // increment a static counter
        object_count++;
    }
    ~HoldRaii()
    {
        object_count--;
    }
};

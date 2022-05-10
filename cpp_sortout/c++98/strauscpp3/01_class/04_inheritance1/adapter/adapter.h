#pragma once
#include <deque>

// This header demonstrates the use of private inheritance
// to implement the "Adapter" design pattern

// Double-ended queue class
class deque_t
{
public:
    deque_t();
    deque_t(const deque_t& c);
    deque_t& operator=(const deque_t& c);
    ~deque_t();

    void push_back(int val);
    void push_front(int val);
    int pop_back();
    int pop_front();
    bool is_empty();
    void clear();

protected:
private:
    deque<int> _deque;
};

// If we want to inherit a realization without interface,
// in other words, we do not want an instance of the adapter_stack_t class to access
// all operations of the deque_t class (as we can only add to the stack at the end).
// We can use _private_ inheritance, so that all of the public class members become private
// for the user of adapter_stack_t.
// However, if we want to inherit further from adapter_stack_t, it is better to use protected.
// For private inheritance, casting pointer or reference to a base class also won't work
class adapter_stack_t : /*public*/ protected /*private*/ deque_t
{
public:
    adapter_stack_t();
    ~adapter_stack_t();

    void push(int val);
    int pop();
    bool is_empty();

protected:
    void test_pr();
private:

    // Delete copy and assign operations
    adapter_stack_t(const deque_t& c);
    adapter_stack_t(const adapter_stack_t& c);
    adapter_stack_t& operator=(const adapter_stack_t& c);
    adapter_stack_t& operator=(const deque_t& c);

};


// protected and public members from deque_t became protected for write_only_adapter_stack_t user
class write_only_adapter_stack_t : protected adapter_stack_t
{
public:
    void push(int val);
};

// In case we want to open only a part of the interface
// instead of redefining methods with new names
// we can open them with `using` directive
class another_adapter_stack_t : private deque_t
{
public:
    // using can be declared in any part of the class, and it affects the entire class
    using deque_t::push_back;
    using deque_t::pop_back;
};

// The Adapter pattern could be implemented by composition as well.
// Such solution considered safer in terms of OOP and exception safety
// (a strong exception guarantee can be implemented in operator= of such class)
class composite_t
{
    deque_t d;
public:
    void push(int val) { d.push_back(val); }
    int pop() { return d.pop_back(); }
    bool is_empty() { return d.is_empty(); }
};

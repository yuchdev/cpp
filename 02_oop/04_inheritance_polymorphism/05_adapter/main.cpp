#include "adapter.h"

#include <iostream>

// adapter_stack_t inherits deque_t PROTECTED: every public (and protected)
// member of deque_t becomes protected in adapter_stack_t, so a caller only
// sees the narrow stack-shaped interface (push/pop/is_empty) that
// adapter_stack_t itself re-exposes. Copy and assignment are private, so
// adapter_stack_t objects can neither be copied nor assigned.
void show_protected_inheritance_adapter()
{
    adapter_stack_t s;
    s.push(1);
    s.push(2);
    s.push(3);

    while (!s.is_empty())
        std::cout << s.pop() << ' ';
    std::cout << '\n';
}

// write_only_adapter_stack_t narrows the interface again: it only exposes
// push(), so once pushed, values can never be retrieved through this type.
void show_write_only_adapter()
{
    write_only_adapter_stack_t w;
    w.push(10);
    w.push(20);
    // no way to read the values back: pop()/is_empty() are not visible here
}

// another_adapter_stack_t inherits deque_t PRIVATELY, then uses
// using-declarations to re-open exactly the two members it wants
// (push_back/pop_back) as public -- a different, more surgical way to
// shape an adapter's interface than overriding each method by hand.
void show_using_declaration_adapter()
{
    another_adapter_stack_t a;
    a.push_back(100);
    a.push_back(200);

    std::cout << a.pop_back() << ' ' << a.pop_back() << '\n';
}

// composite_t reaches the same result as adapter_stack_t through
// composition instead of inheritance: it HAS a deque_t rather than IS a
// deque_t. No access-control tricks are needed, and (per the header's own
// comment) it is easier to make exception-safe.
void show_composition_adapter()
{
    composite_t c;
    c.push(7);
    c.push(8);

    while (!c.is_empty())
        std::cout << c.pop() << ' ';
    std::cout << '\n';
}

int main()
{
    show_protected_inheritance_adapter();
    show_write_only_adapter();
    show_using_declaration_adapter();
    show_composition_adapter();
    return 0;
}

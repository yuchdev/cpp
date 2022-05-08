#pragma once
#include <deque>

// Demonstration of the application of template parameters 
// - The template parameter of template is defined by the keyword template <> class
// - The name inside the template parameter of the template cannot be used in the template
// - If the name of the template parameter of the template is not used anywhere, it can be omitted
// - A template parameter of themplate that has default parameters must repeat them in the parameter list
template <
    typename T,
    template <typename ELEM, typename = std::allocator<ELEM> > class CONT = std::deque >
class tmpl_stack
{
public:
    tmpl_stack();
    void push(const T& p);
    void pop();
    const T& top() const;
    bool empty() const;

    // Template method with template params
    template<typename TCOPY, template <typename, typename> class CONT_COPY>
    tmpl_stack<T, CONT>& operator=(const tmpl_stack<TCOPY, CONT_COPY>& s);

private:
    CONT<T> _copyable_stack;
    // ELEM _elem; - the template parameter of the template cannot be used
};

template <typename T, template <typename, typename> class CONT>
tmpl_stack<T, CONT>::tmpl_stack() {}

template <typename T, template <typename, typename> class CONT>
void tmpl_stack<T, CONT>::push(const T& p)
{
    _copyable_stack.push_back(p);
}

template <typename T, template <typename, typename> class CONT>
void tmpl_stack<T, CONT>::pop()
{
    _copyable_stack.pop_back();
}

template <typename T, template <typename, typename> class CONT>
const T& tmpl_stack<T, CONT>::top() const
{
    return _copyable_stack.back();
}

template <typename T, template <typename, typename> class CONT>
bool tmpl_stack<T, CONT>::empty() const
{
    return _copyable_stack.empty();
}


// Template method in template class containing template parameter
template <typename T, template <typename, typename> class CONT>
template <typename TCOPY, template <typename, typename> class CONT_COPY>
tmpl_stack<T, CONT>& tmpl_stack<T, CONT>::operator=(const tmpl_stack<TCOPY, CONT_COPY>& s)
{

    tmpl_stack<TCOPY, CONT_COPY>  tmp(s);
    _copyable_stack.clear();

    // Copy items one-by-one with compile-time check
    while (!tmp.empty()) {
        _copyable_stack.push_back(static_cast<T>(tmp.top()));
        tmp.pop();
    }
    return *this;
}

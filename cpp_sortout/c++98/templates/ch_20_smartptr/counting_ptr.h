#pragma once
#include "ref_counter.h"

// The smart pointer class with the counter privately inherits from the strategy classes
// defining the counter storage location and the procedure of memory releasing
template <typename T,
    typename ref_count_policy = simple_ref_count,
    typename deletion_policy = standard_object_policy
>
class counting_ptr : ref_count_policy, deletion_policy
{
    typedef ref_count_policy RCP;
    typedef deletion_policy DP;
public:
    // 
    counting_ptr() :_ptr() {}

    // create reference couter
    explicit counting_ptr(T* mem) :_ptr(mem)
    {
        this->init(mem);
    }

    // increment the counter
    counting_ptr(const counting_ptr& rhs)
        : ref_count_policy(rhs)
        , deletion_policy(rhs)
    {

        this->attach(rhs);
    }

    // decrement the counter
    ~counting_ptr()
    {
        this->detach();
    }

    // assignment operator with a this-check
    // incrementing the counter value for the object being assigned
    // decrementing it for itself
    counting_ptr& operator=(const counting_ptr& rhs)
    {
        if (this->_ptr != rhs._ptr) {
            // decrementing for itself
            this->detach();

            // copying policies
            ref_count_policy::operator=(rhs);
            deletion_policy::operator=(rhs);
            this->attach(rhs);
        }
        return (*this);
    }

    // accessor const
    inline const T* operator->() const
    {
        return _ptr;
    };

    inline const T& operator*() const
    {
        return *_ptr;
    };

    // accessor non-const
    inline T* operator->()
    {
        // detouch the object, create a new one
        this->detach();
        this->init(this->clone());
        return _ptr;
    };

    inline T& operator*()
    {
        // detouch the object, create a new one
        this->detach();
        this->init(this->clone());
        return *_ptr;
    };

private:

    // Helper method section

    // initialize by pointer
    void init(T* p)
    {
        if (p) {
            // the counter will be created here, 
            // we may not know the way how
            ref_count_policy::init(p);
        }
        this->_ptr = p;
    }

    // copying the pointer and incrementing the counter
    void attach(counting_ptr<T, RCP, DP> const& rhs)
    {
        this->_ptr = rhs._ptr;
        if (rhs._ptr) {
            ref_count_policy::increment(rhs._ptr);
        }
    }

    // counter decrement and release if the counter == 0
    void detach()
    {
        if (this->_ptr) {
            ref_count_policy::decrement(this->_ptr);

            // release the memory if the counter == 0
            if (ref_count_policy::is_zero(this->_ptr)) {
                ref_count_policy::dispose(this->_ptr);
                deletion_policy::dispose(this->_ptr);
            }
        }
    }

    // copy on detouch
    T* clone()
    {
        return new T(*_ptr);
    }

private:
    T* _ptr;
};

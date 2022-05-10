#pragma once

#include <utility>
#include <iterator>
#include <stdexcept>

// An iterator is an object intended for enumerating elements
// Iterators obey the principle of pure abstraction (everything that behaves as an iterator is an iterator)

// An iterator class that throws exceptions when the container ranges are crossed
// Something similar is used in debug versions.

// Nowhere does it say that the new iterator should inherit only from the iterator.
// So we will inherit directly from the properties

// Output iterators: only move forward (++, ==, !=) and read (val = (*it))
// Input iterators: only move forward (++) and write (*it) = val
// Direct iterators: input + output
// Bidirectional iterators: input+output and --
// Arbitrary access: bidirectional, +=, -=, +n, etc.
template <typename Container, typename Iter = typename Container::iterator>
class Checked_iterator : public std::iterator_traits<Iter>
{
public:

    // Validating an existing iterator from the constructor
    void valid(Iter p) const
    {

        if (cont->end() == p) {
            return;
        }

        for (Iter pp = cont->begin(); pp != cont->end(); ++p) {
            if (pp == p)
                return;
        }

        throw_oor();
    }

    // Commutative equality check
    friend bool operator == (const Checked_iterator& a, const Checked_iterator& b)
    {
        return ((a.cont == b.cont) && (a.iter == b.iter));
    }

    // Initialization by container and iterator
    Checked_iterator(Container& c, Iter p) : cont(&c), iter(p)
    {
        valid(p);
    }

    // Initialization with a container and pointer to its beginning
    Checked_iterator(Container& c) : cont(&c), iter(c.begin()) {}

    // Iterator functions - operator*
    typename std::iterator_traits<Iter>::reference operator*() const
    {
        if (iter == cont->end()) {
            throw std::out_of_range("OOR");
        }
        return iter;
    }

    // Iterator functions - operator->
    typename std::iterator_traits<Iter>::reference operator->() const
    {
        // check by dereferencing &*
        return &*iter;
    }

    // +
    Checked_iterator operator+(typename std::iterator_traits<Iter>::difference_type d) const
    {
        if (
            // check if during addition we crossed the boundries
            ((cont->end() - iter) < d) || (d < -(iter - cont->begin()))) {

            throw std::out_of_range("OOR");
        }
        // create iterator
        return Checked_iterator(cont, iter + d);
    }

    // Iterator functions - operator[]
    typename std::iterator_traits<Iter>::reference operator[](typename std::iterator_traits<Iter>::difference_type d) const
    {
        // check if during accedd we crossed or reach the boundries
        if (((cont->end() - iter) <= d) || (d < -(iter - cont->begin()))) {
            throw_oor();
        }
        // create iterator
        return iter[d];
    }

    // 
    Checked_iterator& operator++()
    {

        if (iter == cont->end()) {
            throw_oor();
        }
        ++iter;
        return *this;
    }

    // 
    Checked_iterator operator++(int)
    {
        Checked_iterator temp = *this;
        ++(*this);
        return temp;
    }

    // 
    Checked_iterator& operator--()
    {

        if (iter == cont->begin()) {
            throw_oor();
        }
        --iter;
        return *this;

    }

    // 
    Checked_iterator operator--(int)
    {
        Checked_iterator temp = *this;
        --(*this);
        return temp;
    }


    // indexing, a RA operation
    typename std::iterator_traits<Iter>::difference_type index() const
    {
        return iter - cont->begin();
    }

    // iberator base
    Iter unchecked() const
    {
        return iter;
    }
protected:

    // This exception is thrown when the boundary is exceeded -
    // this is what makes the iterator "checkable"
    void throw_oor() const
    {
        throw std::out_of_range("OOR");
    }


private:
    // Unchecked base iterator
    Iter iter;

    // Target container
    Container* cont;
};

// Since we can't embed our iterator with validation into third-party containers,
// we can create coneiners for them by inheriting
template <typename C>
class Checked_container : public C
{
public:

    typedef Checked_iterator<C> iterator;
    typedef Checked_iterator<C, typename C::const_iterator> const_iterator;

    // 
    Checked_container() :C() {}

    // container of size
    explicit Checked_container(size_t n) :C(n) {}

    // Begin/end iterators
    iterator begin()
    {
        return iterator(*this, C::begin());
    }

    const_iterator begin() const
    {
        return const_iterator(*this, C::begin());
    }

    iterator end()
    {
        return iterator(*this, C::end());
    }

    const_iterator end() const
    {
        return const_iterator(*this, C::begin());
    }

    //  []
    typename C::reference operator[] (typename C::size_type n)
    {
        return Checked_iterator<C>(*this)[n];
    }

    //  base container
    C& base()
    {
        return this;
    };

};


// Another example of a custom iterator
// Iterator for insertion into an associative container (set)
// Since it is used only for writing, void is passed as element types and diff_t
template <typename CONT>
class associative_insert_iterator : public std::iterator< std::output_iterator_tag, void, void, void, void >
{
public:
    explicit associative_insert_iterator(CONT& c) :cont(c) {}

    // we can only assign using this iterator
    associative_insert_iterator<CONT>& operator=(const typename CONT::value_type& val)
    {
        cont.insert(val);
        return *this;
    }

    associative_insert_iterator<CONT>& operator*()
    {
        return *this;
    }

    // the increment operation is emulated (not needed for insertion into set)
    associative_insert_iterator<CONT>& operator++()
    {
        return *this;
    }

    associative_insert_iterator<CONT>& operator++(int)
    {
        return *this;
    }

protected:
    CONT& cont;
};

template <typename CONT>
inline associative_insert_iterator<CONT> associative_inserter(CONT& c)
{
    return associative_insert_iterator<CONT>(c);
}

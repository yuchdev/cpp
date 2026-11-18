#include <cstring>
#include <algorithm>
#include <cassert>


namespace meyers_refcount
{

// See Meyers 2-29
// Ref count string example

//////////////////////////////////////////////////////////////////////////
// First refcount version. Use internal counter

class rc_string
{
public:

    // constructor from char, creates a new string
    explicit rc_string(const char* val) :_value(new string_value(val)) {}

    // copy constructor increments ref counter
    rc_string(const rc_string& rhs) :
        _value((rhs._value->_shareable) ? (rhs._value) : (new string_value(rhs._value->_data)))
    {
        // attach new value
        if (rhs._value->_shareable)
            ++_value->_ref_count;
    }

    ~rc_string()
    {
        // cleanup if this refcount == 0
        check_delete();
    }

    rc_string& operator=(const rc_string& rhs)
    {
        if (_value == rhs._value) {
            return *this;
        }

        if (rhs._value->_shareable) {
            // cleanup if this refcount == 0
            check_delete();

            // attach new value
            _value = rhs._value;
            ++_value->_ref_count;
        }
        else {
            _value = new string_value(rhs._value->_data);
        }

        return *this;
    }

    // accessor - just return value
    const char& operator[](size_t index) const
    {
        return _value->_data[index];
    }

    // mutator - detach new value
    char& operator[](size_t index)
    {
        if (_value->_ref_count > 1) {
            --_value->_ref_count;
            _value = new string_value(_value->_data);
        }
        // we could save a pointer to returned object
        // so it should be detached next time
        _value->_shareable = false;
        return _value->_data[index];
    }

protected:
    void check_delete()
    {

        if (0 == --_value->_ref_count) {
            delete _value;
        }
    }
private:


    struct string_value
    {
        string_value(const char* init_val) :
            _ref_count(1),
            _data(new char[strlen(init_val) + sizeof(char)]),
            _shareable(true)
        {
            // Copy the null terminator too (+1), not just the characters:
            // _data is later re-read with strlen() (every copy/detach
            // calls this same constructor again from an existing _data),
            // so leaving the terminator uninitialized is a real
            // heap-buffer-overflow waiting to happen the first time this
            // string is copied or detached -- caught by AddressSanitizer,
            // not by anything short of actually exercising this code.
            std::copy(init_val, init_val + strlen(init_val) + 1, _data);
        }

        ~string_value() { delete[] _data; }
        size_t _ref_count;
        char* _data;
        bool _shareable;
    };

    string_value* _value;
};

//////////////////////////////////////////////////////////////////////////
// Second refcount version. Use smart pointer and base class for refcount

// Basic class for ref count *value* (!)
class rc_object
{
public:
    // owner should set correct ref_count!
    rc_object() : _ref_count(0), _shareable(true) {}

    // owner should set correct ref_count!
    rc_object(const rc_object&) :_ref_count(0), _shareable(true) {}

    rc_object& operator=(const rc_object&)
    {
        // refcount value should not be really assigned
        // wrapping objects assigned instead
        return *this;
    }

    // could not create separately
    virtual  ~rc_object() = 0;

    void add_ref()
    {
        ++_ref_count;
    }

    void remove_ref()
    {
        check_clear();
    }

    // modifying operation is performed, references to representation could exist
    void mark_unshareable()
    {
        _shareable = false;
    }

    // object is already shared
    bool is_shared() const
    {
        return _ref_count > 1;
    }

    // whether performed modifying operations
    bool is_shareable() const
    {
        return _shareable;
    }

    // cleanup if all references were removed
    void check_clear()
    {
        if (--_ref_count == 0)
            delete this;
    }
protected:

private:
    size_t _ref_count;
    bool _shareable;
};

// pure virtual destructor should be defined
rc_object::~rc_object() {}


// smart pointer for saving ref count value
// should be contained in wrapping object (not in value!)
template <typename T>
class rc_ptr
{
public:
    rc_ptr(T* ptr = nullptr) :_ptr(ptr)
    {
        re_init();
    }

    rc_ptr(const rc_ptr& rhs) :_ptr(rhs._ptr)
    {
        re_init();
    }

    rc_ptr& operator=(const rc_ptr& rhs)
    {

        // check self
        if (_ptr == rhs._ptr) {
            return *this;
        }

        // detach value
        if (_ptr) {
            T* old_ptr = _ptr;
            _ptr = rhs._ptr;
            re_init();

            if (old_ptr) {
                old_ptr->remove_ref();
            }

        }

        return *this;
    }

    ~rc_ptr()
    {
        if (_ptr)
            _ptr->remove_ref();
    }

    T* operator->() const
    {
        return _ptr;
    }

    T& operator*() const
    {
        return *_ptr;
    }
protected:
    void re_init()
    {

        if (_ptr == nullptr) {
            return;
        }

        // unable to use shared copy anymore
        // detach value
        if (false == _ptr->is_shareable()) {
            _ptr = new T(*_ptr);
        }

        // new reference 
        // (never mind for the new or existing object)
        _ptr->add_ref();
    }
private:

    // T should point to rc_object-derived instance 
    // and provide copy-constructor
    T* _ptr;
    // Smart pointer could even contain a holder for library classes
    // see Meyers 2-29
};


// Ref count usage
class rc_string2
{
public:
    rc_string2(const char* init_val) : _value(new string_value(init_val)) {}

    // We don't need copy constructor and operator=
    // rc_ptr defaults will work fine

    const char& operator[](size_t index) const
    {
        return _value->_data[index];
    }

    char& operator[](size_t index)
    {

        if (_value->is_shared()) {
            _value = new string_value(_value->_data);
        }
        _value->mark_unshareable();
        return _value->_data[index];
    }

private:
    // all refcount logics was moved to base class
    // value class MUST provide copy constructor!
    // Also that class is internal, so that no one 
    // accidentally created it on the stack
    // It uses 'delete this' and could be created
    // in the heap only!
    struct string_value : public rc_object
    {

        // init value
        string_value(const char* init_val) :
            _data(new char[strlen(init_val) + sizeof(char)])
        {

            // strcpy(_data, init_val);
            std::copy(init_val, init_val + strlen(init_val) + 1, _data);
        }

        // copy value - only after modifying operations
        string_value(const string_value& rhs) :
            _data(new char[strlen(rhs._data) + sizeof(char)])
        {

            // strcpy(_data, rhs._data);
            std::copy(rhs._data, rhs._data + strlen(rhs._data) + 1, _data);
        }

        ~string_value()
        {
            delete[] _data;
        }

        char* _data;
    };

    rc_ptr<string_value> _value;
};

} //namespace meyers_refcount

// Proves the two claims that give copy-on-write its name, for both
// implementations, using only their public interface: a copy SHARES the
// same buffer (no allocation, same address) until one side is mutated,
// at which point exactly that side silently detaches into its own copy
// -- the other, unmutated side is left completely untouched.
//
// A genuinely easy mistake while writing this kind of test: indexing a
// NON-const rc_string -- even just to read, never to write through the
// result -- calls the NON-const operator[], which detaches unconditionally
// (it has no way to know you won't write through the reference it hands
// back). Checking "are these still shared?" without destroying the very
// sharing you're checking for requires going through a CONST reference,
// which is exactly the const/non-const overload-resolution rule 03_constness
// demonstrates, now mattering for real.
void show_copy_on_write_sharing_and_detach()
{
    using namespace meyers_refcount;

    {
        rc_string s1("hello");
        rc_string s2(s1); // copy: shares s1's buffer, no new allocation

        const rc_string& view1 = s1;
        const rc_string& view2 = s2;
        assert(&view1[0] == &view2[0]); // const access: same buffer, no detach

        s2[0] = 'H'; // non-const access: detaches s2 into its own buffer

        assert(&view1[0] != &view2[0]); // no longer sharing
        assert(view1[0] == 'h');        // s1's buffer is untouched by s2's write
        assert(view2[0] == 'H');
    }

    {
        rc_string2 t1("world");
        rc_string2 t2(t1); // copy via rc_ptr: shares t1's buffer

        const rc_string2& tview1 = t1;
        const rc_string2& tview2 = t2;
        assert(&tview1[0] == &tview2[0]);

        t2[0] = 'W'; // detaches t2 (rc_object::is_shared()/mark_unshareable() inside)

        assert(&tview1[0] != &tview2[0]);
        assert(tview1[0] == 'w');
        assert(tview2[0] == 'W');
    }
}

int main()
{
    show_copy_on_write_sharing_and_detach();
    return 0;
}

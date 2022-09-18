#pragma once

// Very plain scoped pointer class
template <typename T>
class simple_ptr
{
public:

    // constructor aquiring memory ownership
    explicit simple_ptr(T* mem = nullptr) : _ptr(mem) {}

    // In custom smart pointer classes it is possible to cast one type into another 
    // using template type conversion operator, if the copmiler allows it, of course.
// A rather dangerous approach, given here as an example
    template <typename NewType>
    operator simple_ptr<NewType>()
    {
        // return 'new' value because new pointer should own a new copy
        // so that destroy it (otherwise it should be something like refcount)
        return simple_ptr<NewType>(new T(*_ptr));
    }

    // Release memory
    ~simple_ptr()
    {
        delete _ptr;
    }

    // Pointer semantic
    T* operator->() const
    {
        return _ptr;
    };

    T& operator*() const
    {
        return *_ptr;
    };

    // Functional analog of operator->
    T* get() const
    {
        return _ptr;
    };

    // Release memory
    T* release()
    {
        T* ptr = _ptr;
        _ptr = 0;
        return ptr;
    };


private:
    T* _ptr;
};



class music
{
public:
    virtual ~music() {}
    virtual void play() = 0;
    virtual void pause() = 0;
};

class mp3 : public music
{
public:
    virtual ~mp3() {}
    virtual void play() { cout << "mp3 play\n"; }
    virtual void pause() { cout << "mp3 pause\n"; }
};

class ogg : public music
{
public:
    virtual ~ogg() {}
    virtual void play() { cout << "ogg play\n"; }
    virtual void pause() { cout << "ogg pause\n"; }
};

// This example will not compile without implemented casting into simple_ptr<music>
// For this purpose, there is a template conversion operator in the class
void test_play(const simple_ptr<music>& mucis)
{
    mucis->play();
}

void show_smart_ptr()
{

    simple_ptr<mp3> mymp3(new mp3);
    simple_ptr<ogg> myogg(new ogg);
    test_play(mymp3);
}

int main()
{
    show_smart_ptr();
    return 0;
}

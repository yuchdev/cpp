void complex_object_exception()
{

    // create a complex class with multiple inheritance
    // and ineternal classes and throw an exception from constructor
    class internal
    {
        int i;
    public:
        internal() : i() { cout << "internal()\n"; }
        ~internal() { cout << "~internal()\n"; }

    };

    class base
    {
    public:
        base() { cout << "base()\n"; }
        virtual ~base() { cout << "~base()\n"; }
    };

    class derived1 : public virtual base
    {
    public:
        derived1() { cout << "derived1()\n"; }
        ~derived1() { cout << "~derived1()\n"; }
    };

    class derived2 : public virtual base
    {
    public:
        derived2() { cout << "derived2()\n"; }
        ~derived2() { cout << "~derived2()\n"; }
    };

    class finalizer : public derived1, public derived2
    {
    public:
        internal _i;
        finalizer()
        {
            throw std::runtime_error("Exception from constructor");
        }
        ~finalizer() { cout << "~finalizer()\n"; }
    };

    try {
        finalizer f;
    }
    catch (const std::exception& e) {
        // finalizer has not been created
        cout << e.what();
    }
}


// The function demonstrates private exception inheritance
class my_exception : std::exception
{ // private inheritance
public:
    my_exception() : std::exception("my_exception") {}
};

void foo()
{
    throw my_exception();
}

void show_exception_private_inherit()
{
    try {
        foo();
    }
    // will not be caught here, even though the underlying exception
    // inheritance is private, and what() is closed
    catch (const std::exception&) {
        std::cout << 1 << std::endl;
    }
    catch (const my_exception&) {
        std::cout << 2 << std::endl;
    }
}

int main()
{
    return 0;
}


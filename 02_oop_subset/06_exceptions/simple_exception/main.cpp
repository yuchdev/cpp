void show_simple_ex()
{

    triangle* pt1 = 0;
    try {
        // Everything is nice here
        //triangle t1(4,5,6);

        // Throw an exception here
        // the destructor will not be called, which will cause memory leak
        //triangle t2(1,2,3);

        // Here, in case of an exception, no memory will be allocated by new
        pt1 = new triangle(4, 5, 6);
        //pt1 = new triangle(1,2,3);

        // Let's throw an exception that obviously doesn't get caught
        // But we can catch its parent
        pt1->throw_test();

        pt1->test_nothrow();
        pt1->test_throw_some1();
        pt1->test_throw_some2();

    }
    // First, we catch derived class exceptions
    catch (triangle::wrong_param& w) {
        cout << w.what() << endl;
        cout << w.get_wrong_param() << endl;
    }
    catch (triangle::non_triangle_exception& w) {
        cout << w.what() << endl;
        cout << w.wrong_size() << endl;
    }
    // Then the base classes; due to polymorphism, also all children
    catch (triangle::triangle_exception& w) {
        cout << w.what() << endl;
    }

    // Release the memory in any case
    delete pt1;
}


void show_standard_handlers()
{
    finished f;
    f.unexpected();

    try {
        f.test_custom_unexpected();
    }
    catch (double) {
        // VC++ my_unexpected does not called
        cout << "double caught" << endl;
    }
}

int main()
{
    return 0;
}


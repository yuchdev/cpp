// Integer division by 0 
void show_zero_div_exception()
{

    // TODO: make cross-platform!
#if defined(_WIN32) || defined(_WIN64)
    // Translate SE into C++ exception; must be compiled with `/EHa` key
    structured_exception::enableStructuredExceptions();
    try {
        set_zero_div_exception();
        cout << "Integer division by zero" << endl;
        double ret = zero_div(0.);
        cout << "Division by zero returned " << ret << endl;
    }
    catch (const structured_exception& e) {
        cout << e.what() << endl;
    }
    catch (const std::exception& e) {
        cout << e.what() << endl;
    }
    catch (...) {
        cout << "Unknown exception" << endl;
    }
    _clearfp();
#endif
}

// Floating point division by 0 
void show_fpe_zero_div()
{

    // TODO: make cross-platform!
#if defined(_WIN32) || defined(_WIN64)
    // Translate SE into C++ exception; must be compiled with `/EHa` key
    structured_exception::enableStructuredExceptions();
    try {
        set_zero_div_exception();
        cout << "FP division by zero" << endl;
        double ret = fpe_zero_div(0.);
        cout << "Division by zero returned " << ret << endl;
    }
    catch (const structured_exception& e) {
        cout << e.what() << endl;
    }
    catch (const std::exception& e) {
        cout << e.what() << endl;
    }
    catch (...) {
        cout << "Unknown exception" << endl;
    }
    _clearfp();
#endif
}

// The function shows the SEH mechanism
void show_seh()
{
#if defined(_WIN32) || defined(_WIN64)
    sehhandler seh;
    seh.test_seh();
#endif
}

int main()
{
    // int and float zero div exception
    // test with /EHa
    show_zero_div_exception();
    show_fpe_zero_div();

    show_seh();
    return 0;
}


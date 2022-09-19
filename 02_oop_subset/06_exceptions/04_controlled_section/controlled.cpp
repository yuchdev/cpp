// Controlled constructor intercept all exceptions from the constructor body
// and the initialization list, and passes them further
controlled::controlled() try : _vect(1000)
{
    for (int i = 0; i < 1000; i++)
        _vect.push_back(1);

    throw std::runtime_error("constructor exception");
}
catch (const exception& e) {
    cout << e.what() << endl;
}
catch (...) {
    cout << "Unknown exception" << endl;
}

// Standard says, if the `return` expression occurs inside the function-try-block handler 
// of the constructor, then the program is considered to be ill-formed

// Destructors should not throw exceptions
// At least they must not leave its scope
controlled::~controlled() throw() {}

// Controlled method
void controlled::controlled_method() const try {
    throw std::runtime_error("controlled method exception");
}
catch (std::exception& e) {
    cout << e.what() << endl;
}

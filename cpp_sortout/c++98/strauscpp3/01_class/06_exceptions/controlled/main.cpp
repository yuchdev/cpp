// Show controlled constructor
void show_controlled()
{
    try {
        controlled c;
        c.controlled_method();
    }
    catch (const std::exception& e) {
        // exception from constructor is rethrown here
        cout << e.what() << endl;
    }
    catch (...) {
        cout << "Unknown exception" << endl;
    }
}



int main()
{
    return 0;
}

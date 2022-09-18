// The function demonstrates the SEH mechanism
void show_seh()
{
#if defined(_WIN32) || defined(_WIN64)
    sehhandler seh;
    seh.test_seh();
#endif
    // catch(...) handles all SE unconditionally
}


int main()
{
    return 0;
}


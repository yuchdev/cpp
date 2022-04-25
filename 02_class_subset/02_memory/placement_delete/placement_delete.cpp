#include <iostream>
#include <new>

void show_placement_delete(){

    struct X {
        X() {
            throw std::runtime_error("X object never be created");
        }
        // custom placement new
        static void* operator new(std::size_t sz, bool b){
            std::cout << "custom placement new called, b = " << b << '\n';
            return ::operator new(sz);
        }
        // custom placement delete
        // could be called in functional form only
        // however, pair delete could be found in case of exception
        static void operator delete(void* ptr, bool b)
        {
            std::cout << "custom placement delete called, b = " << b << '\n';
            ::operator delete(ptr);
        }
    };

    try {
        X* p1 = new (true) X;
    }
    catch (const std::exception& e) {
        std::cout << e.what();
    }
}

void test_placement_new_handler()
{
    std::cout << "No more memory!\n";
    throw std::bad_alloc();
}


int main(){

    show_placement_delete();
    //show_memory_pool();

    return 0;
}

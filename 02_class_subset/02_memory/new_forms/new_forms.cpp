#include <cstdio>
#include <new>
#include <iostream>

using namespace std;

/** @brief
3 forms of operator new()
1 - regular(show_new1)
2 - placement(show_new2)
3 - nothrow
*/

void show_new1() {
	int* i = new int(6);
	printf("%d\n", *i);
	delete i;
}


void show_new2() {
	int * i = new int(12);
	printf("*i = %d\n", *i);

	int * j = new(i) int(7);
	printf("*j = %d\n", *j);
	printf("*i = %d\n", *i);
	delete i;
}

// new_op_new.cpp
// compile with: /EHsc

using namespace std;

class MyClass
{
public:
	MyClass() : imember()
	{
		cout << "Construction MyClass." << this << endl;
	};

	~MyClass()
	{
		imember = 0; cout << "Destructing MyClass." << this << endl;
	};
	int imember;
};

void show_new_delete()
{
	// Regular form of operator new()
	// throws bad_alloc in case of inability to allocate
	MyClass* fPtr = new MyClass;
	delete fPtr;

    // Placement new allows us to separate above two things. 
    // In placement new, we can pass a preallocated memory and construct an object in the passed memory
    // Does not require release by operator delete()

	char x[sizeof(MyClass)];
	MyClass* fPtr2 = new(x)MyClass;
	fPtr2->~MyClass();
	cout << "The address of x[0] is : " << (void*)&x[0] << endl;

    // It is intensely used in embedded programming of microcontrollers
    // Where you need a strict control of object placement in meory
    // Often they require from developer knowledge 
    // how much memory should be used in compile time

	// nothrow, legasy version of operator new
	MyClass* fPtr3 = new(nothrow)MyClass;
	if (fPtr3 == 0){
		/*error*/
	}
	delete fPtr3;
}


void no_more_memory(){
	cout << "no more memory: new_handler" << endl;
	// new-handle is being called many times, therefore you need to decide what strategy to use
	// 1. release required memory
	// 2. exit the application
	// 3. set different handler with set_new_handler()
	// 4. set zero new-handler: set_new_handler(0) - in this case just bad_alloc will be thrown

	exit(1);
}

void allocate_a_lot(){
	size_t large_memory = 4000000000;
	long* l = new long[large_memory];
}

void show_bad_alloc(){
	try{
		allocate_a_lot();
	}
	catch (bad_alloc&){
		cout << "no more memory: bad_alloc handler" << endl;
	}

	set_new_handler(no_more_memory);
	allocate_a_lot();
}

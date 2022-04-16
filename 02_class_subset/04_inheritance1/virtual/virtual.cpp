#include "virtual.h"


// ---------------  --------------- 
/**Выполняем функцию f на создании и разрушении класса*/
A_virt::A_virt(){
	cout << "A_virt(): ";
	f();
}

A_virt::~A_virt(){
	cout << "~A_virt(): ";
	f();
}

ostream& A_virt::f() const { 
	return (cout << "A::f" << endl); 
}

// ---------------  --------------- 
/** То же для класса B_virt */
B_virt::B_virt(){
	cout << "B_virt(): ";
	f();
}

B_virt::~B_virt(){
	cout << "~B_virt(): ";
	f();
}

ostream& B_virt::f() const { 
	return ( cout << "B::f" << endl ); 
}

// ---------------  --------------- 
v_base::v_base(){
	cout << "v_base::v_base()" << endl;
}
// чисто виртуальный деструктор может иметь определение!
v_base::~v_base()/* = 0*/{
	cout << "v_base::~v_base() = 0" << endl;
}

void v_base::pure(){
	cout << "v_base::pure()" << endl;
}

// ---------------  --------------- 
v_derived::v_derived(){
	cout << "v_derived::v_derived()" << endl;
}
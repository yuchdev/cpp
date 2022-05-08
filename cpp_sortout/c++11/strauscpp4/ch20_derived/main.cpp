#include <iostream>

using namespace std;

/*

New features:
* Override controls: override and final (20.3.4)
* Inheriting constructors (20.3.5.1)

Examples:
1. override and final - reuse cpp3 examples
2. Unable to open private data using:: (check members and methods)
3. Pointer to member contravariance - use cpp3 examples
4. dynamic_cast and static_cast for casting from virtual base (including cast to void*)(22.2.3) - use cpp3 examples

*/

// 2. Unable to open private data using:: (check members and methods)
namespace cpp4 {

class base
{
public:
    void pub_method() {}
protected:
    int prot_data = 0;
    void prot_method1() {}
    void prot_method2() {}
private:
    void priv_method() {}
};

class derived: base
{
public:
    
    // ok: bring public method to interface
    using base::pub_method;

    // ok, but bad: public morozoff anti-pattern
    using base::prot_method1;
    
    // unaccessible: 'cpp4::base': not all overloads are accessible
    // using base::priv_method;
protected:
    using base::prot_data;
    using base::prot_method2;
private:
};

} // namespace cpp4 

void show_using()
{
    cpp4::base b;
    cpp4::derived d;
    d.pub_method();
    d.prot_method1();

    // using in protected section!
    // 'cpp4::derived::prot_method2': cannot access protected member
    //d.prot_method2();
}

int main()
{
    show_using();
    return 0;
}

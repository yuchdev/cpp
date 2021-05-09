#include "file1.h"
#include "file2.h"

#include <cstdlib>
#include <iostream>

// ������� ����������� ���������� ���������� �� ���������
int g = 0;
int d = g*g;

// ����������� ������ ��� ���������
// ��� ����������� ���������� ������������ �� ������
int& use_count()
{
	static int res = 0;
	return res;
}

// ���������������� ������� ���������� �������
// ����������� ���������� � ������� atexit()
// ���������, �.�. �������� ������������ ���������� ������� atexit()
void my_cleanup()
{
	// ������ ������� �� ���������
	std::cout << "cleanup handler" << std::endl;
}

// Static and extern examples:
// in namespace or global scope
int i = 0; // extern by default
const int ci = 0; // static (intern) by default
extern const int eci; // explicitly extern
static int si = 0; // explicitly static (intern)

// the same goes for functions (but there are no const functions)
int foo(); // extern by default
static int bar(); // explicitly static (intern)

// Note that instead of using static for internal linkage 
// it is better to use anonymous namespaces into which you can also put classes.
// The linkage for anonymous namespaces has changed between C++98 and C++11 
// but the main thing is that they are unreachable from other translation units.

namespace {
	int i; // external linkage but unreachable from other translation units.
	class invisible_to_others { }; // external linkage but unreachable 
}

int main()
{
	// ����� ������������ ������� �����������
	// � ������ �������� ����������
	use_extern();
	show_extern();

	// ������� ���������� ���������.
	// exit(); //  - ������� �����������
	// abort(); // - �� ������� �����������

	if ( atexit( &my_cleanup ) == 0 )
	{
		// ����� ������� my_cleanup
	}
	else
	{
		// �������� ���������� �������
		return 1;
	}

	return 0;
}
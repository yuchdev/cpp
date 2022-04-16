#include <string> // std::getline
#include <iostream>
#include <fstream>
#include <ctime>
#include "streams.h"

using namespace std;

// ��������� �� C++ ������ ����� ���� �� � ������, � � ����������� �������,
// � �������� ����� ����������� � ���������� �����
// ������ ����� (� ������) ������������ ������ � ������ ������ RAII

// ������ ��� ������ � ������� ��������������. 
// ���� ��������� ����������� ��������, ����� �� ������ �� ���� �� ����������

// ��������� ����� ������������� � ����������� �� ������. �������� - �� ��, �� ������.

void file_stream1(){
	// ����� ����� �������� ������� ����� �� ��������� �� ������� �������� �����
	// ����� � ������� ������ ������ ����� ������ ���������-��������� ����
	// ��� ������ �����

	// ������ ����� �� ���������� �����
	ifstream f("file1.txt");
	while (!f.eof()){
		string s;
		getline(f, s);
		cout << s << endl;
	}
}


void show_fstream_write(){
	srand(static_cast<unsigned>(time(0)));

	// ������ 10 ��������� ������ � ����
	ofstream f("random.txt");
	for(int i = 0; i < 10 ; i++){
		f << rand()%10 << '\n';
	}
	// ������ ��������������, ��� ��� ���� ������� ����������� ��������
	// ��������� ������ �������� � ���� �� ��������
}

void show_fstream_read(){

	// ������ � ������ �� �����
	ifstream f("random.txt");
	while(!f.eof()){
		char buf[20] = {};	// ������ ������� ��� int
		f.getline(buf, 20);
		cout << buf << endl;
	}
}

void copy_file(const std::string& from, const std::string& to){
	ifstream inf(from);
	ofstream outf(to);
	char c = 0;
	// ����������� �����������
	while(inf.get(c)) outf.put(c);
}

void show_ofstream_binary_write(){
	srand(static_cast<unsigned>(time(0)));

	// ��������� 10 ��������� ������
	cout << "Generate random array\n";
	int a[10] = {};
	for(int i = 0; i < 10 ; i++){
		a[i] = rand();
		cout << a[i] << '\n';
	}

	// ������ 10 ��������� ������ � �������� ����
	// (����� ���������� ����� ������ ����� - ��������, ���������� � ��)
	ofstream f("random.bin", ios::binary);
	

	// ���� ������ ����� - ����� ����������, ��� �����������
	size_t sz = sizeof(a);
	f.write(reinterpret_cast<char*>(a), sz);
}

void show_ofstream_binary_read(){
	int a[10] = {};
	
	// ������ ����� ���� ������ �����
	cout << "Read random array\n";
	ifstream f("random.bin", ios::binary);
	size_t sz = sizeof(a);
	f.read(reinterpret_cast<char*>(a), sz);
	for (int i = 0; i < 10; i++){
		cout << a[i] << '\n';
	}
}


//////////////////////////////////////////////////////////////////////////
// ������������

// �������� ������������������ ��������� ������ ������������� ������� 
// � �������� ������������
// ������ ��� ����������� �� ����������
struct RandomArray{
	
	size_t sz;
	int* arr;

	RandomArray(size_t s):sz(s),arr(new int[sz]){
		for(int i = 0; i < 10 ; i++){
			arr[i] = 0;
		}
	}

	void generate(){
		for(int i = 0; i < 10 ; i++){
			arr[i] = rand();
		}
	}

	~RandomArray(){
		delete[] arr;
	}
	
	//
	void load(ifstream& f){
		if(f.is_open()){
			// ��������� ������ �������
			f.read(reinterpret_cast<char*>(&sz), sizeof(size_t));

			// ��������� ��� ������
			f.read(reinterpret_cast<char*>(arr), sz*sizeof(int));
		}
	}

	//
	void save(ofstream& f){
		if(f.is_open()){
			// �������� ������ �������
			f.write(reinterpret_cast<char*>(&sz), sizeof(size_t));

			// �������� ��� ������
			f.write(reinterpret_cast<char*>(arr), sz*sizeof(int));
		}
	}
};

void show_serialize(){

	{
		RandomArray ra(10);
		ra.generate();
		ofstream f("rarray.bin", ios::binary);
		ra.save(f);
	}

	{
		RandomArray ra(10);
		ifstream f("rarray.bin", ios::binary);
		ra.load(f);
	}
}

void show_rnd_access(){
	// ��������� ������� tell/seek ��������� ���������� ��������
	// ������-������ � ��������� ��� �������� ������

	// ������� �� ������
	{
		ifstream f("random.bin", ios::binary);

		// ������� ���������� ������� � ����� (��� ������)
		ios::pos_type read_pos = f.tellg();

		// ������� � ���������� ������� ������ 
		// (����� ���������� pos_type ��� int)
		f.seekg(0);

		// ������ ���� ��������
		int out = 0;
		f.read(reinterpret_cast<char*>(&out), sizeof(int));

		// ������� � ������������� ������� ������
		// ������������� ������� ����� ���� ��� �� 
		// ����������������� ��������, ��� � �� ����������� �������:
		// beg, cur, end
		// ��������, ���������� �� ����� �����
		f.seekg(0, ios::end);
		
		// ������� ����� �� ������� ����� ��������� ���� � � bad
		// ��� ������ exception, ���� ��������� ����� ����
	}
	
	// ������� �� (��)������
	{
		// ios::in should be set
		// without it file is truncated!
		ofstream f("random.bin", ios::in | ios::binary/*|ios::trunc*/);

		// ������� ���������� ������� � ����� (��� ������)
		streampos write_pos = f.tellp();

		// ������� � ���������� ������� ������
		// (����� ���������� streampos ��� int)
		
		f.seekp(0, ios::beg);

		// ����������
		int in = 100;
		f.write(reinterpret_cast<char*>(&in), sizeof(int));

		// ������� � ������������� ������� ������
		// ���������� � ������� ������� + 1 int (4 bytes)
		f.seekp(4, ios::cur);

		// ����������
		in = 200;
		f.write(reinterpret_cast<char*>(&in), sizeof(int));
	}
}

void show_fstream(){

	file_stream1();
	show_fstream_write();
	show_fstream_read();
	string f1("random.txt");
	string f2("random2.txt");
	copy_file(f1, f2);

	show_ofstream_binary_write();
	show_ofstream_binary_read();
	show_serialize();
	show_rnd_access();
	
	// �������� ��� ��� ���������� ��������
	show_ofstream_binary_read();
}

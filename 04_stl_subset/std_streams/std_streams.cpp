#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <limits>
#include "streams.h"

using namespace std;

void show_standard_streams_out(){

	// Стандартные потоки

	// Стандартный поток ввода (по умолчанию с клавиатуры)
	string s1;
	cin >> s1;

	// Стандартный поток ввода (по умолчанию на экран)
	cout << "cout: " << s1 << endl;

	// Стандартный поток ошибок, буферированный (по умолчанию на экран)
	clog << "clog: " << s1 << endl;

	// Стандартный поток ошибок, не буферированный (по умолчанию на экран)
	cerr << "cerr: " << s1 << endl;

	// При создании потока вызывается обернутый класс-часовой (sentry), 
	// выполняющий инициализацию

	// Вывод элементарых типов
	int i = 5;
	long l = 50;
	short s = 10;
	double d = 0.000000000000001;
	float f = 5.5678235f;
	char c = 'a';
	unsigned u = 0xffff;
	bool b = true;
	// см. enum выше
	enum{ sz = 10 };

	// интегральные типы по умолчанию выводятся как есть, в десятичной системе
	cout << "int: " << i << endl;
	cout << "long: " << l << endl;
	cout << "short: " << s << endl;
	// даже если константа в HEX
	cout << "hex uint: " << u << endl;

	// дробные числа по умолчанию обрезаются до 6 знаков
	cout << "float: " << f << endl;
	
	// или в экспоненциальную форму
	cout << "double: " << d << endl;

	// символы выводятся как символы, а не числа
	cout << "char: " << c << endl;

	// bool выводятся как 1 0
	cout << "bool: " << b << endl;

	// enum выводятся как int
	cout << "enum: " << sz << endl;

	// символ можно также вывести методом put
	cout << "cout.put: ";
	cout.put(c);

	// или метода write, хотя этот метод обычно используется для ввода в двоичные файлы
	cout << " cout.wtite: ";
	cout.write(&c, 1);	
	// не очень удобно, к тому же не отслеживается \0

	// endl кроме перевода строки очищает буфер
	cout << endl;

	// указатели выводятся в HEX независимо от типа
	int* pi = &i;
	cout << "int*: " << pi << endl;

	// исключение составляют строковые литералы
    const char* str1 = "atatat";
    cout << "const char*: " << str1 << endl;

    // чтобы вывести их как указатель, необходимо привести к const void*
    cout << "(void*)char*: " << static_cast<const void*>(str1) << endl;

	// так же можно вывести как строку символьного массива
	// с помощью write
	cout.write(str1, strlen(str1));

	// Приоритет операций >> << невысок
	// cout <<	d=f << '\n'; ->
	// (cout <<	d)=(f << '\n');
}

void stream1(){
	// Ввод завершается нажатием Enter
	// Ввод продолжается до первого недопусимого символа
	// Например, в int разрешены только числа
	int i = 0;

	// введи 123.456
	cin >> i;

	cout << "You've entered int: " << i << endl;

	// Но введенное значение не пропадает, а остается в буфере,
	// и может попасть в следующую переменную
	double d = 0.;
	cin >> d; // сюда попадут оставшиеся символы из буфера
	cout << "You've entered double: " << d << endl;
	
	// ignore() выбрасывает оставшийся в буфере символ Enter
	cin.ignore();
	// для enum нужно писать собственный operator <<
}

void stream2(){
	// enter, пробелы, табуляции - по умолчанию разделители
	// введи '1 2 3'
	int a,b,c;
	cin >> a >> b >> c;
	cout << "You've entered int " << a
		<< " int " << b
		<< " int " << c
		<< endl;

	// ignore() выбрасывает оставшийся в буфере символ Enter
	cin.ignore();
}

void stream3(){
	// при вводе строк этот режим можно подавить матипулятором noskipws
	char a,b,c;
	// введи '1 2'
	cin >> noskipws >> a >> b >> c;
	cout << "You've entered char " << a
		<< " char " << b
		<< " char " << c
		<< endl;

	// ignore() выбрасывает оставшийся в буфере символ Enter
	cin.ignore();
}

void stream4(){
	// парой методам put/write являются get/read;
	char c1, c2;
	cin.get(c1);
	cin.ignore();
	cin.read(&c2, 1);

	cout << "You've entered char " << c1
		<< " char " << c2
		<< endl;

	// ignore() выбрасывает оставшийся в буфере символ Enter
	cin.ignore();
}

void input_strings1(){
	// ввести строку с пробелами можно тем же методом get 
	// с указанием количества символов
	// это можно сделать только с массивом
	char s[50] = {};

	// на последнюю позицию на всякий случай будет записан '\0'
	cin.get(s, 50);

	cout << "You've entered:  " << s
		<< endl;

	cin.ignore();

	char s1[50] = {};

	// ввод до символа . (введи atatat.a)
	cin.get(s1, 50, '.');

	cout << "You've entered:  " << s1
		<< endl;

	// remove not only /n, but delimiter character as well
	// (2 last characters)
	cin.ignore();
	cin.ignore(1, '.');

	if (cin.fail()){
		cin.clear(/*ios_base::goodbit*/);
		cout << "Fail" << endl;
	}

}

void input_strings2(){

	// при помощи ignore можно удалять любое количество символов
	// стандартный паттерн исползования - удалить все от enter до конца строки
    char s[50] = {};
	cin.get(s, 50);
	cin.ignore(numeric_limits<std::streamsize>::max(), '\n');

	cout << "You've entered:  " << s
		<< endl;

	if (cin.fail()){
		cin.clear(/*ios_base::goodbit*/);
		cout << "Fail" << endl;
	}
}


void input_strings3(){
	// для ввода строк используйте функцию getline
	// она автоматически выполняет ignore()
	string s;

	// есть также необязательный параметр, символ окончания ввода
	std::getline(cin, s);

	// можно посчитать количество реально прочитанных символов
	// последней операцией функцией cin.gcount()
	cout << "Entered  " << s << ", "<< cin.gcount() << " symbols" << endl;

	if (cin.fail()){
		cin.clear(/*ios_base::goodbit*/);
		cout << "Fail" << endl;
	}
}

void input_strings4(){
	// для ввода string используйте функцию getline
	// из хедера <string>
	string s;
	
	// '.' необязательный параметр, символ окончания ввода
	std::getline(cin, s, '.');

	// std::getline() perforas only ignore(),
	// so you should manually remove delimiter
	cin.ignore(1, '.');

	cout << "Entered:  " << s << endl;

	if (cin.fail()){
		cin.clear(/*ios_base::goodbit*/);
		cout << "Fail" << endl;
	}
}

void stream_state(){
	// Каждый поток может находится в 4 состояниях
	// good - все ок
	// fail - небольшая ошибка, например введен неожиданный символ. Можно очистить поток и попробовать вновь.
	// bad - фатальная ошибка, работа с потоком невозможна (включает в себя состояние fail)
	// eof - окончание ввода (для ввода это Ctrl-Z)

	// флаги состояния можно устанавливать функцией setstate() или clear()

	// Смоделируем ситуацию fail
	int aint[10] = {};
	size_t i = 0;
	// вводим массив в цикле 1235e67
	cin >> skipws;

	// !! Здесь мы неявно конвертируем поток при помощи operator void*()
	// т.е. при возвращении nullptr сигнализируется об окончании ввода
	while((cin >> aint[i++]) && (i < 10));

	// после поломанного символа ничего не вводилось
	for (int i = 0; i < 10; i++){
		cout << aint[i] << '\n';
	}
	
	// ...на символе e ввод сломается
	if(cin.fail()){
		// этим же методом можно устанавливать флаги
		cin.clear(/*ios_base::goodbit*/);
		cout << "Fail" << endl;
	}

	// флаги состояния можно читать функцией rdstate()
	ios::iostate state = cin.rdstate();

	// можно сделать так, что по ситуации fail/bad/eof будет бросаться исключение
	cin.exceptions(ios::badbit|ios::failbit);

	// с параметром 0 исключения отменяются
#ifdef __GNUC__
	// because GCC has a typesafe hack to prevent assignment from integers
	cin.exceptions(ios_base::iostate());
#else	
	cin.exceptions(0);
#endif
}

void show_place_back(){
	
	// прочитанные символы можно вернуть в поток ввода
	char s[50];
	cin.get(s, 50); // enter atatat

	cout << "Entered: " << s << endl;

	// положить в буффер последний считанный символ (t)
	cin.unget();

	cout << "Still entered: " << s << endl;
	cout << s << endl;

	char s1[50];
	// проверить, есть ли в потоке символ t, не читая из него
	if (cin.peek() == 't')
		cin.get(s1, 50);

	// символ t опять прочитан из потока
	cout << s1 << endl;

	cin.ignore();

	// положить символ в поток и перечитать еще раз
	cin.putback('c');
	if (cin.peek() == 'c')
		cin.get(s1, 50); // enter atatat
	cout << "Entered and one symbol has been placed before: " << s1 << endl;

	cin.ignore();
}

void stream_buffers(){
	// потоки по умолчанию буферизируют ввод-вывод
	// с ними связаны объекты типа streambuf

	// для любого буферизированного потока можно получить
	// непосредственно буфер и работать с ним
	// Это позволяет удобно выполнять многие операции
	// Например:
	// копирование одного потока в другой
	// (вывод файла в stdout)
	ifstream f("file1.txt");
	cout << f.rdbuf() << endl;

	// привязка одного буфера к нескольким потокам
	ostream hexout(cout.rdbuf());
	hexout.setf(ios::hex, ios::basefield);
	hexout.setf(ios::showbase);
	hexout << 10 << endl;

	// перенаправление ввода в файл
	ofstream out_file("out_file.txt");
	// сохраним предыдущий буфер cout
	streambuf* buf = cout.rdbuf();
	cout.rdbuf(out_file.rdbuf());
	cout << "Enter file contents" << endl;
	// восстановим предыдущий буфер cout
	cout.rdbuf(buf);

	// операция flush() принудительно записывает содержимое буффера
	cout.flush();

	// streambuf имеет protected интерфейс для наследования
	// и реализации пользовательских буфферов
}

void tie_streams(){
	
	// перед использованием паттерна "пара cout-cin"
	// необходимо связать их
	cin.tie(&cout);

	// это делается для того, чтобы строка гарантированное вывелась
	// перед вводом (этого может не произойти, т.к. вывод буферизирован)
	// tie() принудительно делает flush() перед операциями ввода
	string s;
	cout << "Please enter string: ";
	cin >> s;
	
	
}

void test_me(){
	show_place_back();
}

void show_standard_streams_input(){
	stream1();
	stream2();
	stream3();
	stream4();

	input_strings1();
	input_strings2();
	input_strings3();
	input_strings4();

	stream_state();

	stream_buffers();

	tie_streams();
}

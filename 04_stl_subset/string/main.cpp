#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

void show_string(){
	// string - это на самом деле шаблон basic_string
	basic_string<char, char_traits<char> > str;

	// char_traits - это шаблон свойств, обеспечивающий следующую информацию:
	// - типы символов
	// - особенности их приведения к интегральным типам
	// - особенности низкоуровневых операций с массивами символов
	// - особенности ввода-вывода

	// Инициализация
	string s1;
	string s2(s1);
	string s3("str");
	// нельзя инициализировать символом
	
	// передача нулевого указателя вызывает exception
    // В отладочной версии assert
#if 0
    try{
        const char* c = nullptr;
        std::string s(c);
        size_t sz = s.size();
    }
    catch (const std::exception& e){
        std::cout << e.what() << std::endl;
    }
#endif
	
	s1 = 'a';
	// ..., а присвоить - можно

	// можно присваивать c-строки, при будет не будет вызван лишний конструктор
	s2 = "atatat";

	// стандартные алгоритмы менее полезны для строк,
	// чем для других контейнеров
	// для поиска подстрок, слияния и т.п. лучше использовать
	// встроенные операции
	
	// преобразорвания в C-строку (с \0) и в массив (без \0)
	const char* c1 = s3.c_str();
	const char* c2 = s3.data();

	// можно сравнивать c-строки, при будет не будет вызван лишний конструктор
	bool b = (s2 == "atatat");

	// при поиске возвращается индекс начала подстроки или npos
	string::size_type s = s2.find("at");
	if(s != string::npos){
		// вывод строк
		cout << s2 << endl;
	}

	// подстроки
	string s4 = s3.substr(0,2);

	// функции для работы с ASCII
	// можно использовать в алгоритмах, включая модифицирующие
	// isalpha, isdigit, ispunct, tolower, ...
    std::transform(s3.begin(), s3.end(), s3.begin(), ::toupper);
}

void str_rev(char *p) {
	char *q = p;
	while (q && *q) ++q;
	for (--q; p < q; ++p, --q){
		*p = *p ^ *q;
		*q = *p ^ *q;
		*p = *p ^ *q;
	}
}

#define SWP(x,y) (x^=y, y^=x, x^=y)

void str_rev_utf8(char *p){
	char *q = p;
    str_rev(p); /* call base case */

	/* Ok, now fix bass-ackwards UTF chars. */
	while (q && *q) ++q; /* find eos */
	while (p < --q)
		switch ((*q & 0xF0) >> 4) {
		case 0xF: /* U+010000-U+10FFFF: four bytes. */
			SWP(*(q - 0), *(q - 3));
			SWP(*(q - 1), *(q - 2));
			q -= 3;
			break;
		case 0xE: /* U+000800-U+00FFFF: three bytes. */
			SWP(*(q - 0), *(q - 2));
			q -= 2;
			break;
		case 0xC: /* fall-through */
		case 0xD: /* U+000080-U+0007FF: two bytes. */
			SWP(*(q - 0), *(q - 1));
			q--;
			break;
	}
}

void show_c_string_rev(){

	char str[] = "atatat";
	printf("%s ", str);
	str_rev(str);
	str_rev_utf8(str);
	printf("%s\n", str);

	// C++ way
	//std::reverse(str.begin(), str.end());
}

int main(){

	show_string();
	show_c_string_rev();
	return 0;
}

#include "sehhandler.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include "windows.h"

using std::cout;
using std::endl;



// Функция-фильтр или транслятор. Транслятор принимает SE, 
// определяемое данным кодом исключения code, а также info.
// Транслятор в свою очередь может возбудить C++ исключение
int sehhandler::filter( unsigned code, struct _EXCEPTION_POINTERS* ep ){
	
	// Сгенерировано это исключение (Access violation)
	if( code == EXCEPTION_ACCESS_VIOLATION ){
		cout << "Access violation" << endl;
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else{
		cout << "Unexpected exception" << endl;
		return EXCEPTION_CONTINUE_SEARCH;
	}

}
// В многопоточной программе каждый поток имеет отдельный транслятор

void sehhandler::test_seh(){
	int* p = 0x0;
	// защищенный блок
	__try{
		cout << "In try 1" << endl;

		// Оба обработчика (__except и __finally) по причинам ограничений WinAPI писать нельзя
		// Для моделирования ожидаемой модели try-except-finally обработчики вкладывают друг в друга
		__try{
			cout << "In try 2.1" << endl;
			*p = 13;
			cout << "In try 2.2" << endl;
		}
		// Блок завершения: сюда попадем в любом случае
		__finally{
			cout << "In finally" << endl;
			
			// If the __try block terminated abnormally, the return value is nonzero.
			// If the __try block terminated normally, the return value is zero.
			BOOL abnormal = AbnormalTermination();
			cout << "Abnormal = " << abnormal << endl;
		}

		// Внутри __try части блока try вы можете использовать предложение leave

	}
	// А сюда - в зависимости от кода возврата filter (см. MSDN __except keyword [C])
	// filter вызовется тольео в случае исключения SEH!
	__except( filter( GetExceptionCode(), GetExceptionInformation() ) ) {
		// EXCEPTION_CONTINUE_EXECUTION попытка еще раз исполнить инструкцию, вызвавшую исключение
		// реализуется в случае, когда фильтр сам попытался исправить проблему
		// EXCEPTION_CONTINUE_SEARCH пробросить исключение выше по стеку
		// EXCEPTION_EXECUTE_HANDLER выполнить тело обработчика __except (после __finally)
		// если мы хотим обрабатывать исключения всегда (например в обработчике верхнего уровня)
		// вместо фильтра можно передавать константу
		cout << "In except" << endl;
	}
}
#endif

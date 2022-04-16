#include <deque>

void show_deque(){
	int arr[10] = {1,6,3,5,7,6,4,2,4};

	// Объединяет свойства вектора и списка
	deque<int> d;
	d.insert(d.begin(),arr,arr + sizeof(arr)/sizeof(int));
	int a = d.at(1);
	// Гарантии исключений аналогичный вектору
}


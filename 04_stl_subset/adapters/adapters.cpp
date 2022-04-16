#include <queue>
#include <deque>

void show_adapters(){
	// --- Стек - адаптер на основе deque
	stack<int> s1;
	s1.push(1);
	s1.push(2);

	// В стеке также возможно переполнение снизу
	s1.pop();s1.pop();
	//s1.pop(); - UB

	// Можно определить основу для стека из другого контейнера
	stack<int, vector<int> > s2;
	s2.push(1);

	// --- Очередь - адаптер на основе deque
	queue<int> q1;
	q1.push(1);
	q1.push(2);
	q1.push(3);

	q1.pop();
	q1.pop();
	q1.pop();
	
	// --- Очередь с приоритетом
	// По умолчанию основана на heap
	priority_queue<int> pq1;
	pq1.push(1);
	pq1.push(9);
	pq1.push(2);
	pq1.push(8);
	pq1.pop();	// выталкивается наибольший!
	pq1.push(5);
	pq1.pop();
	pq1.pop();

	// Очередь с кастомным контейнером
	priority_queue<string, deque<string> > pq2;
	pq2.push("aaaa");
	pq2.push("Aaaa");
	pq2.push("bbbb");
	pq2.push("Bbbb");
	pq2.push("krabe");
	pq2.pop();
	pq2.push("shmele");
	pq2.pop();

	// Очередь с кастомным контейнером и предикатом сравнения
	priority_queue<string, deque<string>, no_case<string> > pq3;
	pq3.push("AaAa");
	pq3.push("aaaa");
	pq3.push("abab");
	pq3.push("baba");
}


#include <queue>
#include <deque>

void show_adapters()
{
    // stack is adapter based on deque
    stack<int> s1;
    s1.push(1);
    s1.push(2);

    // In stack it is possible to overflow from the bottom
    s1.pop(); s1.pop();
    //s1.pop(); - UB

    // It is possible to define different underlying container than default deque
    stack<int, vector<int> > s2;
    s2.push(1);

    // queue is adapter based on deque
    queue<int> q1;
    q1.push(1);
    q1.push(2);
    q1.push(3);

    q1.pop();
    q1.pop();
    q1.pop();

    // Priority queue
    // based on heap data structure
    priority_queue<int> pq1;
    pq1.push(1);
    pq1.push(9);
    pq1.push(2);
    pq1.push(8);
    // pop the biggest element
    pq1.pop();
    pq1.push(5);
    pq1.pop();
    pq1.pop();

    // define different underlying container
    priority_queue<string, deque<string> > pq2;
    pq2.push("aaaa");
    pq2.push("Aaaa");
    pq2.push("bbbb");
    pq2.push("Bbbb");
    pq2.push("krabe");
    pq2.pop();
    pq2.push("shmele");
    pq2.pop();

    // define different underlying container and comparison predicate
    priority_queue<string, deque<string>, no_case<string> > pq3;
    pq3.push("AaAa");
    pq3.push("aaaa");
    pq3.push("abab");
    pq3.push("baba");
}

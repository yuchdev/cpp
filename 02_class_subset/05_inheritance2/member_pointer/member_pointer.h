#pragma once

// ƒемонстраци€ указателей на член класса

class member_pointer
{
public:
	virtual ~member_pointer() = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void suspend() = 0;
	virtual void resume() = 0;
};

class member_derived : public member_pointer{
public:
	member_derived(){}
	virtual ~member_derived(){}

	virtual void start();
	virtual void stop();
	virtual void suspend();
	virtual void resume();

	static void static_init();

	// операторы new и delete не€вно €вл€ютс€ статическими членами
	//void* operator new(size_t sz);
	//void* operator new[](size_t sz);
	//void operator delete(void* mem, size_t sz);
	//void operator delete[](void* mem, size_t sz);
};

// ”казатель на член member_pointer
typedef void(member_pointer:: *p_mem)();

// ”казатель на статический член
typedef void(*static_member_pointer)();


#pragma once
#include <pthread.h>

class condition;

class mutex
{
public:
	mutex();
	~mutex();

	bool lock();
	bool unlock();

private:
	friend class condition;

	pthread_mutex_t _mutex;
};


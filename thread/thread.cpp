#include "thread.h"

thread::thread(void *(*start_func)(void *), void * arg)
{
	pthread_create(&_id._tid, NULL, start_func, arg);
}

thread::~thread()
{
}

thread::id thread::get_id()
{
	return id(pthread_self());
}

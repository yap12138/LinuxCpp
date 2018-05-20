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

mutex::mutex()
	: _mutex(PTHREAD_MUTEX_INITIALIZER)
{
}

mutex::~mutex()
{
	pthread_mutex_destroy(&_mutex);
}

bool mutex::lock()
{
	return (0 == pthread_mutex_lock(&_mutex));
}

bool mutex::unlock()
{
	return (0 == pthread_mutex_unlock(&_mutex));
}

condition::condition()
	: _cond(PTHREAD_COND_INITIALIZER)
{
}

condition::~condition()
{
	pthread_cond_destroy(&_cond);
}

bool condition::signal()
{
	return (0 == pthread_cond_signal(&_cond));
}

bool condition::broadcast()
{
	return (0 == pthread_cond_broadcast(&_cond));
}

bool condition::wait(mutex & lock)
{
	return (0 == pthread_cond_wait(&_cond, &lock._mutex));
}

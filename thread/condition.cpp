#include "condition.h"
#include "mutex.h"

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

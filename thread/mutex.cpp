#include "mutex.h"

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

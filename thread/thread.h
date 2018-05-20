#pragma once
#include <pthread.h>

class thread
{
public:
	class id
	{
	public:
		id() : _tid() {}
		explicit id(pthread_t tid) : _tid(tid) {}
		~id() {}

		inline operator long() const { return _tid; }

		friend bool
		operator==(thread::id __x, thread::id __y) noexcept
		{
			return pthread_equal(__x._tid, __y._tid);
		}

	private:
		friend class thread;

		pthread_t _tid;
	};

public:
	thread(void * (* start_func)(void *), void * arg = NULL);
	~thread();

	inline int join() const { return pthread_join(_id._tid, NULL); }
	inline int detach() const { return pthread_detach(_id._tid); }

	inline bool joinable() const noexcept{ return !(_id == id()); }

	static id get_id();
private:
	id _id;
};



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

class condition
{
public:
	condition();
	~condition();

	bool signal();
	bool broadcast();	//Ïàµ±ÓÚnotifyall
	bool wait(mutex& lock);

private:
	pthread_cond_t _cond;
};

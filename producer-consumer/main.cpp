#include <cstdio>
#include <queue>
#include <unistd.h>
#include "../thread/thread.h"

using std::queue;

mutex mut;
condition fullcondition;	//条件变量：满
condition emptycondition;	//条件变量：空
queue<char> production;	//模拟缓冲区

bool finish = false;

/*
生产者
*/
void * producer(void * arg)
{
	char * threadinfo = (char *)arg;
	for (int i = 0; i < 100; i++)
	{
		mut.lock();
		while (production.size() >= 10)
		{
			printf("%s: waiting for an empty slot...\n", threadinfo);
			fullcondition.wait(mut);
		}
		char prod = 'A' + (i % 25);
		production.push(prod);
		printf("%s: put[%d] %c\n", threadinfo, i, prod);
		emptycondition.broadcast();
		mut.unlock();
	}
	finish = true;
	emptycondition.broadcast();
	printf("%s: exit.\n", threadinfo);
	return NULL;
}

/*
消费者
*/
void * consumer(void * arg)
{
	char * threadinfo = (char *)arg;
	printf("%s: ready.\n", threadinfo);
	while (true)
	{
		mut.lock();
		while (!finish && production.empty())
		{
			printf("%s: waiting for an full slot...\n", threadinfo);
			emptycondition.wait(mut);
		}
		if (finish && production.empty())
		{
			mut.unlock();
			break;
		}
		printf("%s: get %c\n", threadinfo, production.front());
		production.pop();
		fullcondition.broadcast();
		mut.unlock();
	}
	printf("%s: exit.\n", threadinfo);
	return NULL;
}


/*
执行顺序为cumer1 -> cumer2 -> proc。即为调用的反顺序，函数压栈，最后进的先调用。
消费完所有资源就退出
*/
void worker()
{
	thread proc(producer, (void *)"Producer");
	thread cumer2(consumer, (void *)"Consumer2");
	thread cumer1(consumer, (void *)"Consumer1");
	cumer1.join();
	cumer2.join();
	proc.join();
}

int main()
{
	printf("thread test\n");
	worker();
	printf("test end\n");
    return 0;
}
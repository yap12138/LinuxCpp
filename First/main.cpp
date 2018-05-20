#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <thread>
#include "../thread/thread.h"

pthread_t tid[2];
pthread_mutex_t mut;

int cnt = 200;

void* thread_function(void * arg)
{
	int num = *(int *)arg;
	printf("thread %d func\n", num);
	
	while (cnt != 0)
	{
		pthread_mutex_lock(&mut);
		if (cnt != 0)
		{
			cnt--;
			printf("thread %d: cnt %d\n", num, cnt);
		}
		pthread_mutex_unlock(&mut);
		usleep(2);
	}
	pthread_exit(NULL);
}

void testMyThread()
{
	thread t(thread_function);
	t.join();
}


int main()
{
	int threadNum[2] = { 0, 1 };
	pthread_mutex_init(&mut, NULL);
	if (0 != pthread_create(&tid[0], NULL, thread_function, (void *)&threadNum[0]))
	{
		printf("create thread1 succeed!\n");
	}
	if (0 != pthread_create(&tid[1], NULL, thread_function, (void *)&threadNum[1]))
	{
		printf("create thread2 succeed!\n");
	}

	//pthread_join(tid[0], NULL);
	//pthread_join(tid[1], NULL);
	printf("tid0 : %d\n", pthread_detach(tid[0]));
	printf("tid1 : %d\n", pthread_detach(tid[1]));
	//usleep(100);
	/*testMyThread();
	printf("my thread exit!\n");*/
	//getchar();
	while (cnt != 0)
	{
		usleep(10);
	}

	thread::id myid = thread::get_id();
	pthread_t ptid = pthread_self();
	printf("main: %ld exit!\n", (unsigned long)myid);
	printf("main: %ld exit!\n", (unsigned long)ptid);
	printf("they are equal: %d\n", myid == ptid);
    return 0;
}
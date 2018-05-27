#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

namespace hogason {

	int orphan()
	{
		pid_t pid;
		//创建一个进程
		pid = fork();
		//创建失败
		if (pid < 0)
		{
			perror("fork error:");
			exit(1);
		}
		//子进程
		if (pid == 0)
		{
			printf("I am the child process.\n");
			//输出进程ID和父进程ID
			printf("pid: %d\tppid:%d\n", getpid(), getppid());
			printf("I will sleep five seconds.\n");
			//睡眠5s，保证父进程先退出
			sleep(5);
			printf("pid: %d\tppid:%d\n", getpid(), getppid());
			printf("child process is exited.\n");
		}
		//父进程
		else
		{
			printf("I am father process.\n");
			//父进程睡眠1s，保证子进程输出进程id
			sleep(1);
			printf("father process is  exited.\n");
		}
		return 0;
	}

	int zombie()
	{
		pid_t pid;
		pid = fork();
		if (pid < 0)
		{
			perror("fork error:");
			exit(1);
		}
		else if (pid == 0)
		{
			printf("I am child process.I am exiting.\n");
			exit(0);
		}
		printf("I am father process.I will sleep two seconds\n");
		//等待子进程先退出
		sleep(2);
		//输出进程信息
		system("ps -o pid,ppid,state,tty,command");
		printf("father process is exiting.\n");
		return 0;
	}

	int zombieloop()
	{
		pid_t  pid;
		//循环创建子进程
		while (1)
		{
			pid = fork();
			if (pid < 0)
			{
				perror("fork error:");
				exit(1);
			}
			else if (pid == 0)
			{
				printf("I am a child process.\nI am exiting.\n");
				//子进程退出，成为僵尸进程
				exit(0);
			}
			else
			{
				//父进程休眠20s继续创建子进程
				sleep(20);
				continue;
			}
		}
		return 0;
	}

	static void sig_child(int signo)
	{
		pid_t pid;
		int stat;

		//处理僵尸进程
		while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		{
			printf("child %d terminated.\n", pid);
		}
	}

	int handle_sig()
	{
		pid_t pid;
		//创建捕捉子进程退出信号
		signal(SIGCHLD, sig_child);
		pid = fork();
		if (pid < 0)
		{
			perror("fork error:");
			exit(1);
		}
		else if (pid == 0)
		{
			printf("I am child process,pid id %d.I am exiting.\n", getpid());
			exit(0);
		}
		printf("I am father process.I will sleep two seconds\n");
		//等待子进程先退出
		sleep(2);
		//输出进程信息
		system("ps -o pid,ppid,state,tty,command");
		printf("father process is exiting.\n");
		return 0;
	}

	int fork_double()
	{
		pid_t  pid;
		//创建第一个子进程
		pid = fork();
		if (pid < 0)
		{
			perror("fork error:");
			exit(1);
		}
		//第一个子进程
		else if (pid == 0)
		{
			//子进程再创建子进程
			printf("I am the first child process.pid:%d\tppid:%d\n", getpid(), getppid());
			pid = fork();
			if (pid < 0)
			{
				perror("fork error:");
				exit(1);
			}
			//第一个子进程退出
			else if (pid >0)
			{
				printf("first procee is exited.\n");
				exit(0);
			}
			//第二个子进程
			//睡眠3s保证第一个子进程退出，这样第二个子进程的父亲就是init进程里
			sleep(3);
			printf("I am the second child process.pid: %d\tppid:%d\n", getpid(), getppid());
			exit(0);
		}
		//父进程处理第一个子进程退出
		if (waitpid(pid, NULL, 0) != pid)
		{
			perror("waitepid error:");
			exit(1);
		}
		exit(0);
		return 0;
	}
}
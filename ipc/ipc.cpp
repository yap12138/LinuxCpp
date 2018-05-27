#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>    // O_WRONLY
#include <sys/stat.h>
#include <ctime>
#include <errno.h>

namespace hogason {
	int ipc_pipe()
	{
		int fd[2]; //文件描述符
		pid_t pid;
		char buff[20];

		if (pipe(fd) < 0) // 创建管道
		{
			printf("Create Pipe Error!\n");
			return -1;
		}

		if ((pid = fork()) < 0) // 创建子进程
		{
			printf("Fork Error!\n");
			return -2;
		}
		else if (pid > 0) // 父进程
		{
			close(fd[0]); // 关闭读端
			write(fd[1], "hello my child!\n", 17);
			printf("parent process: sent ok.\n");
		}
		else
		{
			close(fd[1]);
			read(fd[0], buff, 20);
			printf("child process: %s", buff);
			//system("ps -o pid,ppid,state,tty,command");
		}
		return 0;
	}

	int ipc_write_fifo() 
	{
		int fd;
		int n, i;
		char buf[1024];
		time_t tp;

		printf("I am %d process.\n", getpid()); // 说明进程ID

		if ((fd = open("fifo1", O_WRONLY)) < 0) // 以写打开一个FIFO 
		{
			perror("Open FIFO Failed");
			exit(1);
		}

		for (i = 0; i < 10; ++i)
		{
			time(&tp);  // 取系统当前时间
			n = sprintf(buf, "Process %d's time is %s", getpid(), ctime(&tp));
			printf("Send message: %s", buf); // 打印
			if (write(fd, buf, n + 1) < 0)  // 写入到FIFO中
			{
				perror("Write FIFO Failed");
				close(fd);
				exit(1);
			}
			sleep(1);  // 休眠1秒
		}

		close(fd);  // 关闭FIFO文件
		return 0;
	}

	int ipc_read_fifo() 
	{
		int fd;
		int len;
		char buf[1024];

		if (mkfifo("fifo1", 0666) < 0 && errno != EEXIST) // 创建FIFO管道
			perror("Create FIFO Failed");

		if ((fd = open("fifo1", O_RDONLY)) < 0)  // 以读打开FIFO
		{
			perror("Open FIFO Failed");
			exit(1);
		}

		while ((len = read(fd, buf, 1024)) > 0) // 读取FIFO管道
			printf("Read message: %s", buf);

		close(fd);  // 关闭FIFO文件
		return 0;
	}
}
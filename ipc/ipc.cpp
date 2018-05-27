#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>    // O_WRONLY
#include <sys/stat.h>
#include <sys/msg.h>
#include <ctime>
#include <errno.h>
#include <cstring>

namespace hogason 
{
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

	namespace sig
	{
#include <signal.h>
#include <sys/wait.h>
		void sig_send()
		{
			pid_t pid;
			int ret;
			pid = fork();
			if (pid < 0)
			{
				perror("创建子进程失败\n");
			}
			else if (pid == 0)
			{
				raise(SIGSTOP);
				exit(0);
			}
			else
			{
				printf("子进程的PID：%d\n", pid);
				if (waitpid(pid, NULL, WNOHANG) == 0)
				{
					if ((ret = kill(pid, SIGKILL)) == 0)
					{
						printf("用KILL函数的返回值为：%d，发出的SIGKILL信号结束的进程号为：%d\n", ret, pid);
					}
					else
					{
						perror("KILL子进程失败\n");
					}
				}
			}
		}
	}

#define MSG_FILE "/home/hogason/msg_file"
	// 消息结构
	struct msg_form {
		long mtype;
		char mident[128];
		char mtext[128];
	};
	void ipc_msg_server()
	{
		int msqid;
		key_t key;
		msg_form msg;
		
		// 获取key值
		if ((key = ftok(MSG_FILE, 'z')) < 0)
		{
			perror("ftok error");
			exit(1);
		}

		// 打印key值
		printf("Message Queue - Server key is: %d.\n", key);

		// 创建消息队列
		if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
		{
			perror("msgget error");
			exit(1);
		}
		// 打印消息队列ID及进程ID
		printf("My msqid is: %d.\n", msqid);
		printf("My pid is: %d.\n", getpid());

		// 循环读取消息
		while (true)
		{
			msgrcv(msqid, &msg, 256, 888, 0);// 返回类型为888的第一个消息
			if (strcmp("q", msg.mtext) == 0)
			{
				printf("msq del. exit.\n");
				if (msgctl(msqid, IPC_RMID, NULL) < 0)
				{
					perror("msgctl error");
					exit(1);
				}
				else
				{
					break;
				}
			}
			printf("Server: receive msg.mtype is: %d.\n", msg.mtype);
			printf("Server: receive msg.mtype is: %s.\n", msg.mident);
			printf("Server: receive msg.mtext is: %s.\n", msg.mtext);

			msg.mtype = 999; // 客户端接收的消息类型
			sprintf(msg.mtext, "hello, I'm server %d", getpid());
			msgsnd(msqid, &msg, 256, 0);
		}
	}

	void ipc_msg_client()
	{
		int msqid;
		key_t key;
		msg_form msg;

		// 获取key值
		if ((key = ftok(MSG_FILE, 'z')) < 0)
		{
			perror("ftok error");
			exit(1);
		}

		// 打印key值
		printf("Message Queue - Client key is: %d.\n", key);

		// 打开消息队列
		if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
		{
			perror("msgget error");
			exit(1);
		}

		// 打印消息队列ID及进程ID
		printf("My msqid is: %d.\n", msqid);
		printf("My pid is: %d.\n", getpid());

		while (true)
		{
			// 添加消息，类型为888
			msg.mtype = 888;
			snprintf(msg.mident, sizeof msg.mident, "I'm client %d", getpid());
			printf("Enter msg (q to quit): ");
			gets(msg.mtext);
			msgsnd(msqid, &msg, 256, 0);
			if (strcmp("q", msg.mtext) == 0)
			{
				break;
			}

			// 读取类型为999的消息
			msgrcv(msqid, &msg, 256, 999, 0);
			printf("Client: receive msg.mtype is: %d.\n", msg.mtype);
			printf("Client: receive msg.mtext is: %s.\n", msg.mtext);
		}
		
	}
}
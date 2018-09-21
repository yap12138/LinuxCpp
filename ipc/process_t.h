#pragma once


namespace hogason {
	/*孤儿进程示例*/
	int orphan();

	/*僵尸进程示例*/
	int zombie();

	/*循环创建僵尸进程，堆积僵尸进程。 main应采用 & 守护进程调用*/
	int zombieloop();

	/*用handler处理子进程结束时的SIGCHILD信号，避免产生僵尸进程*/
	int handle_sig();

	/*利用两次fork，第一个子进程退出后第二子进程成为孤儿进程，由init进程处理*/
	int fork_double();
}

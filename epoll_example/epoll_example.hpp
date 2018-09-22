#ifndef _HOGASON_EPOLL_SERVER_HPP
#define _HOGASON_EPOLL_SERVER_HPP

#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>

namespace hogason
{
	namespace epoll
	{
		using ep_t = int;

		/*
		 * 1.当用户输入一组字符，这组字符被送入buffer，字符停留在buffer中，又因为buffer由空变为不空，所以ET返回读就绪，输出”welcome to epoll's world！”。
		 * 2.之后程序再次执行epoll_wait，此时虽然buffer中有内容可读，但是根据我们上节的分析，ET并不返回就绪，导致epoll_wait阻塞。
		 *	（底层原因是ET下就绪fd的epitem只被放入rdlist一次）。
		 * 3.用户再次输入一组字符，导致buffer中的内容增多，根据我们上节的分析这将导致fd状态的改变，是对应的epitem再次加入rdlist，
		 *	从而使epoll_wait返回读就绪，再次输出“Welcome to epoll's world！”。
		 */
		void et_stdin()
		{
			using std::cout;
			using std::endl;

			ep_t epfd;
			epoll_event ev;
			epoll_event events[10];	//ev用于注册事件，数组用于返回要处理的事件

			epfd = epoll_create(10);
			ev.data.fd = STDIN_FILENO;
			ev.events = EPOLLIN | EPOLLET; //监听读状态同时设置ET模式
			epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);	//注册epoll事件

			int nfds;
			while (true)
			{
				nfds = epoll_wait(epfd, events, 10, -1);
				for (int i = 0; i < nfds; ++i)
				{
					if (events[i].data.fd == STDIN_FILENO)
					{
						cout << "Trigger event." << endl;
					}
				}
			}
		}

		/*
		 * 程序依然使用ET，但是每次读就绪后都主动的再次MOD IN事件，我们发现程序再次出现死循环，也就是每次返回读就绪。
		 * 但是注意，如果我们将MOD改为ADD，将不会产生任何影响。
		 * 别忘了每次ADD一个描述符都会在epitem组成的红黑树中添加一个项，我们之前已经ADD过一次，再次ADD将阻止添加，所以在次调用ADD IN事件不会有任何影响。
		 */
		void et_stdin_reset()
		{
			using std::cout;
			using std::endl;

			ep_t epfd;
			epoll_event ev;
			epoll_event events[10];	//ev用于注册事件，数组用于返回要处理的事件

			epfd = epoll_create(10);
			ev.data.fd = STDIN_FILENO;
			ev.events = EPOLLIN | EPOLLET; //监听读状态同时设置ET模式
			epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);	//注册epoll事件

			int nfds;
			while (true)
			{
				nfds = epoll_wait(epfd, events, 10, -1);
				for (int i = 0; i < nfds; ++i)
				{
					if (events[i].data.fd == STDIN_FILENO)
					{
						cout << "Trigger event." << endl;
						ev.data.fd = STDIN_FILENO;
						ev.events = EPOLLIN | EPOLLET;                        //设置ET模式
						epoll_ctl(epfd, EPOLL_CTL_MOD, STDIN_FILENO, &ev);    //重置epoll事件（ADD无效）

					}
				}
			}
		}

		/*
		 * 程序陷入死循环，因为用户输入任意数据后，数据被送入buffer且没有被读出，所以LT模式下每次epoll_wait都认为buffer可读返回读就绪。
		 * 导致每次都会输出”welcome to epoll's world！”。
		 */
		void lt_stdin()
		{
			using std::cout;
			using std::endl;

			ep_t epfd;
			epoll_event ev;
			epoll_event events[10];	//ev用于注册事件，数组用于返回要处理的事件

			epfd = epoll_create(10);
			ev.data.fd = STDIN_FILENO;
			ev.events = EPOLLIN;	//默认使用LT模式
			epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);	//注册epoll事件

			int nfds;
			while (true)
			{
				nfds = epoll_wait(epfd, events, 10, -1);
				for (int i = 0; i < nfds; ++i)
				{
					if (events[i].data.fd == STDIN_FILENO)
					{
						cout << "Trigger event." << endl;
					}
				}
			}
		}

		/*
		 * 本程序依然使用LT模式，但是每次epoll_wait返回读就绪的时候我们都将buffer（缓冲）中的内容read出来，所以导致buffer再次清空，下次调用epoll_wait就会阻塞。
		 * 所以能够实现我们所想要的功能——当用户从控制台有任何输入操作时，输出”welcome to epoll's world！”
		 */
		void lt_stdin_rd()
		{
			using std::cout;
			using std::endl;

			ep_t epfd;
			epoll_event ev;
			epoll_event events[10];	//ev用于注册事件，数组用于返回要处理的事件

			epfd = epoll_create(10);
			ev.data.fd = STDIN_FILENO;
			ev.events = EPOLLIN;	//默认使用LT模式
			epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);	//注册epoll事件

			int nfds;
			while (true)
			{
				nfds = epoll_wait(epfd, events, 10, -1);
				for (int i = 0; i < nfds; ++i)
				{
					if (events[i].data.fd == STDIN_FILENO)
					{
						char buf[1024] = { 0 };
						read(STDIN_FILENO, buf, sizeof(buf));
						cout << "Trigger event." << endl;
					}
				}
			}
		}

		/*
		 * 1.首先初始buffer为空，buffer中有空间可写，这时无论是ET还是LT都会将对应的epitem加入rdlist，导致epoll_wait就返回写就绪。
		 * 2.程序想标准输出输出”Trigger stdout event.”和换行符，因为标准输出为控制台的时候缓冲是“行缓冲”,所以换行符导致buffer中的内容清空，
		 *	这就对应第二节中ET模式下写就绪的第二种情况——当有旧数据被发送走时，即buffer中待写的内容变少得时候会触发fd状态的改变。
		 *	所以下次epoll_wait会返回写就绪。如此循环往复。
		 */
		void et_stdout()
		{
			using std::cout;
			using std::endl;

			ep_t epfd;
			epoll_event ev;
			epoll_event events[10];	//ev用于注册事件，数组用于返回要处理的事件

			epfd = epoll_create(10);
			ev.data.fd = STDOUT_FILENO;
			ev.events = EPOLLOUT | EPOLLET; //监听读状态同时设置ET模式
			epoll_ctl(epfd, EPOLL_CTL_ADD, STDOUT_FILENO, &ev);	//注册epoll事件

			int nfds;
			while (true)
			{
				nfds = epoll_wait(epfd, events, 10, -1);
				for (int i = 0; i < nfds; ++i)
				{
					if (events[i].data.fd == STDOUT_FILENO)
					{
						cout << "Trigger stdout event." << endl;
					}
				}
			}
		}

		/*
		 * 与程序et_stdout相比，此程序只是将输出语句的换行移除。我们看到程序成挂起状态。
		 * 因为第一次epoll_wait返回写就绪后，程序向标准输出的buffer中写入“Trigger stdout event.”，
		 * 但是因为没有输出换行，所以buffer中的内容一直存在，下次epoll_wait的时候，虽然有写空间但是ET模式下不再返回写就绪。
		 * 回忆第一节关于ET的实现，这种情况原因就是第一次buffer为空，导致epitem加入rdlist，返回一次就绪后移除此epitem，之后虽然buffer仍然可写，
		 * 但是由于对应epitem已经不再rdlist中，就不会对其就绪fd的events的在检测了。
		 */
		void et_stdout_rm_crlf()
		{
			using std::cout;
			using std::endl;

			ep_t epfd;
			epoll_event ev;
			epoll_event events[10];	//ev用于注册事件，数组用于返回要处理的事件

			epfd = epoll_create(10);
			ev.data.fd = STDOUT_FILENO;
			ev.events = EPOLLOUT | EPOLLET; //监听读状态同时设置ET模式
			epoll_ctl(epfd, EPOLL_CTL_ADD, STDOUT_FILENO, &ev);	//注册epoll事件

			int nfds;
			while (true)
			{
				nfds = epoll_wait(epfd, events, 10, -1);
				for (int i = 0; i < nfds; ++i)
				{
					if (events[i].data.fd == STDOUT_FILENO)
					{
						cout << "Trigger stdout event.";
					}
				}
			}
		}
	}
}

#endif // _HOGASON_EPOLL_SERVER_HPP

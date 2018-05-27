#pragma once

namespace hogason 
{
	/*无名管道*/
	int ipc_pipe();

	/*命名管道 write端*/
	int ipc_write_fifo();

	/*命名管道 read端*/
	int ipc_read_fifo();

	namespace sig
	{
		/*信号发送*/
		void sig_send();
	}

	void ipc_msg_server();

	void ipc_msg_client();
}

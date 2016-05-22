/*
 * main.c
 *
 *  Created on: May 15, 2016
 *      Author: ahnmh
 */

#include "pthread_func.h"

int main(int argc, char *argv[])
{
//	pthread_introduce();
//	pthread_sync();
//	pthread_sync_error_checking_mutex();
//	pthread_cond_var();
//	pthread_safe();
//	pthread_cancellation();
//	ipc_pipe(argc, argv);
//	ipc_pipe_stdinout();
//	ipc_pipe_popen();
	ipc_fifo_client();

	return 0;
}

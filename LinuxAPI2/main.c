/*
 * main.c
 *
 *  Created on: May 15, 2016
 *      Author: ahnmh
 */

#include "pthread_func.h"
#include "ipc_systemv_func.h"
#include "mmap_func.h"
#include "ipc_posix_func.h"

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
//	ipc_fifo_server();
//	ipc_systemv_msg_server();
//	ipc_systemv_sem();
//	ipc_systemv_shm_writer();
//	mmap_file_privated(argc, argv);
//	mmap_file_shared(argc, argv);;
//	mmap_anonymous_shared();
//	mmap_ops();
//	ipc_posix_mq_write();
	ipc_posix_mq_read();

	return 0;
}

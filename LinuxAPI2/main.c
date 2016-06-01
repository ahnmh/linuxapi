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
//	mmap_anonymous_shared();m
//	mmap_ops();

//	if(argc != 2)
//		errExit("Not enough argument\n");
//
//	switch(getopt(argc, argv, "cwrntd")) {
//	case 'c':
//		ipc_posix_mq_create();
//		break;
//	case 'w':
//		ipc_posix_mq_write();
//		break;
//	case 'r':
//		ipc_posix_mq_read();
//		break;
//	case 'n':
//		ipc_posix_mq_read_by_signal();
//		break;
//	case 't':
//		ipc_posix_mq_read_by_thread();
//		break;
//	case 'd':
//		ipc_posix_mq_destroy();
//		break;
//	}

//	if(argc != 2)
//		errExit("Not enough argument\n");
//
//	switch(getopt(argc, argv, "card")) {
//	case 'c':
//		ipc_posix_sem_create();
//		break;
//	case 'a':
//		ipc_posix_sem_acquire();
//		break;
//	case 'r':
//		ipc_posix_sem_release();
//		break;
//	case 'd':
//		ipc_posix_sem_destroy();
//		break;
//	}

//	ipc_posix_unnamed_sem();

	if(argc != 2)
		errExit("Not enough argument\n");

	switch(getopt(argc, argv, "wrd")) {
	case 'w':
		ipc_posix_shm_write();
		break;
	case 'r':
		ipc_posix_shm_read();
		break;
	case 'd':
		ipc_posix_shm_destroy();
		break;
	}

	return 0;
}

//test


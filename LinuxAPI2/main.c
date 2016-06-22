/*
 * main.c
 *
 *  Created on: May 15, 2016
 *      Author: ahnmh
 */

//#include "pthread_func.h"
//#include "ipc_systemv_func.h"
//#include "mmap_func.h"
//#include "ipc_posix_func.h"
//#include "file_lock.h"
//#include "socket_unix_domain.h"
#include "socket_inet_domain.h"
#include "socket_echo.h"
#include "alt_io_sample.h"

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

//	switch(getopt(argc, argv, "card")) {
//	case 'c':
//		ipc_posix_sem_create();
//		break;
//	case 'a':
//		ipc_posix_sem_acquire();
//		break;
//	case 'r':
//		break;
//	case 'd':
//		ipc_posix_sem_destroy();
//		break;
//		ipc_posix_sem_release();
//	}

//	ipc_posix_unnamed_sem();

//	switch(getopt(argc, argv, "wrd")) {
//	case 'w':
//		ipc_posix_shm_write();
//		break;
//	case 'r':
//		ipc_posix_shm_read();
//		break;
//	case 'd':
//		ipc_posix_shm_destroy();
//		break;
//	}

//	file_lock_flock(argc, argv);
//	file_lock_region(argc, argv);

//	switch(getopt(argc, argv, "sc")) {
//	case 's':
////		socket_unix_domain_stream_server();
//		socket_unix_domain_datagram_server();
//		break;
//	case 'c':
////		socket_unix_domain_stream_client();
//		socket_unix_domain_datagram_client();
//		break;
//	}

//	socket_pair_sample(argc, argv);

//	switch(getopt(argc, argv, "sc")) {
//	case 's':
////		socket_inet_domain_stream_server();
////		socket_inet_domain_datagram_server();
//		socket_inet_domain_stream_server_v2();
//		break;
//	case 'c':
////		socket_inet_domain_stream_client();
////		socket_inet_domain_datagram_client();
//		socket_inet_domain_stream_client_v2();
//		break;
//	}

//	gethostbyname_example(argc, argv);
//	getservbyname_example(argc, argv);

//	switch(getopt(argc, argv, "sc")) {
//	case 's':
//		socket_echo_server();
//		break;
//	case 'c':
//		socket_echo_client();
//		break;
//	}

//	alt_io_select();
//	alt_io_poll();
	alt_io_signal();
	alt_io_epoll();
	return 0;
}

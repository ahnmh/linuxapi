/*
 * main.c
 *
 *  Created on: Jun 24, 2016
 *      Author: ahnmh-vw
 */

#include "fileio.h"
#include "process_manage.h"

int main(int argc, char *argv[])
{
//	fileio_read_write(argc, argv);

//	select_sample();
//	pselect_sample();
//	poll_sample();
//	epoll_sample();

//	fileio_stdio_read();
//	fileio_stdio_write();
//	fileio_stdio_write_lock();

//	fileio_readv_writev();

//	mmap_sample(argc, argv);

//	fileio_aio_write();
//	get_inode();
//	get_physical_block_number();

//	fork_exec_wait();
	system_sample();

	return 0;
}

/*
 * main.c
 *
 *  Created on: Jun 24, 2016
 *      Author: ahnmh-vw
 */

#include "fileio.h"

int main(int argc, char *argv[])
{
//	fileio_read_write(argc, argv);

//	select_sample();
//	pselect_sample();
//	poll_sample();
	epoll_sample();

//	fileio_stdio_read();
//	fileio_stdio_write();
//	fileio_stdio_write_lock();

//	fileio_readv_writev();

	return 0;
}

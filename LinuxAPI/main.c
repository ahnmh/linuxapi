/*
 * main.c
 *
 *  Created on: Mar 31, 2016
 *      Author: root
 */

#include "file_io.h"

int main()
{
	int ret = EXIT_SUCCESS;
	printf("[PID: %d]\n", getpid());

//	ret = file_io();
//	ret = file_fcntl();
//	ret = file_scatter_gather();
//	ret = file_ftruncate();
//	ret = file_opentempfile();
	ret = file_pread_pwrite();


	return 0;
}

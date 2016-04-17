/*
 * main.c
 *
 *  Created on: Mar 31, 2016
 *      Author: root
 */

#include "file_io.h"
#include "process_env.h"
#include "memory_alloc.h"
#include "time_func.h"
#include "user_group.h"
#include "get_limit.h"
#include "file_buffering.h"
#include "file_mount.h"
#include "file_stat.h"
#include "directory_link.h"

int main()
{
//	int ret = EXIT_SUCCESS;

/*
	ret = file_io();
	ret = file_fcntl();
	ret = file_scatter_gather();
	ret = file_ftruncate();
	ret = file_opentempfile();
	ret = file_pread_pwrite();
*/

//	ret = get_enviorn();

//	memory_alloc();

//	user_group();

//	time_related_func();

//	get_limit();

//	file_buffering();

//	file_mount();

//	file_stat();

	directory_link();

	return 0;
}

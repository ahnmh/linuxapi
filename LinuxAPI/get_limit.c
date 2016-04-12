/*
 * get_limit.c
 *
 *  Created on: Apr 11, 2016
 *      Author: ahnmh
 */
#include <stdio.h>
#include "get_limit.h"

void get_limit()
{
	int arg_max = sysconf(_SC_ARG_MAX);
	printf("argument max: %d\n", arg_max);

	int fd_max = sysconf(_SC_OPEN_MAX);
	printf("fd max: %d\n", fd_max);

	int page_size = sysconf(_SC_PAGESIZE);
	printf("page size: %d\n", page_size);

	int sig_max = sysconf(_SC_RTSIG_MAX);
	printf("signal max: %d\n", sig_max);

	int stream_max = sysconf(_SC_STREAM_MAX);
	printf("stream max: %d\n", stream_max);

	int name_max = pathconf("/", _PC_NAME_MAX);
	printf("filename max: %d\n", name_max);

	int path_max = pathconf("/", _PC_PATH_MAX);
	printf("filepath max: %d\n", path_max);


/* system options
 * ret :
 * 200809(SUSv4) = support
 * 0 = can be supported. you should check it at your program once again via sysconf()
 * -1 = does not support
 */

	int async_io = sysconf(_SC_ASYNCHRONOUS_IO);
	printf("Does system support async. IO?: %d\n", async_io);

	int chown_restrict = pathconf("/", _PC_CHOWN_RESTRICTED);
	printf("Does only root can change UID and GID of file?: %d\n", chown_restrict);

	int job_control = sysconf(_SC_JOB_CONTROL);
	printf("Does system support job control?: %d\n", job_control);

	int priority_scehdule = sysconf(_SC_PRIORITY_SCHEDULING);
	printf("Does system support priority scheduling?: %d\n", priority_scehdule);

	int posix_semaphore = sysconf(_SC_SEMAPHORES);
	printf("Does system support posix semaphore?: %d\n", posix_semaphore);

	int shared_memory = sysconf(_SC_SHARED_MEMORY_OBJECTS);
	printf("Does system support posix shared memory object?: %d\n", shared_memory);

	int thread = sysconf(_SC_THREADS);
	printf("Does system support posix thread?: %d\n", thread);


	struct utsname utsbuf;
	int result = uname(&utsbuf);
	printf("System name: %s\n", utsbuf.sysname);
	printf("Node name: %s\n", utsbuf.nodename);
	printf("Kernel version: %s\n", utsbuf.release);
	printf("System version: %s\n", utsbuf.version);
	printf("System architecture: %s\n", utsbuf.machine);
	printf("NIS domain name: %s\n", utsbuf.__domainname);

}

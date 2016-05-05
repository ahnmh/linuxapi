/*
 * process_resources.c
 *
 *  Created on: May 6, 2016
 *      Author: ahnmh
 */

#include <sys/resource.h>

#include "process_func.h"
#include "tlpi_hdr.h"

void process_resources()
{

	// 아래 코드로 리소스를 사용하게끔 유도함.
	long i;
	long result = 0;
	for(i = 0; i < 5000000; i++)
		result += i;

	struct rusage res;
	// 자기 자신에 대해 측정함.
	if(getrusage(RUSAGE_SELF, &res) == -1)
		errExit("getrusage()");

	printf("user time: %d(s), %d(us)\n", res.ru_utime.tv_sec, res.ru_utime.tv_usec);
	printf("system time: %d(s), %d(us)\n", res.ru_stime.tv_sec, res.ru_stime.tv_usec);
	printf("minor page fault:%ld\n", res.ru_minflt);
	printf("major page fault:%ld\n", res.ru_majflt);
	printf("swap:%ld\n", res.ru_nswap);
	printf("block input operations:%ld\n", res.ru_inblock);
	printf("block output operations:%ld\n", res.ru_oublock);
	printf("voluntary context switching:%ld\n", res.ru_nvcsw);
	printf("involuntary context switching:%ld\n", res.ru_nivcsw);

	// 자원한도 = 구한 값이 -1(RLIM_INFINITY)인 경우 unlimited를 의미한다.
	struct rlimit limit;
	if(getrlimit(RLIMIT_AS, &limit) == -1)
		errExit("getrlimit");
	printf("process virtual memory size(byte): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_CORE, &limit) == -1)
		errExit("getrlimit");
	printf("core file size(byte): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_CPU, &limit) == -1)
		errExit("getrlimit");
	printf("cpu time(s): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_DATA, &limit) == -1)
		errExit("getrlimit");
	printf("data segment size(byte): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_FSIZE, &limit) == -1)
		errExit("getrlimit");
	printf("file size(byte): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_MEMLOCK, &limit) == -1)
		errExit("getrlimit");
	printf("memory lock(byte): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_NICE, &limit) == -1)
		errExit("getrlimit");
	printf("available nice: soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_NOFILE, &limit) == -1)
		errExit("getrlimit");
	printf("file descriptor number: soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_RTPRIO, &limit) == -1)
		errExit("getrlimit");
	printf("realtime scheduling priority: soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_RTTIME, &limit) == -1)
		errExit("getrlimit");
	printf("realtime CPU time(us): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);

	if(getrlimit(RLIMIT_STACK, &limit) == -1)
		errExit("getrlimit");
	printf("stack segment size(byte): soft = %d, hard = %d\n", limit.rlim_cur, limit.rlim_max);
}

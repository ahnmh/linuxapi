/*
 * timer_clock.c
 *
 *  Created on: Apr 25, 2016
 *      Author: ahnmh
 */
#define _POSIX_C_SOURCE 199309
#define _XOPEN_SOURE 600

#include "timer_func.h"
#include "tlpi_hdr.h"
#include <stdio.h>
#include <signal.h>
#include <time.h>

void timer_clock_gettime()
{
/*	POSIX clock
 *  - rt 라이브러리 필요*/

	int ret;
	struct timespec ts;
	// 실제 시간
	ret = clock_gettime(CLOCK_REALTIME, &ts);
	printf("sec: %ld, nanosec: %ld\n", ts.tv_sec, ts.tv_nsec);
	// 시스템 시작 이후 경과 시간
	ret = clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("sec: %ld, nanosec: %ld\n", ts.tv_sec, ts.tv_nsec);
	// 프로세스가 소비한 kernel + user 시간
	ret = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	printf("sec: %ld, nanosec: %ld\n", ts.tv_sec, ts.tv_nsec);
	// 현재 스레드가 소비한 kernel + user 시간
	ret = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
	printf("sec: %ld, nanosec: %ld\n", ts.tv_sec, ts.tv_nsec);

	// 클록의 정밀도 확인(= 1 nanosecond)
	struct timespec res;
	ret = clock_getres(CLOCK_REALTIME, &res);
	printf("sec: %ld, nanosec: %ld\n", res.tv_sec, res.tv_nsec);

	// 현재 프로세스의 clockid를 구해서 프로세스의 CPU 사용 시간 확인
	// CLOCK_PROCESS_CPUTIME_ID와 뭐가 다른지 모르겠음..
	clockid_t clockid;
	ret = clock_getcpuclockid(getpid(), &clockid);
	printf("clockid: %d\n", clockid);
	ret = clock_gettime(clockid, &ts);
	printf("sec: %ld, nanosec: %ld\n", ts.tv_sec, ts.tv_nsec);

}

void timer_clock_nanosleep()
{
	int ret;
	struct timespec ts;
	// 현재 시간
	ret = clock_gettime(CLOCK_REALTIME, &ts);

	printf("Process will be sleep...\n");

/*
	정확히 현재 시간 + 5초 후에 sleep한다.
	만일, 현재 시간을 추출하고 원하는 타겟 시간까지의 차이를 계산해 상대 수면 시간을 취하려 한다면
	해당 작업 도중에 프로세스가 선점될 가능성이 있고 따라서 결과적으로 원하는 시간보다 더 길게 수면을 취하게 된다.
*/
	ts.tv_sec += 5;

	/* 이 부분에 어떤 작업이 와서 지연이 되어도, 위에서 구한 현재시간 + 5초 이후까지만 sleep이 동작한다 */

	ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
	if(ret != 0) {
		if(ret == EINTR)
			printf("Interrupted by signal\n");
		else
			errExit("clock_nanosleep()");
	}


}

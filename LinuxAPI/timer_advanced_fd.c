/*
 * timer_advanced_fd.c
 *
 *  Created on: Apr 26, 2016
 *      Author: ahnmh
 */
#include <sys/timerfd.h>
#include <time.h>
#include <stdint.h>

#include "timer_func.h"
#include "tlpi_hdr.h"

static int max_expire_cnt = 100;

void timer_advanced_fd()
{
	// 타이머 설정
	struct itimerspec ts;
	ts.it_value.tv_sec = 1; // 타이머의 최초 만료 시간
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 1; // 타이머의 주기적 만료 시간
	ts.it_interval.tv_nsec = 0;

	int tfd = timerfd_create(CLOCK_REALTIME, 0);
	if(tfd == -1)
		errExit("timerfd_create()");

	if(timerfd_settime(tfd, 0, &ts, NULL) == -1)
		errExit("timerfd_settime()");

	int num_expire_cnt;
	uint64_t total_expire_cnt;
	ssize_t s;

	for(total_expire_cnt = 0; total_expire_cnt < max_expire_cnt;) {
		s = read(tfd, &num_expire_cnt, sizeof(uint64_t));
		if(s != sizeof(uint64_t))
			errExit("read()");

		printf("total_expire_cnt = %lu\n", total_expire_cnt);
		total_expire_cnt += num_expire_cnt;
	}

}

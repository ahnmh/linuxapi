/*
 * timer_sync_expire.c
 *
 *  Created on: Apr 24, 2016
 *      Author: ahnmh
 */

#include "timer_func.h"
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h> // alarm
#include "tlpi_hdr.h"
#define BUF_SIZE 4096


static void on_timer(int sig)
{
	printf("Timer has been expired!\n");
}

void timer_sync_expire()
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = on_timer;
	if(sigaction(SIGALRM, &sa, NULL) == -1)
		errExit("sigaction\n");

	struct itimerval tv;
	tv.it_value.tv_sec = 5; // 타이머의 최초 만료 시간
	tv.it_value.tv_usec = 0;
	tv.it_interval.tv_sec = 2; // 타이머의 주기적 만료 시간
	tv.it_interval.tv_usec = 0;

	// 실시간 타이머 설정
	if(setitimer(ITIMER_REAL, &tv, NULL) == -1)
		errExit("setitimer()");

	// 시스템콜
	int cnt;
	char buf[BUF_SIZE];
	cnt = read(0, buf, BUF_SIZE); // stdin으로부터 읽는다.

	// 타이머를 종료하는 또 다른 방법(alarm과 setitimer는 같은 타이머를 공유하므로)
	alarm(0);

	if(cnt == -1 && errno == EINTR)
		errExit("read() has been failed by timer.");

	buf[cnt] = '\0';
	printf("input is \"%s\"\n", buf);
}


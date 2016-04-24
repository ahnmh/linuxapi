/*
 * timer_func.c
 *
 *  Created on: Apr 24, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE

#include "timer_func.h"
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h> // alarm
#include "tlpi_hdr.h"

static volatile sig_atomic_t setalarm = 0;

static void on_timer(int sig)
{
	printf("on_timer\n");
	setalarm = 1;
}

void timer_func()
{
	// 실시간 타이머(ITMER_REAL)가 만료되면 SIGALRM 시그널을 받는다.
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = on_timer;
	if(sigaction(SIGALRM, &sa, NULL) == -1)
		errExit("sigaction\n");

	struct itimerval tv;
	tv.it_value.tv_sec = 3; // 타이머의 최초 만료 시간
	tv.it_value.tv_usec = 0;
	tv.it_interval.tv_sec = 2; // 타이머의 주기적 만료 시간
	tv.it_interval.tv_usec = 0;

	// 실시간 타이머 설정
	if(setitimer(ITIMER_REAL, &tv, NULL) == -1)
		errExit("setitimer()");

	int loop = 0;
	struct itimerval cur_tv;
	while(1) {
		if(setalarm == 1) {
			printf("Timer expired!\n");
			setalarm = 0;

			if(++loop < 5) {
				printf("loop: %d\n", loop);
			}
			else {
				// 5회 반복후 타이머 종료
				// it_value를 모두 0으로 설정하여 호출하면 타이머가 종료됨
				tv.it_value.tv_sec = 0;
				tv.it_value.tv_usec = 0;
				if(setitimer(ITIMER_REAL, &tv, NULL) == -1)
					errMsg("setitimer()");
				break;
			}

		}

		// 현재 타이머의 잔여 시간을 얻는다.
		if(getitimer(ITIMER_REAL, &cur_tv) == -1)
			errMsg("getitimer()");
		printf("Remain time: %d\n", cur_tv.it_value.tv_sec);

		sleep(1);
	}

	// 1회용 타이머 - 한번만 사용되며 3초 후 SIGALRM 시그널을 수신하게 됨.
	alarm(3);
	pause(); // alarm에 의한 SIGALRM 시그널 대기

/*
	 setitimer와 alarm은 프로세스별로 동일한 실시간 타이머를 공유하므로
	 둘 중 하나로 타이머를 설정하면 이전에 설정한 타이머를 변경함을 의미한다.
*/

}


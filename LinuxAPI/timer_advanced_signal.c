/*
 * timer_advanced.c
 *
 *  Created on: Apr 26, 2016
 *      Author: ahnmh
 */
#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <time.h>

#include "timer_func.h"
#include "tlpi_hdr.h"

// timer_create(), timer_settime() -> rt 라이브러리가 필요

#define TIMER_SIG	SIGRTMAX

static void signal_timer_handler(int sig, siginfo_t *si, void *context)
{
	timer_t *ptid = si->si_value.sival_ptr;
	//si_code = -2인데, SI_TIMER를 의미한다.
	printf("si_code: %d, tid: %ld\n", si->si_code, (long)*ptid);
}

// 시그널을 사용한 타이머 통지
void timer_advanced_signal()
{
	struct sigaction sa;
	// 2개의 타이머
	timer_t tid[2];

	sigemptyset(&sa.sa_mask);
	// siginfo_t *si를 파라미터로 하는 확장 시그널 핸들러를 사용한다.
	sa.sa_flags = SA_SIGINFO;
	sa.sa_handler = signal_timer_handler;
	if(sigaction(TIMER_SIG, &sa, NULL) == -1)
		errExit("sigaction()");

	// 시그널을 이용하는 타이머 생성
	// 여기서는 하나의 시그널을 2개의 타이머에 연결했는데, 시그널과 타이머를 1:1로 매칭시켜서 동작할 수 있을 것임.
	struct sigevent sv;
	sv.sigev_notify = SIGEV_SIGNAL; // 통지 방식은 시그널
	sv.sigev_signo = TIMER_SIG; // 통지에 사용되는 시그널

	// 2개의 타이머 생성
	int i;
	for (i = 0; i < 2; i++) {
		sv.sigev_value.sival_ptr = &tid[i];
		if(timer_create(CLOCK_REALTIME, &sv, &tid[i]) == -1)
			errExit("timer_create()");
		printf("TImer Id: %ld\n", (long)tid[i]);
	}

	// 첫번째 타이머 설정
	struct itimerspec ts;
	ts.it_value.tv_sec = 3; // 타이머의 최초 만료 시간
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 2; // 타이머의 주기적 만료 시간
	ts.it_interval.tv_nsec = 0;
	if(timer_settime(tid[0], 0, &ts, NULL) == -1)
		errExit("timer_settime(): %ld", (long)tid[0]);

	// 두번째 타이머 설정
	ts.it_value.tv_sec = 5; // 타이머의 최초 만료 시간
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 3; // 타이머의 주기적 만료 시간
	ts.it_interval.tv_nsec = 0;
	if(timer_settime(tid[1], 0, &ts, NULL) == -1)
		errExit("timer_settime(): %ld", (long)tid[1]);


	// 타이머 시그널 대기
	while(1)
		pause();

}

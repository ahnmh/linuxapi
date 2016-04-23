/*
 * signal_suspend.c
 *
 *  Created on: Apr 23, 2016
 *      Author: ahnmh
 */
#include "signal_func.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include "tlpi_hdr.h"
#include <time.h>

// SIGQUIT 시그널을 받았는지 확인하는 함수. sig_atomic_t 타입을 사용해서 시그널 핸들러와 메인함수간 아토믹한 변경을 보장함.
static volatile sig_atomic_t sigquit = 0;

/*sigsuspend 함수는 아래 상황과 같은 시그널 프로그래밍에서 사용한다.
 * - 일시적으로 시그널을 블록하여, 코드 임계 영역의 실행을 보장한다.
 * - 코드 임계 영역의 실행이 완료되면 시그널을 블록 해제하고 시그널이 전달될 때까지 실행을 블록한다.
 * */

void signal_suspend_handler(int sig)
{
	printf("[signal_handler]signal = %d(%s)\n", sig, strsignal(sig));

	if(sig == SIGQUIT)
		sigquit = 1;
}

void signal_suspend()
{
	sigset_t originmask, blockmask;
	struct sigaction sa;

	// 시그널 전달을 블록한다.
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SIGINT);
	sigaddset(&blockmask, SIGQUIT);
	if(sigprocmask(SIG_BLOCK, &blockmask, &originmask) == -1)
		errExit("sigprocmask()");

	// SIGINT와 SIGQUIT의 시그널 핸들러를 설정한다.
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = signal_suspend_handler;
	if(sigaction(SIGINT, &sa, NULL) == -1)
		errExit("sigaction\n");
	if(sigaction(SIGQUIT, &sa, NULL) == -1)
		errExit("sigaction\n");

	time_t start;

	int loop = 0;
	while(!sigquit) {
		// 시그널에 의해 인터럽트 되지 말아야할 코드 임계 영역
		printf("loop count = %d\n", loop);
		sleep(4);
//		for(start = time(NULL); time(NULL) < start + 4;)
//			continue;
		loop++;

		// 임계 영역 수행을 완료한 후 sigsuspend 함수를 사용해서 수행을 중지(suspend)하고, 시그널 입력을 대기함.
		// suspend 이전에 시그널 입력이 있었거나 대기중 시그널 입력이 발생한 경우 시그널 핸들러를 호출하고 리턴됨.
		if(sigsuspend(&originmask) == -1 && errno != EINTR)
			errExit("sigsuspend()");

	}

	// 시그널 블록을 해제한다.
	if(sigprocmask(SIG_SETMASK, &originmask, NULL) == -1)
		errExit("sigprocmask()");

}

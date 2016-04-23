/*
 * signal_suspend_sync.c
 *
 *  Created on: Apr 23, 2016
 *      Author: ahnmh
 */

#include "signal_func.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include "tlpi_hdr.h"

void signal_suspend_sync()
{
	int sig;
	siginfo_t si;
	sigset_t allsig;

	// 모든 시그널에 대해 블록한다.
	sigfillset(&allsig);
	if(sigprocmask(SIG_SETMASK, &allsig, NULL) == -1)
		errExit("sigprocmask()");

	sleep(5);

	while(1) {
/*
		 시그널을 동기적으로 받기 위한 함수
		 시그널에 대해 동기적으로 처리하기 때문에 sigsuspend 함수와 달리 시그널 핸들러가 필요없다.
		 즉, sigaction에 의한 시그널 핸들러 등록이 필요없이 대기가 가능함
		 sigwaitinfo 함수를 호출하면 파라미터로 지정된 모든 시그널 집합 중 하나가 이미 대기중이거나 전달될 때까지 실행을 멈춘다.
		 sigaction 함수를 SA_SIGINFO 플래그와 함께 실행했을 때 시그널 핸들러로 siginfo_t 변수가 전달되는 것처럼,
		 sigwaitinfo 함수가 리턴될 때 이 값을 채워준다.
		 대기 시간 이후 타임아웃되도록 구현하려면 sigwaitinfo 대신 sigtimedout 함수를 사용할 것
*/
		sig = sigwaitinfo(&allsig, &si);
		if(sig == -1)
			errExit("sigwaitinfo()");

		if(sig == SIGQUIT | sig == SIGTERM)
			exit(EXIT_SUCCESS);

		printf("[signal_handler]signal = %d(%s)\n", sig, strsignal(sig));
		printf("caught signal: %d\n", sig);
		printf(" signal code = %d, ", si.si_code);
		// kill에 의해 전달된 시그널의 코드는 SI_USER(0)에 해당한다.
		if(si.si_code == SI_USER)
			printf("SI_USER ");
		// siqueue에 의해 전달된 실시간 시그널의 코드는 SI_QUEUE(-1)에 해당한다.
		else if(si.si_code == SI_QUEUE)
			printf("SI_QUEUE ");
		else
			printf("other ");
		// si->si_value.sival_int는 siqueue에 의해 전달된 사용자 정의값
		printf("signal value = %d ", si.si_value.sival_int);
		printf("sender pid = %d, sender uid = %d\n", si.si_pid, si.si_uid);


	}
}

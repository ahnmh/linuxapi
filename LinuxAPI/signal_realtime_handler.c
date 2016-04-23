/*
 * signal_realtime_handler.c
 *
 *  Created on: Apr 23, 2016
 *      Author: ahnmh
 */

#include "signal_func.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include "tlpi_hdr.h"

/* 프로세스간 실시간으로 사용자 정의 시그널을 주고 받을 수 있다.
 * 자유롭게 사용할 수 있는 표준 시그널의 갯수는 SIGUSER1, SIGUSER2 밖에 없지만 실시간 시그널을 사용하면 많은 수의 시그널을 정의해서 사용할 수 있다.
 * 표준 시그널과 다르게 실시간 시그널은 큐에 넣어진다. 실시간 시그널이 프로세스로 여러 번 전달되면, 시그널이 여러 번 전달된다.
 * 실시간 시그널을 전달할 때 시그널과 함께 전달되는 데이터(정수 아니면 포인터값)를 명시할 수 있다.
 * 여러 개의 실시간 시그널이 큐에 들어 있는 경우 가장 낮은 번호의 시그널이 우선 전달된다. 즉, 시그널이 낮은 번호의 시그널이 높은 우선순위를 갖도록 보장한다.
 * 동일한 종류의 여러 시그널이 큐에 있다면 보내진 순서대로 해당 데이터와 함께 전달된다.
 *
 * */

static void signal_realtime_handler(int sig, siginfo_t *si, void *ucontext)
{
	printf("caught signal: %d\n", sig);
	printf(" signal code = %d, ", si->si_code);
	// kill에 의해 전달된 시그널의 코드는 SI_USER(0)에 해당한다.
	if(si->si_code == SI_USER)
		printf("SI_USER ");
	// siqueue에 의해 전달된 실시간 시그널의 코드는 SI_QUEUE(-1)에 해당한다.
	else if(si->si_code == SI_QUEUE)
		printf("SI_QUEUE ");
	else
		printf("other ");
	// si->si_value.sival_int는 siqueue에 의해 전달된 사용자 정의값
	printf("signal value = %d ", si->si_value.sival_int);
	printf("sender pid = %d, sender uid = %d\n", si->si_pid, si->si_uid);
}

/*	출력 결과
	caught signal: 36 ---> siqueue에 의한 실시간 시그널
	 signal code = -1, SI_QUEUE signal value = 36 sender pid = 5966, sender uid = 1000
	caught signal: 37 ---> siqueue에 의한 실시간 시그널
	 signal code = -1, SI_QUEUE signal value = 37 sender pid = 5967, sender uid = 1000
	caught signal: 2 ---> kill 명령에 의한 시그널
	 signal code = 0, SI_USER signal value = 2 sender pid = 5998, sender uid = 1000
*/



void signal_realtime()
{
	printf("signal max: %d\n", SIGRTMAX);
	printf("signal min: %d\n", SIGRTMIN);


	struct sigaction act;
	sigemptyset(&act.sa_mask);

	// siginfo_t *si를 파라미터로 하는 확장 시그널 핸들러를 사용한다.
	act.sa_flags = SA_RESTART | SA_SIGINFO;
	act.sa_handler = signal_realtime_handler;


	// SIGSTOP과 SIGQUIT를 제외한 전체 시그널에 대해 모두 핸들러를 사용한다
	int sig;
	for(sig = 1; sig < NSIG; sig++)
		if(sig != SIGSTOP && sig != SIGQUIT)
			sigaction(sig, &act, NULL);


	while(1)
		pause();

}

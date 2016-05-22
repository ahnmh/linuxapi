/*
 * pthread_signal.c
 *
 *  Created on: May 21, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include "pthread_func.h"
#include "tlpi_hdr.h"

pthread_t t1, t2, signal_t3;

// 시그널에 대해 블록되어 있는(시그널을 받아도 무시되는) 일반 worker 스레드
static void *thread_worker(void *arg)
{
	printf("worker thread id = %x was started.\n", pthread_self());
	while(1)
		sleep(1);

	return NULL;
}

// SIGINT 시그널에 대해서만 수신을 허용하고 SIGINT 시그널이 도달하면
// worker 스레드를 종료하는 시그널 수신 전용 스레드
// sigwait 함수를 호출해서 동기적으로 처리한다.
static void *thread_signal_handler(void *arg)
{
	int s;
	int sig;
	sigset_t blockmask;

	printf("signal handler thread id = %x was started.\n", pthread_self());

	// 블록된 시그널 중에 SIGINT만 블록 해제한다.
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SIGINT);

	// SIGINT 시그널 수신을 무한 대기한다.
	if((s = sigwait(&blockmask, &sig)) != 0)
		errExitEN(s, "sigwait");
	else {
		// 정상적인 경우, worker 스레드 종료를 위해 각 worker 스레드에 SIGKILL 시그널을 보낸다.
		printf("signal %d was signaled.\n", sig);

		if((s = pthread_kill(t1, SIGKILL)) != 0)
			errExitEN(s, "sigwait");
		if((s = pthread_kill(t2, SIGKILL)) != 0)
			errExitEN(s, "sigwait");
	}
}

void pthread_signal()
{
	int s;
	sigset_t blockmask, originmask;

	// 모든 시그널에 대해 블록하고 새로 생성되는 스레드도 해당 속성을 부여받는다.
	sigfillset(&blockmask);
	if(pthread_sigmask(SIG_BLOCK, &blockmask, &originmask) != 0)
		errExit("pthread_sigmask()");

	s = pthread_create(&t1, NULL, thread_worker, NULL);
	s = pthread_create(&t2, NULL, thread_worker, NULL);
	s = pthread_create(&signal_t3, NULL, thread_signal_handler, NULL);

	s = pthread_join(t1, NULL);
	s = pthread_join(t2, NULL);
	s = pthread_join(signal_t3, NULL);



}

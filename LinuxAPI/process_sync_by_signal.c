/*
 * process_sync_by_signal.c
 *
 *  Created on: Apr 27, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "process_func.h"
#include "tlpi_hdr.h"

#define SYNC_SIG SIGUSR1

static void signal_handler(int sig)
{
	printf("SYNC_SIG signaled.\n");
}

void process_sync_by_signal()
{
	int status;
	pid_t child_proc;

	sigset_t originmask, blockmask, emptymask;
	struct sigaction sa;

	setbuf(stdout, NULL);

	// SYNC_SIG가 전달되지 않도록 시그널 블록
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SYNC_SIG);
	if(sigprocmask(SIG_BLOCK, &blockmask, &originmask) == -1)
		errExit("sigprocmask()");

	// 시그널 핸들러 설정
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = signal_handler;
	if(sigaction(SYNC_SIG, &sa, NULL) == -1)
		errExit("sigaction()\n");

	switch(child_proc = fork()) {
	case -1:
		errExit("fork()");

	// fork 함수 이후 자식 프로세스의 시작점. child의 경우 0을 리턴한다
	case 0:
		/* 자식 프로세스의 작업 수행 */
		printf("Do something child's job for 2 seconds\n");
		sleep(2);

		// 부모에게 시그널을 보내서 알린다.
		kill(getppid(), SYNC_SIG);

		exit(EXIT_SUCCESS);

	// fork함수 이후 부모 프로세 시작점. 부모의 경우 child의 pid를 리턴한다
	default:
		// 자식 프로세스가 작업을 완료하길 기다린다.
		printf("Wait child finishing his job...\n");

/*
		sigsuspend는 현재 프로세스의 시그널 마스크를 파라미터로 전달받은 마스크 집합으로 교체한후,
		현재 스레드를 블록하고 시그널을 기다린다.
		(여기서는 emptyset이므로 모든 시그널을 허용하게 됨)
		시그널을 받아서 시그널 핸들러가 호출된 다음 리턴되면 블록을 해제하고
		다시 원래의 시그널 마스크(SYNC_SIG를 블록했던 마스크 집합)를 복원한다.
*/
		sigemptyset(&emptymask);
		if(sigsuspend(&emptymask) == -1 && errno != EINTR)
			errExit("sigsuspend()");

		// 부모는 자신의 작업을 할 수 있다.
		printf("Do something parent's job..\n");

		// 시그널 블록을 해제한다.
		if(sigprocmask(SIG_SETMASK, &originmask, NULL) == -1)
			errExit("sigprocmask()");

		exit(EXIT_SUCCESS);
	}


}

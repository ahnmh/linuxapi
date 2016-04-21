/*
 * signal_basic.c
 *
 *  Created on: Apr 19, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE

#include "tlpi_hdr.h"
#include "signal_basic.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>

/*
signal의 경우 이클립스로 디버깅이 안된다.
터미널에서 그냥 실행해도 Ctrl + C(SIGINT)는 전달이 안된다. 그래서 kill 명령을 이용함.

터미널에서 실행한후에, 다른 터미널을 열어서 아래와 같이,
kill이나 pkill 명령을 통해 signal을 전달해주면 된다.
 $ kill -l : signal 확인
 $ kill -SIGINT <pid>
 $ pkill -SIGINT LinuxAPI
 $ kill -SIGINT $(pgrep -n LinuxAPI)
*/

static int cnt = 0;

static void signal_handler(int sig)
{
	// 시그널 설명 출력
	printf("%s\n", strsignal(sig));

	switch(sig)	{
	case SIGINT:
		printf("caught signal is SIGINT: %d\n", ++cnt);

/*
		 SIGINT 시그널이 3번 호출되면 4번째 시그널은 아래에 의해 기본 처리로 복구되므로
		 핸들러가 실행되지 않고 SIGINT 본래의 동작(terminate)가 수행될 것임.
*/
		if(cnt == 3) {
			if(signal(SIGINT, SIG_DFL) == SIG_ERR)
				errExit("signal(): SIGQUIT");
		}

/*		참고로 SIGINT 시그널을 무시(핸들러 호출도 없고, 기본 동작도 안함)하게 하려면 아래와 같다.*/

		if(cnt == 3) {
			if(signal(SIGINT, SIG_IGN) == SIG_ERR)
				errExit("signal(): SIGQUIT");
		}

		break;
	case SIGQUIT:
		printf("caught signal is SIGQUIT\n");
		exit(EXIT_SUCCESS);
	}
}

void signal_basic()
{
/*	2개의 시그널을 하나의 핸들러로 처리함.*/

	if(signal(SIGINT, signal_handler) == SIG_ERR)
		errExit("signal(): SIGINT");
	if(signal(SIGQUIT, signal_handler) == SIG_ERR)
		errExit("signal(): SIGQUIT");

	/*	자기자신에게 시그널을 전달하는 함수*/

//	if(raise(SIGINT) != 0)
//		errMsg("raise(SIGINT)");


/*	SIGINT 시그널 블록
*/
	sigset_t blockset, prevmask;
	// 비어 있는 시그널 셋을 만든다: blockset의 각 bit는 해당 시그널의 블록 여부를 의미하는데 모두 0으로 설정함
	sigemptyset(&blockset);
	// 시그널 셋에 SIGINT를 추가한다.
	sigaddset(&blockset, SIGINT);
	// SIGINT 를 블록하고 현재 시그널 셋을 저장해둔다(복원에 대비해서)
	if(sigprocmask(SIG_BLOCK, &blockset, &prevmask) == -1)
		errMsg("sigprocmask(): set");

/*	전체 시그널 블록
	// 꽉차 있는 시그널 셋을 만든다: blockset의 각 bit는 해당 시그널의 블록 여부를 의미하는데 모두 1로 설정함
	sigfillset(&blockset);
	// 전체 시그널을 블록하고 현재 시그널 셋을 저장해둔다
	// 단, SIGKILL(종료), SIGSTOP(중지)는 절대 블록이 불가능함.
	if(sigprocmask(SIG_BLOCK, &blockset, &prevmask) == -1)
		errMsg("sigprocmask(): set");
*/


	/* SIGINT에 의해 블록되어서는 안되는 코드 */
	sleep(5);

	// 기존 시그널 셋 복구
	if(sigprocmask(SIG_SETMASK, &prevmask, NULL) == -1)
		errMsg("sigprocmask(): reset");

/*	pending 중인 시그널을 확인할 수 있다.
 *  그러나, 정상 동작하지 않음...
 */
//	sigset_t pendingset;
//	if(sigpending(&pendingset) == -1)
//		errMsg("sigpending()");
//
//	printf("%d\n", pendingset);
//	if(sigismember(&pendingset, SIGINT) == 1)
//		printf("SIGINT is pending:\n");

	// 5초동안 pending되었던 시그널이 전달된다.
	// 보류 중인 시그널 집합은 단지 마스크이며 시그널이 발생했는지 여부를 가리키지만,
	// 몇 번 발생했는지는 나타내지 않는다. 따라서 동일한 시그널이 블록된 경우에 여러 번 생성되면 보류 중인 시그널의
	// 집합에 기록되고, 나중에 한 번만 전달됨.
	// SIGINT 블록중에 pkill -SIGINT LinuxAPI를 여러번 실행해도 한 번만 전달됨.

	while(1)
		pause(); // signal이 올때까지 블록시킴.

}

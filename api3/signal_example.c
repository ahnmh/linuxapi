/*
 * signal_example.c
 *
 *  Created on: Jul 22, 2016
 *      Author: ahnmh-vw
 */

#include "signal_example.h"
#include <sys/wait.h>

static void signal_handler(int signo)
{
	if(signo == SIGINT)
		// sys_siglist 배열을 사용하면 시그널 번호에 맞는 문자열을 가져올 수 있다.
		printf("pid = %d, caught signal = %s\n", getpid(), sys_siglist[SIGINT]);
}

/*기본적인 시그널 처리기인 signal 함수를 사용하는 예제*/
void signal_legacy()
{
	// SIGINT를 처리하는 시그널 핸들러를 등록한다.
	if(signal(SIGINT, signal_handler) == SIG_ERR)
		errexit("signal");

	// SIGQUIT(ctrl + \)를 무시하도록 설정한다.
	if(signal(SIGQUIT, SIG_IGN) == SIG_ERR)
		errexit("signal");

	// SIGTERM을 기본 처리기로 처리하도록(프로세스 종료) 설정한다.
	if(signal(SIGTERM, SIG_DFL) == SIG_ERR)
		errexit("signal");

	// pause: 프로세스를 종료시키는 시그널을 받을 때까지 해당 프로세스를 블록한다.
	while(1)
		pause();
}

/*
시그널 동작별 상속
동작					fork				exec
----------------------------------------------------
무시					상속됨				상속됨
기본					상속됨				상속됨
처리					상속됨				상속되지 않음(exec로 실행되는 프로세스가 이전 프로세스와 동일한 시그널 핸들러를 가지고 있지 않기 때문)
대기중인 시그널		상속되지 않음			상속되지 않음
*/

// 프로세스 또는 프로세스 그룹에 시그널을 전달하는 예제
void signal_send()
{
	pid_t pid;

	// SIGINT를 처리하는 시그널 핸들러를 부모 자식 모두 등록한다.(상속되므로)
	if(signal(SIGINT, signal_handler) == SIG_ERR)
		errexit("signal");

	switch(pid = fork()) {
	case -1:
		errexit("fork");
		break;

	case 0:
		printf("child = %d\n", getpid());
		// 시그널 대기
		pause();
		break;

	default:
		printf("parent = %d\n", getpid());
		sleep(5);

/*
		pid: 특정 프로세스에 시그널을 보냄
		0: 호출한 프로세스의 프로세스 그룹(자식 포함)에 속한 모든 프로세스에 시그널을 보냄.
		-pid: 특정 프로세스 그룹에 속한 모든 프로세스에 시그널을 보냄.(killpg 함수를 써도 된다)
*/
		if(kill(pid, SIGINT) == -1)
			errexit("kill");

		// 자식의 종료에 대기함.
		if(waitpid(pid, NULL, 0) == -1)
			errexit("waitpid");

		// 자기자신에게 시그널을 호출함.
		raise(SIGINT);

		break;
	}

}

void signal_helper()
{

}

/*
 * process_wait_by_signal.c
 *
 *  Created on: Apr 30, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "process_func.h"
#include "tlpi_hdr.h"

static volatile int sig_cnt = 0;
static volatile sig_atomic_t child_cnt = 5;


/*
자식 프로세스가 수행을 마치고 exit에 의해 종료되더라도, 부모 프로세스가 wait함수를 호출해서
status값을 읽기 전(쓰던 안쓰던)에는 커널은 자식 프로세스의 몇 가지 정보를 커널에 유지한다.
이를 좀비 프로세스라고 하며, 부모가 wait 함수를 날려주던지, 아니면 1번 프로세스가 거둔 후 wait함수를
날려주기 전까지 메모리에 존재하게 되며 좀비 프로세스는 SIGKILL 로도 죽일 수 없는 상태가 된다.
좀비 프로세스가 늘어나면 시스템 리소스 낭비가 되므로, 부모 프로세스는 생성한 자식 프로세스에
대해 wait함수를 호출하여 메모리에서 삭제될 수 있게 해야 한다.
아래는 SIGCHLD 시그널(자식이 종료되면 수신됨)을 이용한 그 방법을 나타냄.
*/


/*
SIGCHLD 시그널 핸들러
child 프로세스가 종료될 때 부모가 wait 함수를 호출하지 않으면 좀비 프로세스가 된다.
waitpid 함수를 호출하여 child의 종료를 대기한다.
*/
void sigchld_signal_handler(int sig)
{
	printf("sigal count: %d\n", ++sig_cnt);

	pid_t childpid;
	// WNOHANG: 명시된 자식 프로세스중 변경된 것이 없으면 블록킹하지 않고 즉각 리턴한다.
	while((childpid = waitpid(-1, NULL, WNOHANG)) > 0) {
		printf("Child process: %d takes wait from parent.\n", childpid);
		child_cnt--;
	}

/*
	waitpid의 리턴
	변경된 자식 프로세스가 없으면 0을 리턴
	자식 프로세스가 없는 경우 errno = ECHILD가 설정되고 -1리턴
*/
	if(childpid == -1 && errno != ECHILD)
		errMsg("waitpid()");
}

/*
SIGCHLD 시그널 핸들러를 설치한다.
SIGCHLD 시그널을 블록한다.
자식 프로세스들을 생성한다.
자식 프로세스가 하나도 없을 때까지 시그널을 받을 수 있게 대기한다.
*/
void process_wait_by_signal()
{
	sigset_t originmask, blockmask, emptymask;
	struct sigaction sa;

	// 시그널 핸들러 설정
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sigchld_signal_handler;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		errExit("sigaction()\n");

	// SIGCHLD 시그널 블록
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SIGCHLD);
	if(sigprocmask(SIG_BLOCK, &blockmask, &originmask) == -1)
		errExit("sigprocmask()");

	int i;
	pid_t child;
	for(i = 0; i < 5; i++) {
		switch(child = fork()) {
		case -1:
			errExit("fork()");
			break;
		case 0:
			printf("Child process: %d\n", getpid());
			// 자식 프로세스는 5초 후 종료
			sleep(5);

			_exit(EXIT_SUCCESS);
			break;
		default:
			break;
		}
	}

	// 자식을 5개 생성하여 하나라도 살아있으면 계속 SIGCHLD 신호를 대기한다.
	sigemptyset(&emptymask);
	while(child_cnt > 0) {
		if(sigsuspend(&emptymask) == -1 && errno != EINTR)
			errExit("sigsuspend()");
	}

	// 시그널 블록을 해제한다.
	if(sigprocmask(SIG_SETMASK, &originmask, NULL) == -1)
		errExit("sigprocmask()");
}

/*
프로그램의 수행 결과는 다음과 같다.
Child process: 6568
Child process: 6569
Child process: 6570
Child process: 6571
Child process: 6572
sigal count: 1
Child process: 6568 takes wait from parent.
Child process: 6569 takes wait from parent.
Child process: 6570 takes wait from parent.
sigal count: 2
sigal count: 3
Child process: 6571 takes wait from parent.
sigal count: 4
Child process: 6572 takes wait from parent.

즉, 시그널 핸들러는 4번 호출되었지만 5개의 자식 프로세스에 대해 모두 wait를 호출하도록 동작함.
알겠지만, 시그널 핸들러는 큐로 관리되지 않으므로 아직 처리되지 않은 시그널이 있는 경우 뒤에 오는 시그널은 무시된다.
따라서 while((childpid = waitpid(-1, NULL, WNOHANG)) > 0)과 같이 구성하지 않으면,
wait를 못받는 자식 프로세스가 생길 수 있음.

while((childpid = waitpid(-1, NULL, WNOHANG)) > 0)에 의해
waitpid는 자식 프로세스 변경이 있는 경우 해당 자식 프로세스의 pid를 리턴하므로
자식 프로세스 3개의 상태가 변경이 있다면(signal count: 1) 루프를 3번 돌면서 자식 프로세스 3개에 대한 wait를 호출한다.
즉, 한번의 시그널로 3개의 자식 프로세스에 대해 처리하게 됨.

signal count: 2일 때는 변경된 자식 프로세스가 없었기 때문에(count 1일 때 이미 처리됬을 것임)
while루프를 바로 빠져나오고 핸들러가 종료된다.

위에서 보듯이 signal count = 4까지 나온다. 즉 무시된 시그널이 1개 존재한다는 의미가 된다.

*/


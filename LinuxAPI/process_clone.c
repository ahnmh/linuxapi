/*
 * process_clone.c
 *
 *  Created on: May 1, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE	// clone

#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>	// clone

#include "process_func.h"
#include "tlpi_hdr.h"

#define CHILD_SIG	SIGUSR1
const int stack_size = 0x10000;

// clone된 자식 프로세스의 진입 함수
static int child_func(void *arg)
{
	printf("Child process has been started..\n");
	// 자식의 동작이 부모에게 영향을 주는가를 확인하기 위해서
	// 전달인자로 받은 /dev/null 파일 디스크립터를 닫는다.
	int *fd = (int*)arg;
	if(close(*fd) == -1)
		errExit("close()");

	return 0;
}

// 자식 프로세스 종료 시그널 핸들러
static void signal_func_child(int sig)
{
	printf("Child process has been exited. signal is = %d\n", sig);
}

void process_clone()
{
	char *stack;
	char *stack_top;
	int s, fd, flags;

	// dev/null 파일을 연다
	fd = open("/dev/null", O_RDWR);

	// 자식을 위한 스택을 할당한다.
	stack = malloc(stack_size);
	// 스택은 거꾸로 자라나므로 시작 위치는 맨 뒤야 한다.
	stack_top = stack + stack_size;

//	// 자식의 CHILD_SIG 시그널을 무시하도록 설정
//	if(signal(CHILD_SIG, SIG_IGN) == SIG_ERR)
//		errExit("signal()");

	// 자식 프로세스 종료 시그널 핸들러 설치
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = signal_func_child;
	if(sigaction(CHILD_SIG, &sa, NULL) == -1)
		errExit("sigaction()\n");
/*
 * 	clone 함수
	자식을 생성한다. 생성된 자식 프로세스는 fork함수와는 달리 child_func 함수를 수행하면서 시작된다.
	child_func 함수의 전달인자는 fd(/dev/null)이다.
	flags :
		CLONE_FILES: 부모의 파일 디스크립터를 공유한다.
		CHILD_SIG: 별도의 종료 시그널을 사용한다.
*/
	if(clone(child_func, stack_top, CLONE_FILES|CHILD_SIG, (void*)&fd) == -1)
		errExit("clone()");

	// 부모는 여기서 자식을 기다린다.
	// SIGCHLD가 아닌 다른 시그널을 사용해 자식 프로세스에게 알려야 하기 때문에 __WCLONE 이 사용된다.
	// _WCLONE: 복제된 자식만을 대기한다.
	// _WALL: 복제 유무와 상관없이 모든 자식 프로세스를 기다린다.
	// _WNOTHREAD: 호출한 프로세스의 자식만을(호출한 프로세스와 동일한 프로세스 그룹에 있는 다른 프로세스는 제외) 기다린다.
	if(waitpid(-1, NULL, __WCLONE) == -1)
		errExit("waitpid");

	printf("CHILD_SIG has been signalled..\n");

/*
	자식 프로세스의 함수 child_func은 /dev/null 파일 디스크립터를 닫는다.
	부모와 자식은 CLONE_FILES 플래그에 의해 파일 디스크립터를 공유하므로
	이 동작은 부모에게도 영향을 주기 때문에 부모는 여기서 /dev/null에 접근할 수 없다.
*/

	s = write(fd, "x", 1);
	if(s == -1 && errno == EBADF) // bad file number
		printf("file descriptor %d has been closed..\n", fd);
	else if(s == -1)
		printf("write on %d has been failed unexpectedly..\n", fd);
	else
		printf("write on %d succeeded..\n", fd);

}












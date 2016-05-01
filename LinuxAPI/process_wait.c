/*
 * process_wait.c
 *
 *  Created on: Apr 28, 2016
 *      Author: ahnmh
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "process_func.h"
#include "tlpi_hdr.h"

/* 자식 프로세스를 5개 생성하여 5개가 모두 종료되면 부모가 종료되도록 구현함 */
void process_wait()
{
	int i;
	for(i = 1; i <= 5; i++) {
		switch(fork()) {
		case -1:
			errExit("fork()");
		case 0:
			printf("child pid = %d\n", getpid());
			sleep(i);
			_exit(EXIT_SUCCESS);
		default:
			break;
		}
	}

	pid_t child;
	int status;
	while(1) {
/*
		자식 프로세스 종료를 대기하는 wait 함수. 종료된 자기 프로세스의 pid를 리턴함.
		단점:
			부모가 여러개의 자식을 생성하는 경우 특정 자식이 끝나기를 대기하는 것은 불가능하다.
			어떤 자식 프로세스도 종료되지 않으면, wait()는 항상 블록킹을 유지한다.
			종료된 자식에 대한 pid 정보만 얻을 수 있다.(자식 프로세스가 시그널에 의해 멈춰있거나 재개되는 상황은 알 수 없다)
*/
		child = wait(&status);
		// 기다리는 자식 프로세스가 없는 경우 -1을 리턴하고 이 때, errno는 ECHILD이다.
		if(child == -1) {
			if(errno == ECHILD) {
				printf("all children's are exited\n");
				exit(EXIT_SUCCESS);
			}
		}

		printf("child pid = %d is exited.\n", child);

	}
}


/* 자식 프로세스 중 첫번째 프로세스만 시그널에 대기하고 부모는 이 프로세스에 대해서만 기다림
 * waitpid 함수를 통해 자식 프로세스가 시그널에 의해 종료된 사실을 알 수 있음.
 * */

void process_waitpid()
{
	int i;
	pid_t child, wait_child;

	printf("Parent will only wait index no.1 child...\n");
	for(i = 1; i <= 5; i++) {
		switch(child = fork()) {
		case -1:
			errExit("fork()");
		case 0:
			printf("Index = %d, child pid = %d started..\n", i, getpid());
			// index가 0인 자식은 종료되지 않고 시그널을 수신하길 기다린다.
			if(i == 1)
				pause();
			else
				sleep(i);

			// index가 1이 아닌 자식들은 여기서 모두 종료된다.
			printf("Index = %d, child pid = %d finished..\n", i, getpid());
			_exit(EXIT_SUCCESS);

		default:
			// index 1번 자식의 pid를 구함.
			if(i == 1)
				wait_child = child;
			break;
		}
	}

	int status;

/*
	wait대비 waitpid의 장점
	 - 특정 pid를 가진 자식(아래 예제)만을 대기할 수 있다.
	 pid 파라미터로 올 수 있는 값
	  pid : 특정 자식에 대해서만 대기
	  0 : 부모 프로세스와 동일한 프로세스 그룹에 대해 대기
      -1 : 아무 자식 프로세스 중 하나가 끝나기를 대기
      -1보다 작으면 pid의 절대값과 동일한 프로세스 그룹 ID를 갖는 자식 프로세스를 기다림.
	 - 시그널에 의해 종료되는 것(아래 예제)도 감지한다.
	 - 자식 중 상태 변경된 것이 없다면 바로 리턴할 수 있다.
*/

	printf("test=%d\n", wait_child);
	waitpid(wait_child, &status, WUNTRACED); // WUNTRACED == 시그널에 의해 자식 프로세스가 멈출 때도 리턴함.

	printf("status = %d\n", status);
	// 종료 상태값을 확인할 수 있는 매크로
	if(WIFEXITED(status)) // 정상적(exit)에 의해 종료됨
		printf("WIFEXITED\n");
	if(WIFSIGNALED(status)) // 시그널에 의해 종료됨
		printf("WIFSIGNALED: %d\n", WTERMSIG(status)); // 시그널 번호를 얻어오는 매크로.
	if(WIFSTOPPED(status)) // 시그널에 의해 멈추었음.
		printf("WIFSTOPPED\n");
	if(WIFCONTINUED(status)) // SIGCONT를 받고 재개됨.
		printf("WIFCONTINUED\n");



}

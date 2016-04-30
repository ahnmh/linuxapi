/*
 * process_exec.c
 *
 *  Created on: Apr 30, 2016
 *      Author: ahnmh
 */
#include <stdio.h>
#include <sys/wait.h>

#include "process_func.h"
#include "tlpi_hdr.h"


void process_exec()
{
	char *argp[3];
	// 생성되는 프로세스로 전달하는 환경변수 리스트.
	// 리스트는 반드시 NULL로 끝나야 한다.
	char *envp[] = {"ENV1=AHNMH", "ENV2=SAMSUNG", "ENV3=IOT", NULL};

	// 생성되는 프로세스로 전달하는 명령행 인자 리스트
	char *processname = "myproc";
	// 아래와 같이 써도 된다.
//	char *processname = "./myproc";
	argp[0] = processname;
	argp[1] = "test_param";
	argp[2] = NULL; // 리스트는 반드시 NULL로 끝나야 한다.


	pid_t child;
	int status;
	switch(child = fork()) {
	case -1:
		errExit("fork()");
		break;

	case 0:
/*
		v : 명령행 인자는 NULL 원소로 끝나는 배열 구조이다.
		e : 환경 변수를 인자로 받는다. 이 경우 생성되는 프로세스의 전체 환경변수를 envp로 넘겨주게 됨.
		l : 명령행 인자는 NULL로 끝나는 리스트
*/
		execve(processname, argp, envp); // 성공하면 아무것도 리턴하지 않음.

/*
		l : 명령행 인자는 NULL로 끝나는 리스트
		     파라미터로 명령행 인자를 넘겨준다.
		별도로 envp를 지정하지 않으면 호출하는 프로세스의 환경변수를 그대로 넘겨준다.
*/
//		execl(processname, processname, "test_param", NULL);

		errExit("exec()"); // 실패하면 -1을 리턴한다.
		break;

	default:
		wait(&status);
		break;
	}
}

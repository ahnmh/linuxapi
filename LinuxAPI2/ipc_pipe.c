/*
 * ipc_pipe.c
 *
 *  Created on: May 22, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"
#define BUFFER_SIZE 4096

/*
파이프를 통한 IPC의 예제
: 파일 디스크립터를 사용한다.
: 데이터를 한 방향으로 전달한다.
: 파이프의 한쪽 끝이 쓰기로 사용된다면 다른 쪽 끝은 읽기로 사용한다.
: PIPE_BUF 바이트씩 쓰면 아토믹이 보장된다.
: 파이프는 바이트 스트림으로써 파이프를 통과하는 데이터는 순차적으로 전송되어 정확히 쓰여진 순서대로 읽힌다.(lseek를 사용해서 랜덤하게 읽을 수 없다)
: I/O는 read와 write를 통해서 수행함.
: pipe 함수와 주로 fork 호출을 이용해 연결한다. fork가 수행되는 동안 자식 프로세스는 부모의 파일 디스크립터 복사본을 상속받는다.
: 양방향 통신이 필요한 경우 2개의 파이프를 생성하고 각 프로세스는 하나를 정해 데이터 전송 용도로 사용하면 됨.
*/
void ipc_pipe(int argc, char *argv[])
{
	// pipe는 2개의 fd(0 = 읽기, 1 = 쓰기)를 사용한다.
	int pfd[2];
	char buf[BUFFER_SIZE];
	ssize_t numread;

	pid_t child_proc;

	if(pipe(pfd) == -1)
		errExit("pipe()");

	switch(child_proc = fork()) {
	case -1:
		errExit("fork()");

	// fork 함수 이후 자식 프로세스의 시작점. child의 경우 0을 리턴한다
	case 0:
		// 자식은 읽기로 동작하기 때문에 상속받은 2개의 파이프중 쓰기 fd를 닫는다.
		if(close(pfd[1]) == -1)
			errExit("close() - child");

		while(1)
		{
			// 읽기 endpoint에 들어온 데이터를 읽기 시도한다.
			// 쓰여진 데이터가 없으면 read 함수는 블록된다.
			numread = read(pfd[0], buf, BUFFER_SIZE);

			if(numread == -1)
				errExit("read() - child");
			// EOF를 만나면 read는 0을 리턴한다.
			if(numread == 0)
				break; // EOF
			if(write(STDOUT_FILENO, buf, numread) != numread)
				fatal("parent - partial / failed write");
		}

		// EOF를 만나서 루프를 탈출하면 개행 문자 삽입
		write(STDOUT_FILENO, "\n", 1);

		// 파이프의 읽기 endpoint를 닫는다.
		if(close(pfd[0]) == -1)
			errExit("close");

		break;

	// fork함수 이후 부모 프로세스 시작점. 부모의 경우 child의 pid를 리턴한다
	default:
		// 부모는 쓰기로 동작하기 때문에 최초 생성했던(pipe 함수) 파이프의 읽기 fd를 닫는다.
		if(close(pfd[0]) == -1)
			errExit("close() - parent");

		// PIPE로 쓰기 수행
		int len = strlen(argv[1]);
		if(write(pfd[1], argv[1], len) != len)
			fatal("parent - partial / failed write");

		// 파이프의 쓰기 endpoint를 닫는다. 이 때 EOF가 기록되며 자식의 read는 EOF를 보게 된다.
		// 부모의 쓰기 endpoint가 닫히기 전까지 자식의 read 함수는 블록될 것이므로 다 쓴 후에는 쓰기 endpoint를 반드시 닫아야 한다.
		if(close(pfd[1]) == -1)
			errExit("close");

		// 자식의 종료를 기다림.
		wait(NULL);

		break;
	}

	exit(EXIT_SUCCESS);
}

// 표준 입출력 파일 디스크립터를 사용해서 fd 파이프를 구성할 수 있다.
// 일반적으로 파이프를 생성하면 stdin, stdout, stderr가 각각 0, 1, 2 파일 디스크립터를 사용하고 있으므로,
// 3(읽기), 4(쓰기)로 할당된다. dup2를 사용해서 stdout(1)를 해제한다음, fd 4를 fd 1로 복제해서 사용할 수 있다.
void ipc_pipe_stdinout()
{
	int pfd[2];
	char buf[BUFFER_SIZE];
	ssize_t numread;

	pid_t child_proc;

	if(pipe(pfd) == -1)
		errExit("pipe()");

	switch(child_proc = fork()) {
	case -1:
		errExit("fork()");
		break;
	case 0:
		if(close(pfd[1]) == -1)
			errExit("close() - child");

		while(1)
		{
			// 부모의 stdout 으로부터의 쓰기를 읽기로 읽어들인다.
			numread = read(pfd[0], buf, BUFFER_SIZE);

			if(numread == -1)
				errExit("read() - child");
			if(numread == 0)
				break; // EOF
		}

		printf("child buf = %s\n", buf);

		if(close(pfd[0]) == -1)
			errExit("close");

		break;

	default:
		// 부모는 파이프 읽기 디스크립터를 닫아야 한다.
		if(close(pfd[0]) == -1)
			errExit("close() - parent");

		// STDOU_FILENO(1번) 디스크립터를 파이프 쓰기용 pfd[1]에 복제한다.
		// STDOUT_FILENO(1번)를 해제하고 pfd[1]을 1번 디스크립터로 복제한다.
		if(dup2(pfd[1], STDOUT_FILENO) == -1)
			errExit("dup2 - parent");

		// 기존 pfd[1], 즉 4번 디스크립터는 불필요하므로 닫는다.
		if(close(pfd[1]) == -1)
			errExit("close");

		char *msg = "1234567890";

		// 아래와 같이 일반 fd처럼 write를 쓸 수도 있지만
//		if(write(STDOUT_FILENO, msg, strlen(msg)) != strlen(msg))
//			fatal("parent - partial / failed write");

		// stdout을 사용하므로 printf를 사용할 수도 있다.(단 아래와 같이 버퍼링을 해제해야함)
		setbuf(stdout, NULL);
		printf("%s", msg);

		if(close(STDOUT_FILENO) == -1)
			errExit("close");

		wait(NULL);

		break;
	}

	exit(EXIT_SUCCESS);
}


/*
파이프는 보통 셸 명령을 실행할 때, 셸 명령의 실행 출력값을 읽거나 보낼때 사용한다.
이 작업을 쉽게 하고자 popen, pclose 함수를 제공함.
popen 함수
: 파이프 생성 -> 셸(/bin/sh)을 실행할 자식 프로세스를 생성 -> command 인자로 주어진 명령을 실행하기 위한 자식 프로세스를 생성
: 자식 프로세스(command 인자를 실행하는)의 표준 출력을 파이프 읽기 엔드포인트로 읽어들이거나,(아래 예제)
: 자식 프로세스(command 인자를 실행하는)의 표준 입력을 파이프 쓰기 엔드포인트로 사용

popen()의 이점:
파이프를 만들고 디스크립터를 복사하고 사용하지 않는 디스크립터를 닫는 역할을 수행해준다.
fork, exec를 대신 수행해준다.
그러나 보안상의 이유로 특권 프로그램에서는 사용해서는 안된다.(system도 마찬가지임)
*/
#include <limits.h> // PATH_MAX
void ipc_pipe_popen()
{
	FILE *fp;
	int filecnt = 0;
	char pathname[PATH_MAX];

	// ls -l 명령을 실행하는 자식 프로세스를 생성하고, 표준 출력을 읽기 엔드포인트로 읽어들인다.
	// 함수의 리턴값이 바로 읽기 엔드포인트 파일 디스크립터(파일 스트림)임.
	fp = popen("ls -l", "r");
	if(fp == NULL)
		errExit("popen()");

	// 읽기 엔드포인트 읽어온 결과물이 몇 라인인지 확인함.
	while(fgets(pathname, PATH_MAX, fp) != NULL) {
		printf("%s", pathname);
		filecnt++;
	}

/*
	pclose 함수: 파이프를 닫고 자식 셸이 종료되길 기다림.
	자식이 성공적으로 종료되면 자식 셸의 종료 상태를 리턴함.
*/
	int status;
	status = pclose(fp);
	printf("list count = %d\n", filecnt);
	printf("close status = %d\n", status);

	exit(EXIT_SUCCESS);
}






















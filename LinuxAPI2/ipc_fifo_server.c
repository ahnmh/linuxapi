/*
 * ipc_fifo_server.c
*/

#include <stdio.h>
#include <signal.h>
#include "ipc_func.h"

/*
FIFO 서버
: FIFO를 생성하고 클라이언트로부터의 쓰기를 입력받아 화면에 표시한다.
: FIFO는 파이프와 다르게 파일 시스템에 이름을 가지며(그래서 named pipe로 부름) 일반 파일처럼 열 수 있다.
: 파이프는 상속 관계(fork)에 있는 프로세스들끼리 사용가능하지만, FIFO는 관련이 없는 프로세스간에 통신을 할수 있게 해준다.
: 파이프처럼 FIFO도 쓰기 endpoint과 읽기 endpoint을 가지며 데이터 읽기와 같이 쓰여진 순서대로 데이터를 읽는다.
*/

void ipc_fifo_server()
{
	int serverfd, dummyfd;
/*
	주어진 경로와 이름으로 FIFO를 만든다.(마지막에 | 표시는 FIFO임을 의미함)
	prw--w----  1 ahnmh ahnmh     0 May 23 08:47 fifo_server|
*/
	if(mkfifo(SERVER_FIFO, S_IRUSR|S_IWUSR|S_IWGRP) == -1 && errno != EEXIST)
		errExit("server-mkfifo()");

	// FIFO를 연다.
	// 이 동작은 클라이언트가 FIFO가 O_WRONLY 모드로 open 할때까지(쓰기 endpoint가 생길때까지) 블록된다.
	// 서버는 읽기 endpoint이고 반드시 O_RDONLY 플래그를 사용해서 FIFO를 열어야 한다.
	serverfd = open(SERVER_FIFO, O_RDONLY);
	if(serverfd == -1)
		errExit("server-open()");

	// 쓰기 모드로 FIFO를 한번 더 열어서 서버가 EOF를 못보게 한다.
	// 이 과정이 없으면 ... , (아래 while으로 계속)
	dummyfd = open(SERVER_FIFO, O_WRONLY);
	if(dummyfd == -1)
		errExit("server-open()");

	// 고정 길이(request 구조체)로 메시지를 처리한다.
	// 메시지를 처리하는 다른 방법 : 구분 문자의 사용, 길이 필드가 저장된 헤더와 바디형태 사용
	struct request req;
	while(1) {
		// 위에서 dummfd의 쓰기 모드 열기가 없으면?
		// client 데이터를 모두 읽고 쓰기로 열린 endpoint가 더 이상 없으면 읽기는 EOF에 다다르게 되고,
		// 두번째 루프부터 아래 read가 0을 리턴하게 될 것이고, 따라서 무한루프를 돌게 된다.
		// 하지만 쓰기 모드로 열어둔 덕분에 read가 블록되어 계속된 읽기 입력에 대기할 수 있다.
		if(read(serverfd, &req, sizeof(struct request)) != sizeof(struct request))
			fprintf(stderr,"Error reading request.\n");
		else {
			printf("reqeust pid = %d, msg = %s\n", req.pid, req.msg);
		}
		continue;
	}

}



/*
 * ipc_fifo_client.c
 *
 *  Created on: May 23, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include "ipc_func.h"


/*
FIFO를 사용한 IPC 구현
자세한 것은 ipc_fifo/ipc_fifo_server.c를 참조할 것
*/
void ipc_fifo_client()
{
	int serverfd;
	// 서버의 FIFO 열기 동작은 여기 클라이언트의 FIFO 열기가 끝나기 전에는 블록 상태를 유지하며,
	// 반대도 마찬가지이다.
	// 클라이언트는 쓰기 endpoint이고 반드시 O_WRONLY 플래그를 사용해서 FIFO를 열어야 한다.
	serverfd = open(SERVER_FIFO, O_WRONLY);
	if(serverfd == -1)
		errExit("client-open()");

	struct request req;
	req.pid = getpid();
	req.seqLen = 5;
	strcpy(req.msg, "Hello! Server!");

	// 또한, 서버의 read 동작은 여기 클라이언트의 write 동작이 발생할 때까지 블록된다.
	// 반대도 마찬가지이다. 클라이언트가 먼저 실행되었다고 가정하면 서버가 읽어가기전까지 write 동작은 블록됨.
	// 또한, FIFO가 읽기용으로 열려 있지 않은 상태에서 쓰려고 열면 write로부터 SIGPIPE 시그널과 EPIPE에러가 발생함.
	if(write(serverfd, &req, sizeof(struct request)) != sizeof(struct request))
		errExit("client-write");
	else
		printf("message was sent to server.\n");
}


int main()
{
	ipc_fifo_client();
	return 0;
}

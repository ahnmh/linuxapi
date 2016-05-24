/*
 * ipc_systemv_msg_client.c
 *
 *  Created on: May 23, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include "ipc_func.h"

void ipc_systemv_msg_client()
{
	int msqid;

	// 서버가 파일에 써둔 메시지 식별자를 읽어온다.
	int oflags = O_RDONLY;
	int idfd = open(ID_PATH, oflags);
	if(read(idfd, &msqid, sizeof(int)) != sizeof(int))
		errExit("read()");
	close(idfd);

	int flags = 0;
	struct msgbuf msg;
	// 얻은 메시지 식별자로 메시지를 보낸다.
	// 식별자가 가르키는 message queue는 R/W 권한으로 생성했기 때문에, 해당 식별자를 사용해서 바로 쓸 수 있다.
	// client가 별도로 msgget을 호출할 필요없음.
	msg.mtype = 1;
	strcpy(msg.mtext, "test message from client1");
	if(msgsnd(msqid, &msg, strlen(msg.mtext) + 1, flags) == -1)
		errExit("msgsnd");

	msg.mtype = 2;
	strcpy(msg.mtext, "test message from client2");
	if(msgsnd(msqid, &msg, strlen(msg.mtext) + 1, flags) == -1)
		errExit("msgsnd");

}

int main()
{
	ipc_systemv_msg_client();
	return 0;
}

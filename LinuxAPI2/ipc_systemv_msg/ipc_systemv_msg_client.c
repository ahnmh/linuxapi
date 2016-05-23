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

	int oflags = O_RDONLY;
	int idfd = open(ID_PATH, oflags);
	if(read(idfd, &msqid, sizeof(int)) != sizeof(int))
		errExit("read()");
	close(idfd);

	struct msgbuf msg;
	msg.mtype = 1;
	strcpy(msg.mtext, "test message from client");

	int flags = 0;
	if(msgsnd(msqid, &msg, strlen(msg.mtext) + 1, flags) == -1)
		errExit("msgsnd");
}

int main()
{
	void ipc_systemv_msg_client();
	return 0;
}

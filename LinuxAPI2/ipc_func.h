/*
 * ipc_func.h
 *
 *  Created on: May 22, 2016
 *      Author: ahnmh
 */

#ifndef IPC_FUNC_H_
#define IPC_FUNC_H_

#include <sys/types.h>
#include "tlpi_hdr.h"

void ipc_pipe(int argc, char *argv[]);
void ipc_pipe_stdinout();
void ipc_pipe_popen();


#include <sys/stat.h>
#include <fcntl.h>
#define SERVER_FIFO "/tmp/fifo_server"

struct request {
	pid_t pid;
	int seqLen;
	char msg[100];
};
void ipc_fifo_server();
void ipc_fifo_client();

#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_MTEXT 1024
#define KEY_PATH "/tmp/LinuxAPI2"
#define ID_PATH "/tmp/id"
struct msgbuf {
	long mtype;
	char mtext[MAX_MTEXT];
};
void ipc_systemv_msg_server();
void ipc_systemv_msg_client();


#include <sys/sem.h>

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
#if defined(__linux__)
	struct seminfo *__buf;
#endif
};

void ipc_systemv_sem(int argc, char *argv[]);

#endif /* IPC_FUNC_H_ */

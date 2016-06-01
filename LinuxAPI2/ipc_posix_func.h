/*
 * ipc_posix_func.h
 *
 *  Created on: May 30, 2016
 *      Author: ahnmh
 */

#ifndef IPC_POSIX_FUNC_H_
#define IPC_POSIX_FUNC_H_

//-lrt 가 필요함

#define POSIX_MQ "/posix_message_queue"
#define MQ_SIZE	10

struct mq_message {
	int number;
	char name[100];
};

void ipc_posix_mq_create();
void ipc_posix_mq_write();
void ipc_posix_mq_read();
void ipc_posix_mq_read_by_signal();
void ipc_posix_mq_read_by_thread();
void ipc_posix_mq_destroy();

#define POSIX_SEM "/posix_semaphore"
void ipc_posix_sem_create();
void ipc_posix_sem_acquire();
void ipc_posix_sem_release();
void ipc_posix_sem_destroy();

void ipc_posix_unnamed_sem();

#endif /* IPC_POSIX_FUNC_H_ */

/*
 * ipc_posix_mq.c
 *
 *  Created on: May 30, 2016
 *      Author: ahnmh
 */

#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ipc_posix_func.h"
#include "tlpi_hdr.h"

/*
POSIX 메시지 큐 :
시스템V 메시지 큐와 동일하게 메시지 단위로 통신한다.
각 메시지에 우선순위를 할당할 수 있고 높은 우선순위의 메시지가 낮은 우선순위의 메시지보다 우선적으로 큐에 할당됨.
파일 시스템과 매우 비슷하다 : POSIX IPC 객체는 객체의 참조 카운트를 유지함.
따라서 프로그램은 객체가 언제 안전하게 삭제될 수 있는지 쉽게 설정할 수 있다.
시스템V IPC와 마찬가지로 POSIX IPC도 링크 해제되거나 시스템이 종료될 때까지 계속해서 존재함.
*/

void ipc_posix_mq_write()
{
	int flags;
	mode_t perms;
	mqd_t mqd;
	struct mq_attr attr;

	flags = O_CREAT | O_EXCL | O_RDWR;
	perms = S_IRUSR | S_IWUSR | S_IWGRP;


/*
	키를 통해 고유 식별자를 생성하는 시스템V IPC와 다르게,
	POSIX IPC는 /로 시작하는 문자열을 사용함.
	메시지 큐는 mq_open 함수의 리턴에 의해 얻어지는 메시지 큐 디스크립터에 의해 참조된다.
	attr 파라미터에 메시지큐의 메시지 최대 갯수, 메시지 최대 길이를 지정할 수 있다.
*/
	attr.mq_maxmsg = MQ_SIZE;
	attr.mq_msgsize = sizeof(struct mq_message);

	mqd = mq_open(POSIX_MQ, flags, perms, &attr);
	if(mqd == -1)
		errExit("mq_open()");

	struct mq_message msg;
	msg.number = 100;
	strcpy(msg.name, "Myunghoon Ahn");
	// 메시지 전송
	if(mq_send(mqd, (char*)&msg, sizeof(struct mq_message), 1) == -1)
		errExit("mq_send()");
	msg.number = 101;
	strcpy(msg.name, "Soohyun Ahn");
	if(mq_send(mqd, (char*)&msg, sizeof(struct mq_message), 0) == -1)
		errExit("mq_send()");
	msg.number = 102;
	strcpy(msg.name, "Ahhyun Kim");
	if(mq_send(mqd, (char*)&msg, sizeof(struct mq_message), 2) == -1)
		errExit("mq_send()");

	if(mq_getattr(mqd, &attr) == -1)
		errExit("mq_getattr()");

	printf("Number of message on queue = %ld\n", attr.mq_curmsgs);

	close(mqd);

}

void ipc_posix_mq_read()
{
	int flags;
	mqd_t mqd;
	struct mq_attr attr;

	flags = O_RDONLY;

	mqd = mq_open(POSIX_MQ, flags, 0, NULL);
	if(mqd == -1)
		errExit("mq_open()");

	if(mq_getattr(mqd, &attr) == -1)
		errExit("mq_getattr()");

	printf("Number of message on queue = %ld\n", attr.mq_curmsgs);

	ssize_t numread;
	struct mq_message msg;
	unsigned int priority;
	numread = mq_receive(mqd, (char*)&msg, sizeof(struct mq_message), &priority);
	if(numread == -1)
		errExit("mq_receive()");

	printf("read bytes = %ld, priority = %d\n", numread, priority);
	printf("number = %d\n", msg.number);
	printf("name = %s\n", msg.name);

	close(mqd);

}

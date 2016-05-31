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

void ipc_posix_mq_create()
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
}

void ipc_posix_mq_write()
{
	mqd_t mqd;
	struct mq_attr attr;

	mqd = mq_open(POSIX_MQ, O_RDWR, 0, NULL);
	if(mqd == -1)
		errExit("mq_open()");

/*
	메시지 전송
	마지막 파라미터인 우선순위대로 정렬된다. (0이 가장 낮음)
	동일한 우선순위인 경우 같은 우선순위 뒤에 삽입된다.
*/

	struct mq_message msg;
	msg.number = 100;
	strcpy(msg.name, "Myunghoon Ahn");
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

	// 메시지 큐에 대한 정보를 얻음.
	if(mq_getattr(mqd, &attr) == -1)
		errExit("mq_getattr()");

	// 메시지 큐의 메시지 갯수
	printf("Number of message on queue = %ld\n", attr.mq_curmsgs);

	close(mqd);

}

void ipc_posix_mq_read()
{
	int flags;
	mqd_t mqd;
	struct mq_attr attr;

	flags = O_RDONLY;

	// 열때는 O_RDONLY만 지정하면 됨.
	mqd = mq_open(POSIX_MQ, flags, 0, NULL);
	if(mqd == -1)
		errExit("mq_open()");

	if(mq_getattr(mqd, &attr) == -1)
		errExit("mq_getattr()");

	printf("Number of message on queue = %ld\n", attr.mq_curmsgs);

	ssize_t numread;
	struct mq_message msg;
	unsigned int priority;

/*
	메시지 큐 디스크립터로부터 우선순위가 가장 높은 오래된(FIFO) 메시지를 하나 꺼내온다.
	메시지 사이즈를 알지 못하는 경우, mq_getattr함수로부터 알 수 있다.
	메시시 큐가 현재 비어있다면 mq_receive는 메시지가 가용해질때까지 블록됨.
	O_NONBLOCK을 지정한 경우 EAGAIN으로 실패함
*/
	numread = mq_receive(mqd, (char*)&msg, sizeof(struct mq_message), &priority);
	if(numread == -1)
		errExit("mq_receive()");

	printf("read bytes = %ld, priority = %d\n", numread, priority);
	printf("number = %d\n", msg.number);
	printf("name = %s\n", msg.name);

	close(mqd);

}

#include <signal.h>
#define NOTIFY_SIG SIGUSR1

static void handler(int sig)
{
}

// 시그널을 이용한 메시지 도착 통지 : 메시지큐에 메시지가 들어오면 등록한 프로세스에 시그널을 전달함.
void ipc_posix_mq_read_by_signal()
{
	struct sigevent sev;
	mqd_t mqd;
	struct mq_attr attr;
	void * buffer;
	ssize_t numread;
	sigset_t blockMask, emptyMask;
	struct sigaction sa;

	mqd = mq_open(POSIX_MQ, O_RDONLY | O_NONBLOCK, 0, NULL);
	if (mqd == -1)
		errExit("mq_open()");

	if (mq_getattr(mqd, &attr) == -1)
		errExit("mq_getattr()");
	buffer = malloc(attr.mq_msgsize);

	// NOTIFY_SIG 시그널을 블록한다.
	sigemptyset(&blockMask);
	sigaddset(&blockMask, NOTIFY_SIG);
	if(sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1)
		errExit("sigprocmask()");

	sigemptyset(&sa.sa_mask); // 시그널 핸들러가 호출될 때 블록될 시그널을 위한 마스크 - 전체 시그널에 대해 블록하지 않음.
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	if(sigaction(NOTIFY_SIG, &sa, NULL) == -1)
		errExit("sigaction()");


/*
	메시지가 메시지큐에 도착하면 시그널 방식으로 통지를 받는 방법:
	- 하나의 프로세스만 등록 가능하다.(이미 등록되어 있는 프로세스가 있는 경우 실패)
	- 메시지큐가 비어있는 경우에만 통지를 받을 수 있다 : 이미 큐잉된 메시지가 다 제거되고 등록 이후에 온 메시지들에 대해서 통지를 받음.
	- 다른 프로세스가 현재 해당 큐에 대해 mq_receive 호출에 의해 블록되어 있지 않은 경우에만 통지를 받는다.
	- notification 인자를 NULL로 설정하여 mq_notify를 호출하면 명시적으로 등록 해제된다.(프로그램 종료시 자동 해제됨)
*/
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = NOTIFY_SIG;
	if(mq_notify(mqd, &sev) == -1)
		errExit("mq_notify()");

	sigemptyset(&emptyMask);

	// 메시지 수신 통지를 받은 후, 전체 메시지를 모두 읽어들인 후, 다시 통지를 기다리도록 동작함.
	while(1) {
		// 모든 시그널에 대해 대기한다.
		sigsuspend(&emptyMask);

		// 통지가 오면 곧바로 재등록함.
		// 메시지를 먼저 읽은 후에 통지를 등록하면 메시지가 수신된 후 등록 이전에 메시지가 큐에 들어오게 되면 통지를 받지 못하게 됨.
		if(mq_notify(mqd, &sev) == -1)
			errExit("mq_notify()");

		// 한번에 모든 메시지를 다 읽어들인다.
		unsigned int priority;
		while((numread = mq_receive(mqd, (char*)buffer, attr.mq_msgsize, &priority)) >= 0) {
			struct mq_message *pmsg;
			pmsg = (struct mq_message *)buffer;
			printf("read bytes = %ld, priority = %d\n", numread, priority);
			printf("number = %d\n", pmsg->number);
			printf("name = %s\n", pmsg->name);
		}

		if(errno != EAGAIN)
			errExit("mq_receive()");
	}

}

static void thread_func(union sigval sv)
{
	ssize_t numread;
	mqd_t *pmqd;
	void * buffer;
	struct mq_attr attr;
	struct sigevent sev;

	pmqd = sv.sival_ptr;

	if (mq_getattr(*pmqd, &attr) == -1)
		errExit("mq_getattr()");
	buffer = malloc(attr.mq_msgsize);

	// 통지가 오면 곧바로 재등록함.
	// 메시지를 먼저 읽은 후에 통지를 등록하면 메시지가 수신된 후 등록 이전에 메시지가 큐에 들어오게 되면 통지를 받지 못하게 됨.
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = thread_func;
	sev.sigev_notify_attributes = NULL;
	sev.sigev_value.sival_ptr = pmqd;
	if(mq_notify(*pmqd, &sev) == -1)
		errExit("mq_notify()");

	// 한번에 모든 메시지를 다 읽어들인다.
	unsigned int priority;
	while((numread = mq_receive(*pmqd, (char*)buffer, attr.mq_msgsize, &priority)) >= 0) {
		struct mq_message *pmsg;
		pmsg = (struct mq_message *)buffer;
		printf("read bytes = %ld, priority = %d\n", numread, priority);
		printf("number = %d\n", pmsg->number);
		printf("name = %s\n", pmsg->name);
	}

	if(errno != EAGAIN)
		errExit("mq_receive()");

}

void ipc_posix_mq_read_by_thread()
{
	struct sigevent sev;
	mqd_t mqd;

	mqd = mq_open(POSIX_MQ, O_RDONLY | O_NONBLOCK, 0, NULL);
	if (mqd == -1)
		errExit("mq_open()");

	// 이벤트 통지는 스레드 방식으로 수행한다.
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = thread_func;
	sev.sigev_notify_attributes = NULL;
	sev.sigev_value.sival_ptr = &mqd;

	// 스레드 방식 이벤트 통지 등록
	if(mq_notify(mqd, &sev) == -1)
		errExit("mq_notify()");

	// 아무 시그널이 전달될 때까지 블록
	pause();
}

void ipc_posix_mq_destroy()
{
	// 파일 삭제하는 것과 동일하게 메시지큐를 삭제함.
	if(mq_unlink(POSIX_MQ) == -1)
		errExit("mq_unlink()");

}

/*
메시지 큐는 실질적으로 파일 디스크립터를 사용한다. 따라서 아래와 같이 파일처럼 접근하여 확인이 가능함.

ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ sudo su
[sudo] password for ahnmh-vw:
root@ubuntu:/home/ahnmh-vw/workspace/linuxapi/LinuxAPI2/Debug#
root@ubuntu:/home/ahnmh-vw/workspace/linuxapi/LinuxAPI2/Debug#
root@ubuntu:/home/ahnmh-vw/workspace/linuxapi/LinuxAPI2/Debug# mkdir /dev/mqueue
root@ubuntu:/home/ahnmh-vw/workspace/linuxapi/LinuxAPI2/Debug# mount -t mqueue none /dev/mqueue
root@ubuntu:/home/ahnmh-vw/workspace/linuxapi/LinuxAPI2/Debug# exit
exit
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ cat /proc/mounts | grep mqueue
none /dev/mqueue mqueue rw,relatime 0 0
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ ls -ld /dev/mqueue/
drwxrwxrwt 2 root root 60 May 31 20:31 /dev/mqueue/
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ cd /dev/mqueue/
ahnmh-vw@ubuntu:/dev/mqueue$ ll
total 0
drwxrwxrwt  2 root     root       60 May 31 20:31 ./
drwxr-xr-x 17 root     root     4340 May 31 20:36 ../
-rw--w----  1 ahnmh-vw ahnmh-vw   80 May 31 20:32 posix_message_queue
ahnmh-vw@ubuntu:/dev/mqueue$ cat posix_message_queue
QSIZE:0          NOTIFY:0     SIGNO:0     NOTIFY_PID:0

*/








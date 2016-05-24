/*
 * ipc_systemv_msg_server.c
 *
 *  Created on: May 23, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include "ipc_func.h"


/* messag queue 예제
프로세스간 메시지를 전달할 때 사용한다. 파이프와의 차이점
: 메시지 범위가 정해지지 않은 파이프와는 달리 메시지의 경계가 존재하며 송수신자가 메시지 단위로 통신함.
: 각 메시지의 type 필드를 통해 쓰여진 순서대로 메시지를 읽지 않고 종류별로 구분해서 메시지를 선택할 수 있다.
ipcs 명령을 통해 시스템의 전체 시스템V message queue, semaphore, shared memory를 확인할 수 있다.
$ ipcs
...
------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages
0x0101039b 32768      ahnmh      620        0            0
*/

void ipc_systemv_msg_server()
{
	key_t key;
	int msqid;
	int flags;
	const unsigned int perms = S_IRUSR | S_IWUSR | S_IWGRP; // message queue에 대해 얻고자 하는 권한

	// IPC 식별자는 파일 디스크립터가 파일을 가르키는 것처럼 message queue, semaphore, shared memory 객체를 가르킨다.(일종의 핸들)
	// IPC 식별자를 구할 키를 생성한다. 또는 아래와 같이 ftok를 사용하지 않고 IPC_PRIVATE를 직접 명시해서 식별자를 생성해도 된다.
	// msqid = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR); 이 경우 IPC_CREAT, IPC_EXCL을 명시할 필요 없음.
	// 첫번째 파라미터 pathname은 어플리케이션이 만들거나 형성한 하나의 파일이나 디렉토리를 가리키고,
	// message queue를 통해 통신하는 다른 프로세스도 동일한 pathname을 사용해야 함.
	key = ftok(KEY_PATH, 1);
	if(key == -1)
		errExit("ftok()");

	flags = 0;
	flags |= IPC_CREAT; // 신규로 생성
	flags |= IPC_EXCL; // 하나의 IPC 객체 생성을 보장

	// 키와 플래그를 통해 IPC 식별자를 생성한다.
	// 파일 디스크립터와 유사하나, 시스템 전체적으로 공개되며, 식별자를 통해 같은 객체에 접근하는 모든 프로세스는 같은 식별자를 사용함.
	// 플래그 값이 0이면(IPC_CREAT도 아니고 IPC_EXCL도 아니고 message queue에 대해 특별히 권한을 요구하지 않음)
	// key에 대응하는 현재 존재하는 식별자가 리턴됨.
	while((msqid = msgget(key, flags | perms)) == -1) {
		if(errno == EEXIST) {
			// 키에 해당하는 식별자가 있는지 확인
			msqid = msgget(key, 0);
			// 이미 존재하는 message queue인 경우 삭제함.
			if(msgctl(msqid, IPC_RMID, NULL) == -1)
				errExit("msgctl()");
			printf("remove old message queue. msqid = %d\n", msqid);
		}
	}

	if(msqid == -1)
		errExit("msgget()");

	// 여기서 출력되는 식별자를 사용해서 클라이언트는 서버에게 메시지를 보낸다.
	// 클라이언트는 이 식별자를 msgsend 함수의 첫번째 파라미터로 사용할 것임.
	printf("msqid = %d\n", msqid);

	// 클라이언트가 식별자 값을 참조할 수 있도록 파일에 써둔다.
	int oflags = O_CREAT | O_WRONLY | O_TRUNC;
	mode_t fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	int idfd = open(ID_PATH, oflags, fileperms);
	if(write(idfd, &msqid, sizeof(int)) != sizeof(int))
		errExit("write()");
	close(idfd);

	ssize_t msgLen;
	int msgflag = 0;
	struct msgbuf msg;
	// 메시지 큐의 통계 정보를 볼 수 있는 구조체
	struct msqid_ds ds;

/*
	messag queue로부터 메시지를 수신한다.
	지정한 type의 메시지가 들어올 때까지 블록한다.
	type을 0으로 지정하면, 가장 먼저 들어온 메시지를 받아들인다.
	type을 양수로 지정하면 해당 type과 정확히 일치하는 메시지만 받아들이고,
	type을 음수로 지정하면 절대값보다 작은 type을 모두 받아들인다.
	아래 반복문을 통해서 메시지 type이 1, 2에 해당하는 모든 타입을 받아들인다.
	함수 전달 인자로 수신 길이에 해당하는 파라미터가 없다. 즉, 구분된 메시지 형태로 수신하는 것임(파일 스트림 구조인 파이프나, FIFO와 다른점임)

	메시지 flag :
	IPC_NOWAIT = type에 해당하는 메시지가 없으면 들어올때까지 블록킹하는데 이 플래그를 지정하면 바로 ENOMSG를 리턴함.
	MSG_NOERROR = 메시지의 mtext 필드 크기가 사용 가능 공간(3번째 파라미터)을 초과하면 msgrcv는 실패한다.
	                이 플래그를 지정하면 실패하지 않고 최대크기만큼 잘라서 리턴함.
*/
	while((msgLen = msgrcv(msqid, &msg, MAX_MTEXT, -2, msgflag)) != -1) {
		if(msgLen == -1)
			errExit("msgrcv");

		printf("received message id = %ld, text = %s\n", msg.mtype, msg.mtext);

		// message queue의 통계 정보 확인
		if(msgctl(msqid, IPC_STAT, &ds) == -1)
			errExit("msgctl()");
		printf("message queue info :\n");
		printf("\tremained message = %d\n", ds.msg_qnum); // 큐에 남은 메시지 수
		printf("\tremained byte = %d\n", ds.__msg_cbytes); // 큐에 보관할 수 있는 최대 바이트 수
		printf("\tsender pid = %d\n", ds.msg_lspid); // 큐에 메시지를 보낸 PID
		printf("\treceiver pid = %d\n", ds.msg_lrpid); // 큐로부터 메시지를 받은 PID
		printf("\tmaximum byte = %d\n", ds.msg_qbytes); // 큐에 보관할 수 있는 최대 바이트 수

	}

/*
	type = 2에 해당하는 메시지만 수신하는 경우의 예제
	msgLen = msgrcv(msqid, &msg, MAX_MTEXT, 2, 0);
	if(msgLen == -1)
		errExit("msgrcv");
	printf("received message id = %ld, text = %s\n", msg.mtype, msg.mtext);
*/

}

// 커널 전역적인 구조체이기 때문에 프로세스가 종료되어도 message queue는 사라지지 않음

/*
시스템V Messag queue의 단점:
1. 대부분의 unix I/O는 파일 디스크립터를 사용하는 반면 메시지큐는 ID를 사용한다.
   따라서 파일 디스크립터 기반 I/O 기술을 메시지 큐에 사용할 수 없다.
2. 메시지큐는 비연결성이기 때문에 커널은 파이프, FIFO, 소켓이 하는 것처럼 현재 큐를 참조하는 프로세스의 수를 관리하지 않는다.
   (언제 프로그램이 메시지 큐를 삭제하면 좋을지 알기가 어려움)
3. 메시지큐의 총 갯수, 메시지 크기, 개별큐의 용량에는 한도가 있음.
*/


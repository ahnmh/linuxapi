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

}

// 프로세스가 종료되어도 message queue는 사라지지 않음


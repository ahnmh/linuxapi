/*
 * ipc_systemv_shm_writer.c
 *
 *  Created on: May 26, 2016
 *      Author: ahnmh
 */

#include "ipc_func.h"

/* 시스템 V 공유 메모리
 * 공유 메모리는 둘 이상의 프로세스 간의 동일한 메모리 페이지를 공유하도록 지원한다.
 * 공유 메모리는 데이터를 주고받을 때 이를 커널이 중재할 필요가 없기 때문에 빠른 IPC를 제공한다.
 * 단, 시스템 V 세마포어 같은 동기화 기술을 사용해 공유 메모리에 접근해야 한다.
 * 공유 메모리를 세그먼트에 부착할 때(shmat) 커널이 프로세스의 가상 주소 공간을 결정하도록 맡기는 것이 좋다(shmat함수의 shmaddr 파라미터를 NULL로 넘기는 것)
$ ipcs
------ Shared Memory Segments --------
...
0x00001001 2195474    ahnmh      660        1028       1
*/
void ipc_systemv_shm_writer() {
	// 읽기, 쓰기 세마포어 식별자 생성한다.
	int semid, shmid;
	union semun arg;
	struct sembuf sop;
	struct shmmsg *shmp;

	// 그냥 pre-define 키를 사용함. 안그러면 ftok로 생성한 고유키를 reader에게 넘겨주는 과정이 필요하다.
	semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);

	// 쓰기 세마포어를 사용 가능하게 설정
	arg.val = 1;
	if (semctl(semid, WRITE_SEM, SETVAL, arg) == -1)
		errExit("semctl()");

	// 읽기 세마포어를 사용중으로 설정
	arg.val = 0;
	if (semctl(semid, READ_SEM, SETVAL, arg) == -1)
		errExit("semctl()");

	// 공유 메모리를 생성한다.
	// 요청 크기는 공유메모리를 통해 주고받을 메시지 크기로 설정하지만 실제 할당은 페이지 크기로 확장됨.
	shmid = shmget(SHM_KEY, sizeof(struct shmmsg), IPC_CREAT | OBJ_PERMS);

	// 공유 메모리를 가상 메모리에 할당한다.
	shmp = shmat(shmid, NULL, 0);

	// stdin의 데이터 블록 끝(EOF)까지 반복
	int bytes;
	for (bytes = 0;; bytes += shmp->cnt) {
		// 쓰기 세마포어를 얻는다.
		// 공유 메모리에 쓴 이후에는 reader가 읽은 후 (루프를 돌아서) 쓰기 세마포어를 릴리즈해주기전까지는 블록될 것임.
		sop.sem_num = WRITE_SEM;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		if (semop(semid, &sop, 1) == -1) {
			errExit("semop()");
		}
		// stdin의 데이터 내용을 공유 메모리에 쓴다.(stdin으로부터 읽은 내용을 공유 메모리 주소에 저장)
		shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);

		// reader가 공유 메모리를 읽을 수 있도록 읽기 세마포어를 해제한다.
		sop.sem_num = READ_SEM;
		sop.sem_op = 1;
		sop.sem_flg = 0;
		if (semop(semid, &sop, 1) == -1) {
			errExit("semop()");
		}

		// EOF에 다다르면 루프를 탈출한다.
		if (shmp->cnt == 0)
			break;
	}

	// 쓰기를 모두 완료한 이후 쓰기 세마포어를 한번 더 얻는다.(수신자 작업 종료 확인용)
	sop.sem_num = WRITE_SEM;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	if (semop(semid, &sop, 1) == -1) {
		errExit("semop()");
	}

	// 세마포어 식별자를 삭제한다.
	if (semctl(semid, IPC_RMID, 0) == -1)
		errExit("semctl()");

	// 공유 메모리를 삭제한다.
	if (shmdt(shmp) == -1)
		errExit("shmdt()");

	// 공유 메모리 식별자를 삭제한다.
	if (shmctl(shmid, IPC_RMID, NULL) == -1)
		errExit("shmctl()");

}

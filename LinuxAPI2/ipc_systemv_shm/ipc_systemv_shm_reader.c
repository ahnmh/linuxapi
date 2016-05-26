/*
 * ipc_systemv_shm_reader.c
 *
 *  Created on: May 26, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include "ipc_func.h"

void ipc_systemv_shm_reader()
{
	int semid, shmid;
	struct sembuf sop;
	struct shmmsg *shmp;

	// writer가 생성한 세마포어와 공유 메모리를 얻는다.
	semid = semget(SEM_KEY, 2, 0);
	shmid = shmget(SHM_KEY, sizeof(struct shmmsg), 0);
	// 공유 메모리를 읽기 전용으로 attach한다.
	shmp = shmat(shmid, NULL, SHM_RDONLY);

	int bytes;
	// 공유 메모리로부터 stdout으로 데이터를 전송한다.
	// EOF를 만날때까지 반복
	for (bytes = 0;; bytes += shmp->cnt) {
		// 읽기 세마포어를 얻는다. writer가 데이터를 공유메모리에 쓰고 나면 읽기 세마포어를 릴리즈해줌. 그전까지 블록됨.
		sop.sem_num = READ_SEM;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		if (semop(semid, &sop, 1) == -1) {
			errExit("semop()");
		}

		// EOF인지 검사한다.
		if(shmp->cnt == 0)
			break;

		// 공유 메모리의 데이터를 stdout에 쓴다.
		write(STDOUT_FILENO, shmp->buf, shmp->cnt);

		// 읽기를 완료했으므로 writer가 공유 메모리에 다시 쓸수 있게 쓰기 세마포어를 해제한다.
		sop.sem_num = WRITE_SEM;
		sop.sem_op = 1;
		sop.sem_flg = 0;
		if (semop(semid, &sop, 1) == -1) {
			errExit("semop()");
		}
	}

	// 공유 메모리를 detach한다.
	if(shmdt(shmp) == -1)
		errExit("shmdt()");

	// EOF 검사 이후 쓰기 세마포어가 해제되지 않은 상태이다.
	// 쓰기 세마포어를 한번 더 해제하여 write에게 reader의 작업이 모두 종료했음을 알림.
	sop.sem_num = WRITE_SEM;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	if (semop(semid, &sop, 1) == -1) {
		errExit("semop()");
	}

	// 수신 정보 출력
}

int main()
{
	ipc_systemv_shm_reader();
	return 0;
}

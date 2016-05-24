/*
 * ipc_systemv_sem.c
 *
 *  Created on: May 24, 2016
 *      Author: ahnmh
 */

#include "ipc_func.h"

void ipc_systemv_sem(int argc, char *argv[])
{
	int ret;
	int semid;
	union semun arg;
	key_t key;
	const unsigned int perms = S_IRUSR | S_IWUSR | S_IWGRP;
	//	arg.val = getInt(argv[1], 0, "init-value");
	struct semid_ds ds;

	key = ftok(KEY_PATH, 1);
	semid = semget(key, 1, IPC_CREAT | IPC_EXCL | perms);

	if(semid == -1) {
		union semun arg;
		struct sembuf sop;

		// 세마포어 값 = 0
		arg.val = 0;
		ret = semctl(semid, 0, SETVAL, arg);

		// 세마포어를 생성하는 프로세스가 no-op 동작을 수행하여 sem_optime(접근 시간)을 변경해서
		// 뒤이어 다른 프로세스가 세마포어가 초기화되었음을 알 수 있게 함.
		struct sembuf sop;
		sop.sem_num = 0; // 세마포어 번호 0에 대해서,
		sop.sem_op = 0; // 세마포어 값은 이미 0이므로 아무 작업도 이루어지지 않는다.
		sop.sem_flg = 0;
		ret = semop(semid, &sop, 1); // 이 때 sem_optime이 업데이트됨.
	}
	else {
		const int MAX_TRIES = 10;
		int j;
		union semun arg;

		semid = semget(key, 1, perms); // 이미 열려진 세마포어 식별자를 얻어옴.

		arg.buf = &ds;
		// 다른 프로세스가 semop를 호출할 때까지 대기
		ret = semctl(semid, 0, IPC_STAT, arg);
		// 세마포어 생성이 완료되었다면, 루프 종료
		if(ds.sem_otime != 0)
			break;
		sleep();
	}

	if(ds.sem_otime == 0)
		fatal("Existing semaphore not initialized.");
}

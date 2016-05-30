/*
 * ipc_systemv_sem.c
 *
 *  Created on: May 24, 2016
 *      Author: ahnmh
 */

#include "ipc_systemv_func.h"


/* 시스템V 세마포어 예제
프로세스 간(두 스레드 간이 아닌) 리소스(공유 메모리 등) 접근 동기화에 사용된다.
시스템V 메시지큐와 마찬가지로 시스템 전역 구조체임.
$ipcs
...
------ Semaphore Arrays --------
key        semid      owner      perms      nsems
0xffffffff 229376     ahnmh      620        1

세마포어 삭제 방법
$ipcs -s <id>

*/

void ipc_systemv_sem()
{
	int semid;
	key_t key;
	const unsigned int perms = S_IRUSR | S_IWUSR | S_IWGRP; // 읽기/쓰기 권한이 있어야 함.
	struct semid_ds ds;

	key = ftok(KEY_PATH, 1);

	// 세마포어의 갯수가 1개인(0번) 세마포어 집합을 생성한다.
	semid = semget(key, 1, IPC_CREAT | IPC_EXCL | perms);
/*
	세마포어에 접근하는 프로세스 중 세마포어를 생성하는 프로세스는 1개이다.
	나머지 프로세스는 여기서 EEXIST 에러를 리턴받고 else로 가서 이미 생성된 세마포어에 접근하게 됨.
*/
	if(semid != -1) {
		union semun arg;
		struct sembuf sop;

		// 0번 세마포어 값을 0으로 설정
		arg.val = 0;
		if(semctl(semid, 0, SETVAL, arg) == -1)
			errExit("semctl()");
/*
		sem_op = 0의 값으로 semop을 호출하는 것은 세마포어 값이 0이 될때까지 블록한다는 의미이다.
		그런데 이미 세마포어의 값이 0으로 초기화되기 때문에 no-op에 해당한다.
		하지만 semop 호출은 sem_optime(접근 시간)을 변경해서
		뒤이어 실행되는 다른 프로세스가 세마포어가 초기화되었음을 알 수 있게 한다.
*/
		sop.sem_num = 0; // 세마포어 번호 0에 대해서,
		sop.sem_op = 0; // 세마포어 값은 이미 0이므로 아무 작업도 이루어지지 않는다.
		sop.sem_flg = 0;
		if(semop(semid, &sop, 1) == -1) {
			errExit("semop()");
		} // 이 때 sem_optime이 업데이트됨.

		// 0번 세마포어 값을 2로 설정.
		// 따라서 세마포어 값이 1, 2일 때 프로세스들은 '작업'으로 진입할 수 있다.
		arg.val = 2;
		if(semctl(semid, 0, SETVAL, arg) == -1)
			errExit("semctl()");
	}
	// key에 해당하는 세마포어 식별자가 이미 생성되어 있는 경우,
	else {
		const int MAX_TRIES = 10;
		int j;
		union semun arg;
		// 이미 열려진 세마포어 식별자를 얻어옴.
		if((semid = semget(key, 1, perms)) == -1)
			errExit("semget()");

		arg.buf = &ds;
		// 세마포어를 생성하는 프로세스가 semop를 호출할 때까지 대기
		for (j = 0; j < MAX_TRIES; ++j) {
			if(semctl(semid, 0, IPC_STAT, arg) == -1)
				errExit("semctl()");
			// 세마포어 생성이 완료되었다면, 루프 종료
			if(ds.sem_otime != 0)
				break;
			sleep(1);
		}
	}

	if(ds.sem_otime == 0)
		fatal("Semaphore not initialized yet.");

	// 0번 세마포어 값 조회
	int semval = semctl(semid, 0, GETVAL, 0);
	printf("val = %d\n", semval);

	// 세마포어 값을 -1한다. -1이 가능할 때까지 대기함.
	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	if(semop(semid, &sop, 1) == -1) {
		errExit("semop()");
	}
/*
	여기서 만일 semop에 의해 블록되고 있는 상태에서 SIGINT등을 받아서 블록이 중단되면,
	세마포어를 해제하는 작업이 수행되지 않기 때문에, 다른 프로세스들이 영원히 진입할 수 없는 문제가 생김.
*/
	// 세마포어로 보호해야 하는 '작업'을 수행
	printf("job start..\n");
	sleep(5);
	printf("job end..\n");


	// 세마포어 값을 +1한다. 세마포어에 대기하고 있는 다른 프로세스들이 '작업'으로 진입한다.
	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	if(semop(semid, &sop, 1) == -1) {
		errExit("semop()");
	}


}

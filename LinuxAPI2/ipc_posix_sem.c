/*
 * ipc_posix_sem.c
 *
 *  Created on: May 31, 2016
 *      Author: ahnmh-vw
 */
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ipc_posix_func.h"
#include "tlpi_hdr.h"

/*
POSIX named 세마포어:
관련이 없는 두 프로세스 간 세마포어를 사용하는 경우
프로세스가 세마포어를 0이하로 떨어뜨리려는 경우, 블록되거나 실패함.
named 세마포어는 /dev/shm 디렉토리에 마운트된 전용의 tmpfs 파일 시스템에서 이름이 sem.name 형태의 공유 메모리 객체로 생성된다.
커널 지속성을 갖지만 시스템이 종료되면 없어진다.
자식은 부모의 모든 named 세마포어의 참조를 상속한다.
*/
void ipc_posix_sem_create()
{
	int flags;
	mode_t perms;
	unsigned int value;
	sem_t *sem;

	flags = O_CREAT | O_EXCL | O_RDWR; // 세마포어를 열때 접근 권한은 O_RDWR이어야 함.
	perms = S_IRUSR | S_IWUSR | S_IWGRP;

	value = 0; // 세마포어의 초기값
	sem = sem_open(POSIX_SEM, flags, perms, value);
}

void ipc_posix_sem_acquire()
{
	sem_t *sem;
	// 기존 세마포어를 참조할 때 파라미터 2개만 필요함.
	sem = sem_open(POSIX_SEM, 0);
	if (sem == SEM_FAILED)
		errExit("sem_open()");

	// 세마포어 감소를 시도한다. 현재 값이 0인 경우 블록됨.
	if (sem_wait(sem) == -1)
		errExit("sem_wait()");

/*
	비블로킹 버전
	감소가 즉시 실행될 수 없으면 EAGAIN 에러로 실패함.
	if (sem_trywait(sem) == -1)
		errExit("sem_wait()");
*/

	printf("semaphore acquired.\n");

	int value;
	// 현재 세마포어 값을 얻어옴.
	if (sem_getvalue(sem, &value) == -1)
		errExit("sem_getvalue()");
	printf("semaphore value = %d\n", value);
}

void ipc_posix_sem_release()
{
	sem_t *sem;
	sem = sem_open(POSIX_SEM, 0);
	if (sem == SEM_FAILED)
		errExit("sem_open()");

	// 세마포어 값을 증가시킨다.
	if (sem_post(sem) == -1)
		errExit("sem_post()");

	printf("semaphore released.\n");

	int value;
	// 현재 세마포어 값을 얻어옴.
	if (sem_getvalue(sem, &value) == -1)
		errExit("sem_getvalue()");
	printf("semaphore value = %d\n", value);

}

void ipc_posix_sem_destroy()
{
	if (sem_unlink(POSIX_SEM) == -1)
		errExit("sem_unlink()");
}


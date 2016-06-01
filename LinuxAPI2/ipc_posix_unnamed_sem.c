/*
 * ipc_posix_unnamed_sem.c
 *
 *  Created on: Jun 1, 2016
 *      Author: ahnmh-vw
 */

#include <semaphore.h>
#include <sys/stat.h>

#include "ipc_posix_func.h"
#include "tlpi_hdr.h"

/*
POSIX unnamed 세마포어 :
스레드 간 또는 관련된 프로세스(부모 - 자식) 간에 공유되는 세마포어.
이름이 필요없다.
unnamed 세마포어를 공유 변수(전역, 힙)로 만들면 모든 스레드에 의해 접근 가능해짐.
자식은 fork 오퍼레이션의 일부로 매핑과 세마포어를 자동으로 상속받는다.
*/
#include <pthread.h>
#include "pthread_func.h"
#include "tlpi_hdr.h"

static int gsum = 0;
static sem_t sem;

// unnamed 세마포어를 사용해서 쓰레드간 동기화 수행
static void *thread_func(void *arg)
{
	int *start = (int *)arg;
	int i;
	for(i = *start; i < *start + 100; i++) {
		if (sem_wait(&sem) == -1)
			errExit("sem_wait()");

		// thread 보호 구역
		gsum += i;

		if (sem_post(&sem) == -1)
			errExit("sem_post()");

	}
	return NULL;
}

void ipc_posix_unnamed_sem()
{
	pthread_t t1, t2;
	int s;
	int start1, start2;

/*
	pshared : 0이면 스레드간 공유를 허용함. 0이 아니면 프로세스간 공유된다.
	fork를 통해 생성된 자식은 부모의 메모리 매핑을 상속하기 때문에, 프로세스 공유 세마포어는 fork의 자식에 의해 상속된다.
	세마포어 초기값은 1
*/
	if(sem_init(&sem, 0, 1) == -1)
		errExit(s, "sem_init");

	start1 = 0;
	s = pthread_create(&t1, NULL, thread_func, &start1);
	if(s != 0)
		errExitEN(s, "pthread_create");

	start2 = 100;
	s = pthread_create(&t2, NULL, thread_func, &start2);
	if(s != 0)
		errExitEN(s, "pthread_create");

	s = pthread_join(t1, NULL);
	s = pthread_join(t2, NULL);

	printf("result = %d\n", gsum);
}


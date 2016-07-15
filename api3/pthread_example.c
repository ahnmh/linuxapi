/*
 * pthread_example.c
 *
 *  Created on: Jul 15, 2016
 *      Author: ahnmh-vw
 */
#include "pthread_example.h"
#include <stdio.h>
#include <unistd.h>

void *start_routine_create(void *arg)
{
	char *msg = (char *)arg;
	printf("%s\n", msg);

	return 0;
}

void create()
{
	pthread_t t1, t2;
	const char *msg1 = "Thread 1";
	const char *msg2 = "Thread 2";

	if(pthread_create(&t1, NULL, start_routine_create, (void *)msg1) != 0)
		errexit("pthread_create");

	if(pthread_create(&t2, NULL, start_routine_create, (void *)msg2) != 0)
		errexit("pthread_create");

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}

void *start_routine_exit_cancel(void *arg)
{
	int i;
	for (i = 0; i < 3; ++i) {
		printf("tid = %x, %d\n", (unsigned int)pthread_self(), i);
		sleep(2);
	}

	// 일반 프로세스에서 exit함수를 호출하는 것과 동일하다.
	pthread_exit((void *)100);
}

void exit_cancel()
{
	pthread_t t1, t2;

	if(pthread_create(&t1, NULL, start_routine_exit_cancel, NULL) != 0)
		errexit("pthread_create");

	if(pthread_create(&t2, NULL, start_routine_exit_cancel, NULL) != 0)
		errexit("pthread_create");

	// 작업
	sleep(3);

	// t1 스레드 취소
	// 5초 후에 취소요청하지만 정확히 6초 이후에 종료된다.
	// 스레드 루틴에서 sleep으로 진입하면 취소가 안되기 때문(재진입 가능함수가 아님)
	int ret;
	if((ret = pthread_cancel(t1)) > 0) {
		errexit("pthread_cancel");
	}

	int *retval;
	// t2 스레드의 종료를 대기하며 블록한다.
	pthread_join(t2, (void**)&retval);

	printf("%d\n", *retval);
}

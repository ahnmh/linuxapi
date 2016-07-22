/*
 * pthread_example.c
 *
 *  Created on: Jul 15, 2016
 *      Author: ahnmh-vw
 */
#include "pthread_example.h"

void *start_routine_create(void *arg)
{
	char *msg = (char *)arg;
	printf("%s\n", msg);

	return 0;
}

void create()
{
	pthread_t t1, t2, t3;
	const char *msg1 = "Thread 1";
	const char *msg2 = "Thread 2";
	const char *msg3 = "Thread 3";

	if(pthread_create(&t1, NULL, start_routine_create, (void *)msg1) != 0)
		errexit("pthread_create");

	if(pthread_create(&t2, NULL, start_routine_create, (void *)msg2) != 0)
		errexit("pthread_create");

	if(pthread_create(&t3, NULL, start_routine_create, (void *)msg3) != 0)
		errexit("pthread_create");

	// t1, t2, t3 스레드의 종료를 대기하며 블록한다.
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	// join 하지 않는 스레드는 명시적으로 detach를 수행하는 것이 좋다.
	pthread_detach(t3);
}

/*
스레드 종료
- 스레드 1개가 종료되는 상황
: start_routine이 리턴하는 경우
: pthread_exit를 호출하여 스스로 종료하는 경우. exit를 호출한 경우와 비슷하다.
: pthread_cancel 함수를 통해 다른 스레드에서 중지시킨 경우. kill을 통해 SIGKILL 시그널을 보낸 경우와 비슷하다.
- 스레드 전체가 종료되는 경우
: 프로세스의 main 함수 리턴
: 프로세스의 exit 함수 호출
: 프로세스가 execve 계열 함수 호출
*/

void *start_routine_exit_cancel(void *arg)
{
	int i;
	for (i = 0; i < 5; ++i) {
		printf("tid = %x, %d\n", (unsigned int)pthread_self(), i);
		sleep(2);
	}

	// 일반 프로세스에서 exit함수를 호출하는 것과 동일하다.
	// 쓰레드의 종료 코드를 전달하는 방법에 유의할 것!
	pthread_exit((void*)-1);
}

void exit_cancel()
{
	pthread_t t1, t2;

	if(pthread_create(&t1, NULL, start_routine_exit_cancel, NULL) != 0)
		errexit("pthread_create");

	if(pthread_create(&t2, NULL, start_routine_exit_cancel, NULL) != 0)
		errexit("pthread_create");

	// 작업
	sleep(5);

	// t1 스레드 취소
	// 5초 후에 취소요청하지만 정확히 6초 이후에 종료된다.(sleep 완료 후)
	int ret;
	if((ret = pthread_cancel(t1)) > 0) {
		errexit("pthread_cancel");
	}

	// t2 스레드의 종료를 대기하며 블록한다.
	// 쓰레드의 종료 코드를 받아오는 방법에 유의할 것!
	int retval;
	pthread_join(t2, (void**)&retval);

	printf("%d\n", retval);
}

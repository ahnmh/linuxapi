/*
 * ptherad_introduce.c
 *
 *  Created on: May 15, 2016
 *      Author: ahnmh
 */

#include <pthread.h>
#include "pthread_func.h"
#include "tlpi_hdr.h"

static void *thread_func(void *arg)
{
	char *str = (char*)arg;
	// 호출한 스레드의 thread id 구하기
	pthread_t tid = pthread_self();
	printf("thread id = 0x%x, arg = %s\n", (unsigned int)tid, str);

	// 리턴형이 void *인데 int 데이터를 리턴한다.
	return (void *)strlen(str);
}

void pthread_introduce()
{
/*
	thread id 는 커널의 TID와 다르다.
	id는 pthread의 다른 함수 호출시 해당 스레드를 가르키는데 사용된다.
*/
	pthread_t t1;
	void *retval; // 스레드 함수의 리턴
	int s;

	char *str = "hello thread";
/*
	스레드 생성. 성공하면 0, 실패하면 에러 번호를 리턴함.
	스레드 함수 원형 : (void *)(*thread_func)(void *)
*/
	s = pthread_create(&t1, NULL, thread_func, (void*)str);
	if(s != 0)
		errExitEN(s, "pthread_create");

/*
	생성한 스레드(t1)로 식별된 스레드가 종료되길 기다린다.(조인)
	이미 조인된 스레드에 대해 다시 조인할 수 없다.
*/
	s = pthread_join(t1, &retval);
	if(s != 0)
		errExitEN(s, "pthread_join");

	// 리턴형이 원래 void* 이지만 실제 리턴값은 정수형이므로 정수형태로 형변환한다.
	printf("thread id = 0x%x, ret = %ld\n", (unsigned int)t1, (long)retval);

/*
	생성한 스레드의 리턴에 관심이 없고,
	시스템이 자동으로 스레드를 제거하고 정리하길 원하는 경우 분리할 수 있다.
	pthread_detach(pthread_self())로 스스로 분리될 수도 있다.
	분리되면 더 이상 pthread_join으로 리턴 상태를 얻을 수 없고, 다시 조인될 수 없다.
	s = pthread_detach(t1);
	sleep(3);
*/
}

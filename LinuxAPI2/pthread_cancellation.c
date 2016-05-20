/*
 * pthread_cancel.c
 *
 *  Created on: May 20, 2016
 *      Author: ahnmh
 */
#include <stdio.h>
#include <pthread.h>
#include "pthread_func.h"
#include "tlpi_hdr.h"

/*
취소할 수 있는 스레드
취소 가능 지점: 주로 스레드를 블록하는 함수가 호출된 시점이다. 꼭 블록킹 함수만 취소 가능 지점은 아니다.
printf 역시 취소 가능 지점임.

계산만 하는 스레드는 취소 가능 지점이 없다. 이 경우 의도적으로 취소 가능 지점을 만들려면,
pthread_testcancel(); 함수를 호출해서 취소 가능 지점을 수동으로 만들 수 있다.
*/
static void *thread_func(void *arg)
{
	int i, s;
	char *arr;
	for (i = 0; i < 10; i++) {
		// 스레드 cancel을 불가하도록 만들 때 사용하는 함수
		// 다음 취소 가능 지점이 될때까지만 취소되지 않도록 한다. 따라서 취소 가능 지점이 계속 발생하는 경우 새로 호출해야 함.
		// s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		arr = malloc(100);
		sleep(1); // 스레드 취소 가능 지점
		printf("count = %d\n", i); // 스레드 취소 가능 지점
		free(arr);
	}

	return NULL;
}

// 클린업 핸들러 - 스레드가 중간에 취소된 경우 반드시 정리해야 하는 작업을 수행하기 위한 핸들러.
static void cleanup_func(void *arg)
{
	char *arr = (char*)arg;
	if(arr != NULL)
		free(arr);
	printf("cleanup function\n");
}

// 클린업 핸들러를 사용하는 취소 가능한 스레드
static void *thread_func_cleanup(void *arg)
{
	int i, s;
	char *arr;
	for (i = 0; i < 10; i++) {
		// 동적 메모리 할당은 스레드가 취소되는 경우 반드시 어디선가 해제되어야 한다.
		arr = malloc(100);

		// 스레드가 취소되었을때 호출되는 클린업 핸들러를 설치한다.
		pthread_cleanup_push(cleanup_func, arr);
		sleep(1); // 스레드 취소 가능 지점
		printf("count = %d\n", i); // 스레드 취소 가능 지점

		// 스레드가 취소되지 않는 경우 클린업 핸들러를 해제한다. 설치와 핸들러는 반드시 같은 블록 안에 있어야 함.
		// 0이면 클린업 핸들러를 삭제하고, 0이 아니면 무조건 클린업 핸들러를 실행함.
		pthread_cleanup_pop(0);

		if(arr != NULL)
			free(arr);
	}

	return NULL;
}

void pthread_cancellation()
{
	pthread_t t1;
	void *retval;
	int s;

//	s = pthread_create(&t1, NULL, thread_func, NULL);
	s = pthread_create(&t1, NULL, thread_func_cleanup, NULL);

	// 스레드가 한동안 수행되도록 함.
	sleep(3);

	// 스레드를 취소 가능 지점에 도달했을때 취소하게 한다.
	s = pthread_cancel(t1);
	s = pthread_join(t1, &retval);

	// 스레드가 취소되어, pthread_join 함수의 대기가 끝날 때 2번째 파라미터를 PTHREAD_CANCELED로 설정한다.
	if(retval == PTHREAD_CANCELED)
		printf("Thread was canceled\n");

}

/*
 * pthread_sync.c
 *
 *  Created on: May 15, 2016
 *      Author: ahnmh
 */

#include <pthread.h>
#include "pthread_func.h"
#include "tlpi_hdr.h"

static int gsum = 0;
/*
정적으로 할당된 뮤텍스를 기본 속성으로 초기화하는 방법. 이 경우 pthread_mutex_destroy는 호출할 필요 없음.
스택(자동)이나 동적으로 할당된 뮤텍스의 경우, pthread_mutex_init() 함수를 사용해야 한다.
자동이나 동적으로 할당된 뮤텍스는 pthread_mutex_destroy()를 사용해서 제거해야 함.
*/
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *thread_func(void *arg)
{
	int *start = (int *)arg;
	int i;
	for(i = *start; i < *start + 100; i++) {
		pthread_mutex_lock(&mutex);
		gsum += i;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void pthread_sync()
{
	pthread_t t1, t2;
	int s;
	int start1, start2;

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






/*
에러 체킹을 하는 뮤텍스 속성을 부여하고 초기화하는 방법
뮤텍스를 지역변수로 선언하는 방법
*/

typedef struct _param {
	int start;
	pthread_mutex_t *mtx;
}param;

static void *thread_func_error_checking_mutex(void *arg)
{
	param *p = (param *)arg;
	int i;
	int s;
	for(i = p->start; i < p->start + 100; i++) {
		s = pthread_mutex_lock(p->mtx); // 에러가 발생하면 에러 번호를 리턴한다.
		if(s != 0)
			errExitEN(s, "pthread_mutex_lock");
		gsum += i;
		s = pthread_mutex_unlock(p->mtx); // 에러가 발생하면 에러 번호를 리턴한다.
		if(s != 0)
			errExitEN(s, "pthread_mutex_unlock");
	}
	return NULL;
}

void pthread_sync_error_checking_mutex()
{
	pthread_t t1, t2;
	int s;

	pthread_mutex_t mtx;
	pthread_mutexattr_t mtx_attr;

	// 뮤텍스 속성 구조체 초기화
	s = pthread_mutexattr_init(&mtx_attr);
/*
	뮤텍스 타입으로 PTHREAD_MUTEX_ERRORCHECK를 사용 :
	잘못된 뮤텍스 사용시 에러를 리턴함. 일반 뮤텍스보다 느리지만 어디서 뮤텍스 사용 규칙을 위한하는지 알 수 있다.
*/
	s = pthread_mutexattr_settype(&mtx_attr, PTHREAD_MUTEX_ERRORCHECK);

	s = pthread_mutex_init(&mtx, &mtx_attr);

	param p1, p2;

	// 2개의 스레드가 같은 mutex(메인 스레드의 지 변수로 선언된)를 사용
	p1.mtx = &mtx;
	p1.start = 0;
	s = pthread_create(&t1, NULL, thread_func_error_checking_mutex, &p1);
	if(s != 0)
		errExitEN(s, "pthread_create");

	p2.mtx = &mtx;
	p2.start = 100;
	s = pthread_create(&t2, NULL, thread_func_error_checking_mutex, &p2);
	if(s != 0)
		errExitEN(s, "pthread_create");

	s = pthread_join(t1, NULL);
	s = pthread_join(t2, NULL);


	s = pthread_mutexattr_destroy(&mtx_attr);
	s = pthread_mutex_destroy(&mtx);

	printf("result = %d\n", gsum);
}

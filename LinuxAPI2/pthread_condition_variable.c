/*
 * pthread_condition_variable.c
 *
 *  Created on: May 15, 2016
 *      Author: ahnmh
 */

#include <pthread.h>
#include "pthread_func.h"
#include "tlpi_hdr.h"

static int work = 0;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; // 전역 뮤텍스 초기화
static pthread_cond_t con = PTHREAD_COND_INITIALIZER; // 전역 조건변수 초기화

static void *thread_func(void *arg)
{
	int * sec = (int *)arg;

	int i;
	for(i = 0; i < 1000; i++) {
		// 조건변수가 필요한 상황을 시뮬레이션
		sleep(*sec);
		// 뮤텍스를 얻어서 work를 추가한다.
		pthread_mutex_lock(&mtx);
		work++;
		printf("thread(tid: %x) make a work = %d\n", (unsigned int)pthread_self(), work);
		pthread_mutex_unlock(&mtx);

		// 조건 변수를 시그널한다.
		pthread_cond_signal(&con);
	}

	// 작업 완료 후  스스로 detach를 수행함.
	pthread_detach(pthread_self());

	return NULL;
}

void pthread_cond_var()
{
	pthread_t t1, t2;
	int s;
	int sec1, sec2;

	sec1 = 1;
	s = pthread_create(&t1, NULL, thread_func, &sec1);
	if(s != 0)
		errExitEN(s, "pthread_create");

	sec2 = 2;
	s = pthread_create(&t2, NULL, thread_func, &sec2);
	if(s != 0)
		errExitEN(s, "pthread_create");

	pthread_mutex_lock(&mtx);

	while(1) {
/*
		work가 0이면 계속 대기한다.
		조건변수가 시그널될때까지 뮤텍스를 해제하고 기다림.
		조건변수를 사용하지 않는다면? 뮤텍스를 얻고 해제하는 과정을 무수히 반복해야 함.
		즉, 뮤텍스를 얻어도 work가 갱신되지 않으므로(1초나 2초에 한번씩 갱신되므로) 그냥 뮤텍스를 해제하고 끝나게 됨. 이 과정을 반복.
*/
/*
		if가 아닌 while을 사용하는 이유 :
		조건변수를 기다리는 다른 스레드가 먼저 깨어나서 work를 모두 소비할 수도 있기 때문
		이 경우 work가 발생해서 if 아래로 내려가더라도 할 일이 없게 됨.
*/
		while(work == 0)
			pthread_cond_wait(&con, &mtx);

		// work가 발생하면(조건 변수가 시그널되면) 뮤텍스를 얻고 work를 소비한다.
		while(work > 0) {
			work--;
			printf("main thread consumed a work = %d\n", work);
		}
	}

}

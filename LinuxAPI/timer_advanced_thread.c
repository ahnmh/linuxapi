/*
 * timer_advanced_thread.c
 *
 *  Created on: Apr 26, 2016
 *      Author: ahnmh
 */
#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "timer_func.h"
#include "tlpi_hdr.h"

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; // pthread_mutex_init()
static pthread_cond_t con = PTHREAD_COND_INITIALIZER; // pthread_cond_init()
static volatile int expire_cnt = 0;


// 스레드를 사용한 타이머 통지
static void thread_func(sigval_t sv)
{
	timer_t *ptid = sv.sival_ptr;
	printf("tid: %ld\n", (long)*ptid);

	pthread_mutex_lock(&mtx);
	expire_cnt++;
	pthread_mutex_unlock(&mtx);

	pthread_cond_signal(&con);


}

void timer_advanced_thread()
{
	struct sigaction sa;
	// 2개의 타이머
	timer_t tid[2];

	// 쓰레드을 이용하는 타이머 생성 - 타이머 만료시 쓰레드가 생성됨.
	struct sigevent sv;
	sv.sigev_notify = SIGEV_THREAD;
	sv.sigev_notify_function = thread_func;
	sv.sigev_notify_attributes = NULL;

	// 2개의 타이머 생성
	int i;
	for (i = 0; i < 2; i++) {
		sv.sigev_value.sival_ptr = &tid[i];
		if(timer_create(CLOCK_REALTIME, &sv, &tid[i]) == -1)
			errExit("timer_create()");
		printf("TImer Id: %ld\n", (long)tid[i]);
	}

	// 첫번째 타이머 설정
	struct itimerspec ts;
	ts.it_value.tv_sec = 3; // 타이머의 최초 만료 시간
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 2; // 타이머의 주기적 만료 시간
	ts.it_interval.tv_nsec = 0;
	if(timer_settime(tid[0], 0, &ts, NULL) == -1)
		errExit("timer_settime(): %ld", (long)tid[0]);

	// 두번째 타이머 설정
	ts.it_value.tv_sec = 5; // 타이머의 최초 만료 시간
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 3; // 타이머의 주기적 만료 시간
	ts.it_interval.tv_nsec = 0;
	if(timer_settime(tid[1], 0, &ts, NULL) == -1)
		errExit("timer_settime(): %ld", (long)tid[1]);


	pthread_mutex_lock(&mtx);
	while(1) {
/*
		pthread_cond_wait는 pthread_mutex_init이 호출되고 mutex가 lock인 상태에서 호출되어야 한다.
		pthread_cond_wait는 lock 상태인 mutex를 unlock하고 호출 스레드의 블록킹을 수행한다.
		조건 변수가 시그널되면 mutex를 lock하고 호출 스레드의 블록을 해제한다.
		따라서 조건 변수가 시그널된 이후, mutex를 unlock할때까지 Thread-safe를 보장한다.

		아래 expire_cnt를 체크하는 부분은 expire_cnt가 thread_func 함수에서 증가가 있기 전까지는
		수행되어서는 안된다.(블록되어 있어야 한다) 따라서, expire_cnt의 증가가 발생한 이후,
		pthread_cond_signal 함수에 의해 조건 변수가 시그널되면 이 때 expire_cnt를 체크하도록
		동작하기 위해서 pthread_cond_wait를 써서 대기하는 것임.

*/
		pthread_cond_wait(&con, &mtx);
		printf("Expire count: %d\n", expire_cnt);
		pthread_mutex_unlock(&mtx);
	}


}


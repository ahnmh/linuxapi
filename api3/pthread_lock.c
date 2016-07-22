/*
 * pthread_mutex.c
 *
 *  Created on: Jul 16, 2016
 *      Author: ahnmh
 */
#include "pthread_example.h"

struct tdata {
	int value;
	pthread_mutex_t mutex;
};

// 뮤텍스를 통해 크리티컬 섹션에 해당하는 pdata를 보호한다.
// 락을 걸때는 아래와 같이 될 수 있으면 글로벌 락을 피하고 데이터 구조의 특정 인스턴스와 관련된 락을 사용해야 한다.
// 아래의 경우 글로벌 락을 걸게 되면 경쟁 관계에 없는 data1도 t2,t3 스레드에 의해 data2가 뮤텍스 락이 걸리면 대기해야 함.
void *start_routine_lock(void *arg)
{
	struct tdata *pdata = (struct tdata *)arg;

	pthread_mutex_lock(&pdata->mutex);
	int i = 0;
	for (i = 0; i < 10000; ++i) {
		pdata->value += i;
	}
	pthread_mutex_unlock(&pdata->mutex);

	return 0;
}

void mutex_example()
{
	pthread_t t1, t2, t3;

	// 뮤텍스를 선언과 동시에 초기화할 수 있는 경우 아래와 같이 초기화 가능
	struct tdata data1 = {0, PTHREAD_MUTEX_INITIALIZER};
	struct tdata data2 = {0, PTHREAD_MUTEX_INITIALIZER};
/*
	// 그렇지 않은 경우, pthread_mutex_init 함수를 통해 명시적으로 초기화. mutexattr은 NULL로 지정 가능
	data1.value = 0;
	pthread_mutex_init(&data1.mutex, NULL);
	data2.value = 0;
	pthread_mutex_init(&data2.mutex, NULL);
*/

	// t1 스레드는 data1에 대해 혼자 접근하므로 경쟁 관계가 아님.
	if(pthread_create(&t1, NULL, start_routine_lock, (void *)&data1) != 0)
		errexit("pthread_create");

	// t2, t3 스레드는 data2에 대해 서로 경쟁 관계이므로 뮤텍스를 통해 보호해야 한다.
	if(pthread_create(&t2, NULL, start_routine_lock, (void *)&data2) != 0)
		errexit("pthread_create");

	if(pthread_create(&t3, NULL, start_routine_lock, (void *)&data2) != 0)
		errexit("pthread_create");

	// t1, t2, t3 스레드의 종료를 대기하며 블록한다.
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	printf("data1.value = %d\n", data1.value);
	printf("data2.value = %d\n", data2.value);
}


pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t con = PTHREAD_COND_INITIALIZER;
static int job = 0;

void *start_routine_cond(void *arg)
{
	int *period = (int *)arg;
	int i;
	for (i = 0; i < 10; ++i) {
		sleep(*period);
		// job을 추가하고 소비하는 과정은 뮤텍스를 통해 Thread-safe하게 수행해야 한다.
		pthread_mutex_lock(&mtx);
		job++;
		printf("thread(tid: %x) add a job = %d\n", (unsigned int)pthread_self(), job);
		pthread_mutex_unlock(&mtx);

		// 작업이 추가됬으므로 조건 변수를 시그널하여 작업을 대기하는 스레드를 깨움.
		pthread_cond_signal(&con);
	}

	return 0;
}

void mutex_cond_example()
{
	pthread_t t1, t2;
	int period_t1 = 1;
	int period_t2 = 2;

	if(pthread_create(&t1, NULL, start_routine_cond, (void *)&period_t1) != 0)
		errexit("pthread_create");

	if(pthread_create(&t2, NULL, start_routine_cond, (void *)&period_t2) != 0)
		errexit("pthread_create");

	// pthread_cond_wait을 최초 실행하면 뮤텍스를 해제하는 동작을 수행하므로 그 전에 먼저 뮤텍스를 얻는다.
	pthread_mutex_lock(&mtx);

	while(1) {
		// pthread_cond_wait
		// 조건변수가 시그널될때까지 뮤텍스를 해제하고 기다림.
		// 조건변수가 시그널되면 뮤텍스를 얻고 리턴함. 뮤텍스를 얻었으므로 Thread-safe한 작업(여기서는 work를 감소시키는 일)을 수행할 수 있음.
		// 조건변수를 사용하지 않으면 작업이 추가됬는지 계속 검사해야 함.
		while(job == 0) // job을 소비하는 다른 스레드가 있다면 해당 스레드에 의해 job이 모두 소비되고 계속 0일 수 있으므로 while을 사용함.
			pthread_cond_wait(&con, &mtx);

		// job을 소비한다. pthread_cond_wait에 의해 뮤텍스를 얻었으므로 Thread-safe하다.
		while(job) {
			printf("job consuming... %d\n", job);
			job--;
		}
	}

}

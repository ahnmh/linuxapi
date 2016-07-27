/*
 * timer_example.c
 *
 *  Created on: Jul 27, 2016
 *      Author: ahnmh-vw
 */
#include "time_timer.h"
#include <signal.h>

static void signal_handler_adv(int sig, siginfo_t *si, void *ctx)
{
	printf("caught signal = %s\n", sys_siglist[sig]);

	printf("signal code = %d", si->si_code);
	switch(si->si_code) {
	case SI_KERNEL:
		printf("(SI_KERNEL)\n"); // 타이머의 의한 경우 si_code
		break;
	case SI_MESGQ:
		printf("(SI_MESGQ)\n");
		break;
	case SI_QUEUE:
		printf("(SI_QUEUE)\n");
		break;
	case SI_USER:
		printf("(SI_USER)\n");
		break;
	case SI_TIMER:
		printf("(SI_TIMER)\n");
		break;
	case SI_SIGIO:
		printf("(SI_SIGIO)\n");
		break;
	}

	int *p = (int *)si->si_value.sival_ptr;
	printf("sig_val = %d\n", *p);
}

void timer_example_alarm()
{
	// SIGALRM 시그널 핸들러 설치
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signal_handler_adv;

	if(sigaction(SIGALRM, &sa, NULL) == -1)
		errexit("sigaction");

	// 2초 이후에 SIGALRM 시그널을 발생시킨다.
	alarm(2);

	pause();
}

// 인터벌 타이머
void timer_example_setitimer()
{
	// SIGALRM 시그널 핸들러 설치
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signal_handler_adv;

	if(sigaction(SIGALRM, &sa, NULL) == -1)
		errexit("sigaction");

	// 타이머 시간 설정 구조체
	struct itimerval delay;
	delay.it_value.tv_sec = 5; // 최초 만료 시간은 5초
	delay.it_value.tv_usec = 0;
	delay.it_interval.tv_sec = 1; // 주기적 만료 시간은 1초
	delay.it_interval.tv_usec = 0;

/*
	타이머 설정
	which:
	- ITIMER_REAL: 실제 시간 측정, 지정한 시간 경과하면 SIGALRM 발생함.(alarm과 같은 방식)
	- ITIMER_VIRTUAL: 프로세스의 사용자 영역 코드가 수행되는 동안에만 타이머가 흘러감. SIGVTALRM 시그널을 보낸다.
	- ITIMER_PROF: 프로세스가 실행 중이거나 커널이 프로세스를 대신에서 실행중인 경우에만 타이머 시간이 흘러감. SIGPROF 시그널을 보낸다.
*/
	if(setitimer(ITIMER_REAL, &delay, NULL) == -1)
		errexit("setitimer");

	// 주기적인 타이머 만료를 보기 위해 5회 반복
	int i;
	for (i = 0; i < 5; ++i) {
		pause();
	}

	// it_interval 값을 0으로 설정하면 타미어가 중지됨.
	// it_value 값을 0으로 설정하면 타이머가 종료됨
	delay.it_value.tv_sec = 0;
	if(setitimer(ITIMER_REAL, &delay, NULL) == -1)
		errexit("setitimer");
}

// POSIX 시계를 사용한 타이머 - 시그널 타입
void timer_example_settime_signal()
{
	// SIGALRM 시그널 핸들러 설치 - timer_create의 sigevent 파라미터가 NULL이거나 evp.sigev_notify = SIGEV_SIGNAL인 경우 필요.
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signal_handler_adv;

	if(sigaction(SIGUSR1, &sa, NULL) == -1)
		errexit("sigaction");

	// 타이머 생성하는 여러가지 방법
	timer_t timer; // 고유한 타이머 식별자

/*

	// sigevent evp 파라미터가 NULL이면, notify 타입이 SIGEV_SIGNAL로 지정되어 SIGALRM 시그널이 발생하며 setitimer와 같이 별도로 시그널 핸들러를 설치해주어야 한다.(위에 주석처리한 부분)
	if(timer_create(
			CLOCK_REALTIME, // POSIX 시계를 지정
			NULL, // sigevent 구조체. NULL이 아닌 경우 타이머 만료시 발생하는 통지를 지정할 수 있다.(시그널 핸들러 또는 스레드)
			&timer
			) == -1)
		errexit("timer_create");

*/
	// sigevent evp 파라미터가 NULL이 아니고 notify 타입이 SIGEV_SIGNAL인 경우. 시그널 타입을 SIGUSR1로 사용(임의 지정도 가능함)
	struct sigevent evp;
	evp.sigev_value.sival_ptr = &timer; // 시그널 핸들러로 넘길 데이터(sigval_t) 지정. siginfo_t.si_value로 받을 수 있음.
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR1;
	if(timer_create(CLOCK_REALTIME, &evp, &timer) == -1)
		errexit("timer_create");

	// 타이머 설정
	struct itimerspec ts;
	ts.it_value.tv_sec = 5; // 최초 만료 시간
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 1; // 이후 주기적인 만료 시간
	ts.it_interval.tv_nsec = 0;
	if(timer_settime(
			timer, // 타이머 식별자
			0, // flags: TIMER_ABSTIME을 지정하면 절대 시간으로 해석함.
			&ts,
			NULL
			) == -1)
		errexit("timer_settime");

	int i;
	for (i = 0; i < 5; ++i) {
		pause();
	}

	// 타이머 제거
	if(timer_delete(timer) == -1)
		errexit("timer_delete");
}

#include <pthread.h>
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t con = PTHREAD_COND_INITIALIZER;

static volatile int loop_count = 0;

static void thread_func(sigval_t sv)
{
	int *p = (int *)sv.sival_ptr;
	printf("timer expired, sig_val = %d\n", *p);

	// critical section
	pthread_mutex_lock(&mtx);
	loop_count++;
	pthread_mutex_unlock(&mtx);
	pthread_cond_signal(&con);

}

// POSIX 시계를 사용한 타이머 - 스레드 타입
void timer_example_settime_thread()
{
	// 타이머 생성하는 여러가지 방법
	timer_t timer; // 고유한 타이머 식별자

	// sigevent evp 파라미터가 NULL이 아니고 notify 타입이 SIGEV_THREAD인 경우. 시그널을 사용하지 않으므로 시그널 핸들러를 설치할 필요 없다.
	// 타이머 만료시 sigev_notify_function로 지정한 스레드가 실행된다.
	// 스레드 타입: void (*_function) (sigval_t)
	struct sigevent evp;
	evp.sigev_value.sival_ptr = &timer; // 스레드로 넘길 데이터(sigval_t) 지정
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = thread_func;
	evp.sigev_notify_attributes = NULL;
	if(timer_create(CLOCK_REALTIME, &evp, &timer) == -1)
		errexit("timer_create");

	// 타이머 설정
	struct itimerspec ts;
	ts.it_value.tv_sec = 5; // 최초 만료 시간
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 1; // 이후 주기적인 만료 시간
	ts.it_interval.tv_nsec = 0;
	if(timer_settime(
			timer, // 타이머 식별자
			0, // flags: TIMER_ABSTIME을 지정하면 절대 시간으로 해석함.
			&ts,
			NULL
			) == -1)
		errexit("timer_settime");

	// 타이머 만료시 스레드 실행하는 경우 pause 함수를 사용할 수 없다. pause 함수는 시그널에 대기하는 함수이기 때문임.
	// 아래와 같이 뮤텍스와 조건 변수를 사용하여 타이머 만료로 인한 스레드의 수행 횟수를 카운트하여 반복을 중지한다.
	pthread_mutex_lock(&mtx);
	while(1) {
		// 조건변수가 pthread_cond_signal에 의해 시그널될 때까지 대기한다.
		// pthread_cond_wait는 뮤텍스를 release하고 조건변수가 시그널되길 기다림
		// 조건 변수가 시그널되면 뮤텍스를 lock하고 함수를 리턴함.
		pthread_cond_wait(&con, &mtx);
		// 조건변수가 시그널되면 loop_count가 증가했음을 의미한다.
		if(loop_count > 5) {
			break;
		}
		printf("loop_count = %d\n", loop_count);
	}

	pthread_mutex_unlock(&mtx);

	// 타이머 제거
	if(timer_delete(timer) == -1)
		errexit("timer_delete");
}

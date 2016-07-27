/*
 * time_sleep.c
 *
 *  Created on: Jul 26, 2016
 *      Author: ahnmh-vw
 */
#define __USE_POSIX199309
#include "time_timer.h"

#include <signal.h>
#include <errno.h>
#include <sys/select.h>

static void signal_handler_adv(int sig, siginfo_t *si, void *ctx)
{
	printf("caught signal = %s\n", sys_siglist[sig]);
	printf("signal code = %d\n", si->si_code);
}

/*
일부 유닉스 시스템은 SIGALRM을 사용하여 sleep, usleep 함수를 구현한다.
alarm, settimer 함수도 SIGALRM을 사용하기 때문에 이러한 함수들을 중첩해서 사용해선 안된다.
잠깐 동안 기다리는 경우 SIGALRM을 사용하지 않는 nanosleep 함수를 사용해야 한다.
*/
void time_sleep()
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signal_handler_adv;

	if(sigaction(SIGINT, &sa, NULL) == -1)
		errexit("sigaction");

	// 위와 같이 시그널 핸들러가 설치되어 있을 때 SIGINT가 발생하면 시그널 핸들러 호출 이후, sleep 함수로 복귀하고,
	// sleep 함수는 리턴되며 리턴값은 남은 시간에 해당한다.
	// 시그널 핸들러가 설치되어 있지 않으면 시그널의 기본 처리 방식에 따라 프로그램이 종료됨.
	int remain = sleep(10);
	printf("remain = %d\n", remain);

	struct timespec req, rem;
	req.tv_sec = 10;
	req.tv_nsec = 0;

	// nsec 단위로 sleep을 수행한다.
	// 시그널에 의해 인터럽트되면 -1을 리턴하고 errno를 EINTR로 설정한다.
	if(nanosleep(&req, &rem) == -1) {
		perror("nanosleep"); // 시그널에 의해 인터럽트되었을때도 -1을 리턴하므로 errexit로 종료해버리면 남은 시간을 구할 수 없음.
		if(errno == EINTR) {
			printf("remain sec = %ld\n", rem.tv_sec);
			printf("remain nsec = %ld\n", rem.tv_nsec);

		}
	}

	// select를 사용한 잠들기: 이식성이 높다.
	struct timeval tv = {10, 500}; // 10.5s
	select(0, NULL, NULL, NULL, &tv);

}

// POSIX 시계를 통한 고급 잠들기 기법
void time_clock_nanosleep()
{
	// 상대 시간 잠들기
	struct timespec ts;
	ts.tv_sec = 1;
	ts.tv_nsec = 500000000;

	if(clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL) == -1)
		errexit("clock_nanosleep");


	// 절대 시간 잠들기

	// 현재 시간을 구한다.
	struct timespec ts2;
	if(clock_gettime(CLOCK_MONOTONIC, &ts2) == -1)
		errexit("clock_gettime");

	// 정확히 1초 동안 잠든다.
	// 페이지 폴트가 나거나 sleep 이전에 컨텍스트 스위칭이 발생해서 시간이 경과하면 의도한 시간보다 더 길게 잠들 수도 있는 부분을 제거한다.
	ts2.tv_sec += 1;
	if(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts2, NULL) == -1)
		errexit("clock_nanosleep");

}

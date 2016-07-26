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

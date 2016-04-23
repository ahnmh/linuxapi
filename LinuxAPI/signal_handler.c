/*
 * signal_handler.c
 *
 *  Created on: Apr 20, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>

#include "tlpi_hdr.h"
#include "signal_func.h"

// 변수의 현재값을 캐시하지 않도록 하고 메모리에 상주하도록 volatile로 지정함.
// 정수 데이터 sig_atomic_t는 시그널 핸들러와 메인함수 간 읽기와 쓰기가 아토믹함을 보장한다.
static volatile sig_atomic_t canjump = 0;
static sigjmp_buf senv;

void print_sigset(const sigset_t *sigset)
{
	int sig;
	int cnt = 0;
	for (sig = 1; sig < NSIG; sig++) {
		if(sigismember(sigset, sig)) {
			cnt++;
			printf("%d(%s)\n", sig, strsignal(sig));
		}
	}

	if(cnt == 0)
		printf("Empty signal set\n");
}

int print_sigmask()
{
	sigset_t current_mask;
	// 현재 마스크값을 구함
	if(sigprocmask(SIG_BLOCK, NULL, &current_mask) == -1)
		return -1;

	print_sigset(&current_mask);
	return 0;
}

void signal_handler_func(int sig)
{
	// 아래에서 현재 마스크 상태인 시그널을 확인해보면 현재 전달된 시그널이 포함됨을 알 수 있다.
	// 즉, 핸들러를 호출한 시그널은 핸들러가 종료되기 전까지 마스크 상태를 유지함.
	printf("signal mask at handler: \n");
	if(print_sigmask() == -1)
		errMsg("print_sigmask");

	printf("signal is %d(%s).\n", sig, strsignal(sig));

	// 아직 sigsetjmp가 set되기 전이므로 점프할 수 없다!
	if(!canjump) {
		printf("env buffer not yet set.\n");
		return;
	}
	else
		// sigsetjmp가 호출된 이후라서 롱점프가 가능하면, 시그널 핸들러에서 signal_longjmp()로 점프한다.
		// 이 때 senv에 저장해두었던 마스크 값을 복원해준다.
		// 따라서 핸들러가 완료되지 않더라도(롱점프하면 중간에 나가니까)
		// 현재 시그널이 마스크 상태로 남게될 걱정을 안해도 된다.
		siglongjmp(senv, 1);
}

void signal_longjmp()
{
	struct sigaction sa;

	printf("signal mask at start: \n");
	if(print_sigmask() == -1)
		errMsg("print_sigmask");

	// 핸들러가 호출될 때 블록될 시그널을 위한 마스크
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = signal_handler_func;
	// SIGINT를 위한 시그널 핸들러를 등록함.
	// 새로운 속성을 부여하는 경우 세번째 파라미터에 NULL을 지정할 수 있다.
	// 기존 속성을 알려면 두번째 파라미터를 NULL로 설정한다.
	if(sigaction(SIGINT, &sa, NULL) == -1)
		errExit("sigaction\n");

/*
	 현재의 시그널 마스크 상태는 senv 에 백업된다.(두번째 파라미터가 1이면 저장, 0이면 저장안함)
	 setjmp는 현재의 스택 컨텍스트와 시그널 마스크를 senv에 저장해두었다가 longjmp를 통해 현재 흐름으로 복귀했을 때
	 senv를 통해 복구된다.
*/
	if(sigsetjmp(senv, 1) == 0) {
		canjump = 1;
	}
	else {
		// 롱점프하게 되면 여기로 넘어온다.
		printf("After jump from handler, signal mask is: \n");
		if(print_sigmask() == -1)
			errMsg("print_sigmask");
	}

	for(;;)
		pause();

}

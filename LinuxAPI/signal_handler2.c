/*
 * signal_handler2.c
 *
 *  Created on: Apr 22, 2016
 *      Author: ahnmh
 */


#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <fcntl.h>

#include "tlpi_hdr.h"
#include "signal_func.h"
#define BUF_SIZE 4096

void signal_handler_func2(int sig)
{
	printf("signal is %d(%s).\n", sig, strsignal(sig));
}

void signal_restart()
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
/*
	read와 같은 시스템 호출이 입력을 대기하기 위해 블록되는 상태(stdin으로부터 입력을 받는 것과 같은)에서
	시그널을 전달받으면, 시그널 핸들러에 의해 해당 시스템 호출은 인터럽트되어 EINTR(Interrupted system call) 에러와 함께 -1이 리턴된다.

	사용자 입력을 대기하고 있는데 시그널에 의해 시스템 콜이 의도치 않은 리턴을 하면 안될 것이다.

	따라서, sigaction 호출시 SA_RESTART 플래그를 부여해서 해당 시그널을 받으면 시그널 핸들러에 의해 시스템 콜이 인터럽트 되지 않고,
	실행을 계속하도록 할 수 있다.

	단, 모든 시스템 호출이 재시작을 지원하지는 않는다. 지원 가능 함수의 예: read, wait, write, ioctl, accept, send, recv, ...
*/

//	sa.sa_flags = 0; // SA_RESTART가 없는 경우, 시그널을 받으면 아래 51번줄 read 호출은 EINTR와 함께 리턴된다.
	sa.sa_flags = SA_RESTART; // SA_RESTART가 있는 경우, 시그널을 받으면 시그널 핸들러를 호출하고 read 호출이 계속된다.
	sa.sa_handler = signal_handler_func2;
	if(sigaction(SIGINT, &sa, NULL) == -1)
		errExit("sigaction\n");

	int cnt;
	char buf[BUF_SIZE];

	/* EINTR: Interrupted system call */
	cnt = read(0, buf, BUF_SIZE);

/*
	시그널에 SA_RESTART 플래그를 부여하지 않고도 아래와 같이 read를 호출하게 되면, 시그널에 의해
	시스템 호출이 인터럽트가 걸려도 계속 수행할 수 있게 된다.
*/
//	while((cnt = read(0, buf, BUF_SIZE)) == -1 && errno == EINTR)
//		continue;

	printf("read size = %d\n", cnt);

	if(cnt == -1)
		errExit("read");

}

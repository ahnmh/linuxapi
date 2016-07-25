/*
 * signal_example.c
 *
 *  Created on: Jul 22, 2016
 *      Author: ahnmh-vw
 */

#include "signal_example.h"
#include <sys/wait.h>

static void signal_handler(int signo)
{
	if(signo == SIGINT)
		// sys_siglist 배열을 사용하면 시그널 번호에 맞는 문자열을 가져올 수 있다.
		printf("pid = %d, caught signal = %s\n", getpid(), sys_siglist[SIGINT]);
}

/*기본적인 시그널 처리기인 signal 함수를 사용하는 예제*/
void signal_legacy()
{
	// SIGINT를 처리하는 시그널 핸들러를 등록한다.
	if(signal(SIGINT, signal_handler) == SIG_ERR)
		errexit("signal");

	// SIGQUIT(ctrl + \)를 무시하도록 설정한다.
	if(signal(SIGQUIT, SIG_IGN) == SIG_ERR)
		errexit("signal");

	// SIGTERM을 기본 처리기로 처리하도록(프로세스 종료) 설정한다.
	if(signal(SIGTERM, SIG_DFL) == SIG_ERR)
		errexit("signal");

	// 시그널의 최대값 NSIG

	// pause: 프로세스를 종료시키는 시그널을 받을 때까지 해당 프로세스를 블록한다.
	while(1)
		pause();
}

/*
시그널 동작별 상속
동작					fork				exec
----------------------------------------------------
무시					상속됨				상속됨
기본					상속됨				상속됨
처리					상속됨				상속되지 않음(exec로 실행되는 프로세스가 이전 프로세스와 동일한 시그널 핸들러를 가지고 있지 않기 때문)
대기중인 시그널		상속되지 않음			상속되지 않음
*/

// 프로세스 또는 프로세스 그룹에 시그널을 전달하는 예제
void signal_send()
{
	pid_t pid;

	// SIGINT를 처리하는 시그널 핸들러를 부모 자식 모두 등록한다.(상속되므로)
	if(signal(SIGINT, signal_handler) == SIG_ERR)
		errexit("signal");

	switch(pid = fork()) {
	case -1:
		errexit("fork");
		break;

	case 0:
		printf("child = %d\n", getpid());
		// 시그널 대기
		pause();
		break;

	default:
		printf("parent = %d\n", getpid());
		sleep(5);

/*
		pid: 특정 프로세스에 시그널을 보냄
		0: 호출한 프로세스의 프로세스 그룹(자식 포함)에 속한 모든 프로세스에 시그널을 보냄.
		-pid: 특정 프로세스 그룹에 속한 모든 프로세스에 시그널을 보냄.(killpg 함수를 써도 된다)
*/
		if(kill(pid, SIGINT) == -1)
			errexit("kill");

		// 자식의 종료에 대기함.
		if(waitpid(pid, NULL, 0) == -1)
			errexit("waitpid");

		// 자기자신에게 시그널을 호출함.
		raise(SIGINT);

		break;
	}

}

void signal_helper()
{
	// 시그널 집합
	sigset_t set;

	// 시그널 집합을 비어 있는 상태로 초기화한다.
	sigemptyset(&set);
	// 시그널 집합에 SIGINT를 추가한다.
	sigaddset(&set, SIGINT);
	// 시그널 집합에 SIGINT를 제거한다.
	sigdelset(&set, SIGINT);
	// 시그널 집합을 가득차 있는 상태로 초기화한다.
	sigfillset(&set);
	// 시그널 집합에 SIGINT가 포함되어 있는지 확인한다.
	if(sigismember(&set, SIGINT))
		printf("SIGINT is included in set\n");
}

/*
시그널 블록:
시그널 핸들러는 아무때나 비동기적으로 동작하기 때문에
시그널 핸들러와 프로그램의 다른 부분이 데이터를 공유해야하는 경우 시그널을 블록해야 할 수 있다.
블록되는 동안 발생하는 어떤 시그널도 블로깅 해제되기 전까지 처리되지 않는다.
*/
void signal_block()
{
	sigset_t set;

	// 빈 시그널 집합에 SIGINT를 추가한다.
	sigemptyset(&set);
	sigaddset(&set, SIGINT);

	// 블록할 시그널 집합(시그널 마스크)을 설정한다.
	// SIG_SETMASK: 프로세스의 시그널 마스크를 set 집합으로 변경한다.
	// SIG_BLOCK: 프로세스의 시그널 마스크에 set에 포함된 시그널을 추가한다.(현재 시그널 마스크 | set 시그널 마스크)
	// SIG_UNBLOCK: 프로세스의 시그널 마스크에 set에 포함된 시그널을 제거한다.(현재 시그널 마스크 - set 시그널 마스크)
	if(sigprocmask(SIG_SETMASK, &set, NULL) == -1)
		errexit("sigprocmask");

	// SIGINT(ctrl + c)는 블록되었기 때문에 처리되지 않는다.
	pause();
}


sig_atomic_t count = 0;

static void signal_handler_sigint(int signo)
{
	if(signo == SIGINT) {
		// sys_siglist 배열을 사용하면 시그널 번호에 맞는 문자열을 가져올 수 있다.
		printf("pid = %d, caught signal = %s\n", getpid(), sys_siglist[SIGINT]);
		// 아토믹하게 접근해야 하는 전역변수.
		// SIGINT 시그널이 발생하면 전역변수를 초기화하는 작업을 수행한다.
		count = 0;
	}
}

void signal_suspend()
{
	sigset_t set, oldset;

	// 빈 시그널 집합에 SIGINT를 추가한다.
	sigfillset(&set);

	// 크리티컬 섹션으로 진입할 것이기 때문에 전체 시그널에 대해 수신을 블록한다.
	if(sigprocmask(SIG_SETMASK, &set, &oldset) == -1)
		errexit("sigprocmask");

	// SIGINT에 대해 시그널 핸들러를 설치한다.
	// 그러나, sigprocmask에 의해 크리티컬 섹션이 끝나는 동안에는 시그널이 발생하지 않을 것임.
	if(signal(SIGINT, signal_handler_sigint) == SIG_ERR)
		errexit("signal");

	// 크리티컬 섹션 - 시그널이 블록되어야 하는 구간
	int i;
	for (i = 0; i < 5; ++i) {
		printf("critical section ... %d\n", i);
		count = i; // 아토믹하게 접근해야 하는 전역변수
		sleep(1);
	}
	// sigsuspend:
	// 시그널 마스크를 파라미터로 넘기는 집합으로 변경하고 시그널 집합에 해당하는 시그널이 발생할때까지 기다린다.
	// 크리티컬 섹션에 머물러 있을 때 도착해서 블록되었던 시그널에 대해 처리할 수 있게 된다.
	// 이를 위해 sigprocmask함수를 통해 얻은 oldset(여기서는 기본 시그널 마스크 값)을 활용한다.
	// 시그널이 발생해서 시그널 핸들러에 의해 처리되는 경우에는 -1을 리턴하고 errno를 EINTR로 설정한다.
	// 리턴되면 다시 원래의 시그널 마스크를 복원한다.
	if(sigsuspend(&oldset) == -1)
		errexit("sigsuspend");

	// sigsuspend가 리턴되면 이전 시그널 마스크로 복귀(모두 블록)하므로, 기본 시그널 마스크(모두 허용)로 변경한다.
	if(sigprocmask(SIG_SETMASK, &oldset, NULL) == -1)
		errexit("sigprocmask()");
}

/*
sigaction 함수를 사용한 시그널 처리
- signal 함수보다 정교하게 시그널을 처리할 수 있다.
- SIGKILL, SIGSTOP을 제외한 모든 시그널에 대해 처리할 수 있다.
*/

// sa_sighandler 대신에 sa_sigaction 을 이용하는 경우 siginfo_t 구조체를 파라미터로 하여 더 많은 정보를 수신할 수 있게 됨.
static void signal_handler_adv(int sig, siginfo_t *si, void *ctx)
{
	printf("caught signal = %s\n", sys_siglist[sig]);
	// si_code: 프로세스가 어디서 시그널을 받았는지에 대한 설명
	printf("signal code = %d\n", si->si_code);

/*
	kill(pkill), raise()에 의해 전달된 시그널의 si_code는 SI_USER(0)에 해당함
	ctrl + c에 의해 전달된 시그널의 si_code는 SI_KERNEL(128)에 해당함
	sigqueue에 의해 전달된 시그널의 si_code는 SI_QUEUE(-1)에 해당함
	si_code값은 시그널의 종류에 따라 값의 의미가 달라진다.
	시그널이 SIGCHLD인 경우 si_code의 값은 아래와 같다.
	- CLD_DUMPED: 자식이 비정상 종료
	- CLD_EXITED: 자식이 정상(exit 함수 호출) 종료
	- CLD_KILLED: 자식이 종료
	- CLD_STOPPED: 자식이 정지
*/

	// sigqueue에 의해 전달된 시그널의 경우, si->si_value를 통해 int나 포인터 데이터를 핸들러로 전달받을 수 있다.
	if(si->si_code == SI_QUEUE) {
		printf("sig_val = %d\n", si->si_value.sival_int);
	}
}

void signal_sigaction()
{
	struct sigaction sa;

	// 시그널 핸들러가 실행되는 동안 블록해야할 시그널 집합 설정
	sigemptyset(&sa.sa_mask);
/*
	SA_SIGINFO를 설정하면 sa.sa_handler가 아닌 sa_sigaction이 시그널을 처리하는 함수로 지정된다.
	이 때 파라미터로 siginfo_t 구조체 데이터를 전달한다.
	사용 가능한 플래그
 	- SA_NOCLDSTOP: signo가 SIGCHLD인 경우 시스템에 자식 프로세스가 정지하거나 재시작할 때 이를 통보하지 않도록 함.
 	- SA_NOCLDWAIT: signo가 SIGCHLD인 경우 자식 프로세스에 대한 자동 처리를 활성화한다. 종료된 자식 프로세스는 좀비로 바뀌지 않으며 부모도 wait를 호출할 필요가 없어짐.
 	- SA_RESTART: 시그널로 중단된 시스템 콜(read, write 등)에 대해 재시작 가능하도록 함.
 	- SA_RESETHAND: 일회성 모드를 활성화한다. 시그널의 동작 방식은 일단 시그널 핸들러가 리턴된 다음 초기 설정으로 돌아온다.
*/
	sa.sa_flags = SA_SIGINFO;
	// sa.sa_handler와 sa.sa_sigaction은 union이므로 동시에 사용할 수 없고,
	// sa.sa_handler를 사용하는 경우, 기존 signal함수에서 사용하던 핸들러를 동일하게 쓸 수 있다.
	// 이 경우, SIG_DFL, SIG_IGN도 동일하게 쓸 수 있다.
	sa.sa_sigaction = signal_handler_adv;

	// SIGUSR1 시그널에 핸들러를 설치
	if(sigaction(SIGUSR1, &sa, NULL) == -1)
		errexit("sigaction");

/*
	플래그를 SA_SIGINFO로 설정하고 핸들러를 sa_sigaction으로 지정한 경우, 핸들러는 siginfo_t 구조체 데이터를 받는데,
	여기에는 si_value라는 필드를 포함하며, 이를 통해 핸들러에 데이터를 넘겨줄 수 있다
	이 때, 사용하는 함수가 sigqueue임.
*/
	// SIGUSR1 시그널을 프로세스에 전달(여기서는 자기 자신)하면서 100이라는 정수값을 전달함.
	// 이를 통해, 간단한 IPC를 구현할 수 있다.
	// 두 프로세스가 메모리를 공유한다면 공유 메모리 주소를 sigval_ptr로 전달할 수 있다.
	union sigval value;
	value.sival_int = 100;
	if(sigqueue(getpid(), SIGUSR1, value) == -1)
		errexit("sigqueue");

	// default 시그널 set을 구한다. sigprocmask를 호출할 때 두번째 파라미터를 NULL로 지정하면 현재 적용중인 시그널 마스크를 구할 수 있다.
	sigset_t oldset;
	if(sigprocmask(SIG_SETMASK, NULL, &oldset) == -1)
		errexit("sigprocmask");

	// 시그널 대기
	if(sigsuspend(&oldset) == -1)
		errexit("sigsuspend");

}

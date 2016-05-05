/*
 * process_setpgid.c
 *
 *  Created on: May 4, 2016
 *      Author: ahnmh
 */
#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <signal.h>

#include "process_func.h"
#include "tlpi_hdr.h"

static void handler(int sig)
{
	printf("SIGHUP signalled.\n");
}


/*
setpgid 함수를 통해 PGID를 분리하면, 분리된 자식 프로세스는
부모와 PGID가 다르기 때문에, 터미널 종료에 의해 발생하는 SIGHUP에 의해 종료되지 않는다.
(터미널이 종료되서 터미널 제어 프로세스(일반적으로 /bin/bash)가 종료되면 해당 터미널에서 실행된 모든 프로세스들이 SIGHUP 시그널을 받고 종료됨)
*/

void process_setpgid()
{
	pid_t child;
	struct sigaction sa;

	setbuf(stdout, NULL);

	// SIGHUP에 핸들러 설정
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	if(sigaction(SIGHUP, &sa, NULL) == -1)
		errExit("sigaction");

	// 자식 프로세스 생성
	child = fork();

	// 자식 프로세스인 경우 PGID 재설정
	if(child == 0)
		if(setpgid(0, 0) == -1) // 0, 0인 경우 자식 프로세스의 PID가 자식 프로세스의 PGID가 됨.
			errExit("setpgid");

	printf("PID = %ld, PPID = %ld, PGID = %ld, SID = %ld\n",
			(long)getpid(),
			(long)getppid(),
			(long)getpgrp(),
			(long)getsid(0)
			);

	// 자식 프로세스는 SIGHUP을 안받기 때문에 30초후에 SIGALRM에 의해 종료되도록 alarm 함수를 호출한다.
	alarm(30);

	while(1) {
		pause();
		printf("PID=%ld caught SIGHUP\n", (long)getpid());
	}
}


/*
 * 자식 프로세스에서 setpgid(0, 0)을 호출한 경우, 자식 프로세스의 프로세스 그룹 ID가 자신의 PID로 설정됨
ahnmh@ahnmh-samsung:~/workspace/linuxapi/LinuxAPI/Debug$ ./LinuxAPI
PID = 3294, PPID = 2833, PGID = 3294, SID = 2833
PID = 3295, PPID = 3294, PGID = 3295, SID = 2833

 * 일반적인 경우, 자식 프로세스의 프로세스 그룹 ID는 부모의 프로세스 그룹 ID(부모의 PID)와 동일함.
ahnmh@ahnmh-samsung:~/workspace/linuxapi/LinuxAPI/Debug$ ./LinuxAPI
PID = 3330, PPID = 2833, PGID = 3330, SID = 2833
PID = 3331, PPID = 3330, PGID = 3330, SID = 2833

ahnmh@ahnmh-samsung:~/workspace/linuxapi/LinuxAPI/Debug$ ./LinuxAPI > log 2>&1
와 같이 실행한 후에 해당 터미널을 종료(SIGHUP 발생)하면, log 파일에 아래와 같이 기록된다.
PID = 4903, PPID = 3743, PGID = 4903, SID = 3743
PID = 4904, PPID = 4903, PGID = 4904, SID = 3743
SIGHUP signalled. ---> 부모 프로세스의 경우 터미널 종료에 의해 SIGHUP을 받음
PID=4903 caught SIGHUP



*/

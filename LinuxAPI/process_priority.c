/*
 * process_priority.c
 *
 *  Created on: May 5, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE

#include <sys/resource.h>
#include <sched.h>

#include "process_func.h"
#include "tlpi_hdr.h"

/*
 * [표준 커널 스케쥴링[SCHED_OTHER] - 표준 라운드 로빈]
프로세스 속성인 nice값은 프로세스로 하여금 간접적으로 커널의 스케쥴링 알고리즘에 영향을 준다.
각 프로세스는 -20 ~ +19 범위의 nice 값을 갖는다.
기본값은 0이다.
nice값은 fork를 통해 생성된 자식에게 상속되며 exec에 걸쳐서 유지된다.
*/

void process_priority()
{
	int which, prio;
	id_t who;

	printf("PID = ");
	scanf("%d", &who);

	// set priority - who에 해당하는 PID를 가진 프로세스의 nice값을 -1로 설정
	if(setpriority(PRIO_PROCESS, who, 1) == -1)
		errExit("setpriority()");

	// get priority - who에 해당하는 PID를 가진 프로세스를 조회
	// PRIO_PROCESS: PID가 who인 프로세스에 적용함. who가 0이면 자기자신에게 적용함
	// PRIO_PGRP: PGID가 who인 프로세스 그룹에 적용함. who가 0이면 자기자신이 속한 프로세스 그룹에 적용함.
	// PRIO_USER: RUID(실제 사용자 ID)가 who인 모든 프로세스에 적용함. who가 0이면 호출자의 RUID를 사용한다.
	prio = getpriority(PRIO_PROCESS, who);
	if(prio == -1 && errno != 0)
		errExit("getpriority()");

	printf("nice value = %d\n", prio);
}


/*
[실시간 프로세스 스케줄링]
실시간(realtime)응용 프로그램은 외부 입력에 대해 보장된 최대 반응 시간을 제공해야 한다.
우선순위가 높은 프로세스는 CPU 사용을 완료하거나 자발적으로 CPU를 포기하기 전까지 CPU에 대한 전용 접근 권한을 가져야 한다.
리눅스는 1부터 99까지의 실시간 우선순위 레벨을 제공함.
실시간 프로세스 스케줄링 정책
- SCHED_RR
 : 우선순위가 동일한 프로세스는 라운드 로빈 시간 공유 방식으로 동작함.
 : 프로세스는 CPU를 사용할 때마다 고정된 길이의 할당 시간을 받는다.
 : 실행하는 프로세스가 CPU 접근권을 잃으면(할당 시간 만료, 자발적으로 블로킹, 선점)해당 우선순위 레벨 큐의 마지막으로 이동
 : 표준 라운드 로빈(SCHED_OTHER)과 다른 점은 낮은 nice값은 프로세스에게 CPU에 대한 전용 접근권을 주지 않는다.
    * SCHED_OTHER는 적어도 얼마간의 CPU 시간을 보장받는다.
- SCHED_FIFO
 : SCHED_RR과 비슷하지만 할당 만료 시간이 없다. SCHED_FIFO가 CPU 접근을 획득하면, 다음과 같은 상황이 발생할 때까지 실행한다.
   자발적으로 CPU 포기, 종료, 우선순위가 더 높은 프로세스가 선점.
*/

/*
[그밖의 스케쥴링]
- SCHED_BATCH
 : SCHED_OTHER 정책과 유사. 차이점은 SCHED_BATCH 정책은 자주 깨어나는 작업을 덜 스케쥴링되도록 유도함.
- SCHED_ILDE
 : SCHED_OTHER 정책과 유사하지만, 매우 낮은 nice값(+19보다 낮은 값)과 동일한 기능을 제공한다.
*/


static void get_process_scheduler(pid_t pid)
{
	int sched_policy;
	struct sched_param sp;
	sched_policy = sched_getscheduler(pid);
	if(sched_getparam(pid, &sp) == -1)
		errExit("sched_getparam()");

	printf("scheduling policy = ");
	switch(sched_policy) {
	case SCHED_RR:
		printf("SCHED_RR\n"); break;
	case SCHED_FIFO:
		printf("SCHED_FIFO\n"); break;
	case SCHED_OTHER:
		printf("SCHED_OTHER\n"); break;
	case SCHED_IDLE:
		printf("SCHED_IDLE\n"); break;
	case SCHED_BATCH:
		printf("SCHED_BATCH\n"); break;
	}

	printf("priority = %d\n", sp.sched_priority);

}

/* 프로세스 스케쥴링 정책을 SCHED_RR이나 SCHED_FIFO로 변경하는 경우
 * sudo 권한으로 실행해야 함.
 */
void process_scheduler()
{
	pid_t pid;
	int pol, priority;
	struct sched_param sp;

	printf("PID = ");
	scanf("%d", &pid);

	printf("before..\n");
	get_process_scheduler(pid);

/*
	#define SCHED_OTHER		0
	#define SCHED_FIFO		1
	#define SCHED_RR		2
	#define SCHED_BATCH		3
	#define SCHED_IDLE		5
*/
	printf("new SCHEDULE POLICY(0,1,2,3,5) = ");
	scanf("%d", &pol);

	printf("new PRIORITY(1 ~ 99 is available if new POLICY is 1 or 2, otherwise it shuold be 0) = ");
	scanf("%d", &priority);

/*
	sp.sched_priority 값은 SCHED_RR, SCHED_FIFO인 경우에만 1 ~ 99에 해당하고,
	그밖의 정책인 경우 0의 값이어야 함.
*/
	sp.sched_priority = priority;
	if(sched_setscheduler(pid, pol, &sp) == -1)
		errExit("sched_setscheduler()");

	printf("after..\n");
	get_process_scheduler(pid);

}

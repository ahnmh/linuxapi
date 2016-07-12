/*
 * process_manage_adv.c
 *
 *  Created on: Jul 12, 2016
 *      Author: ahnmh-vw
 */

#define _GNU_SOURCE
#include "process_manage.h"
#include <sched.h>

/*
nice:
프로세스에 할당되는 CPU 시간은 nice 값에 의해서 가중치가 적용된다.
nice 값의 범위: -20 ~ 19, 기본값: 0
nice값이 적을수록 우선순위가 높아짐.
nice값이 클수록 프로세스의 우선순위가 낮아지고 타임 슬라이스는 작아진다.
*/

// 나이스를 10으로 낮추는 설정
void nice_control()
{
	int val, ret;

	// 현재 나이스 값 얻기
	val = nice(0);
	printf("current nice value = %d\n", val);

	val = 10 - val;
	errno = 0;
	// 나이스 증가폭을 val값으로 설정
	// 현재 나이스가 2라면 10 - 2 = 증가폭 8이므로 결과적으로 10이 됨.
	if((ret = nice(val)) == -1 && errno != 0)
		errexit("nice");

	printf("new nice value = %d\n", ret);
}

// setpriority를 사용하여 nice값을 10으로 낮추는 설정
#include <sys/resource.h> // getpriority
void priority_control()
{
	int ret;

	// 현재 나이스값을 리턴
	if((ret = getpriority(PRIO_PROCESS, 0)) == -1)
		errexit("getpriority");
	printf("current nice value = %d\n", ret);

	//
	if(setpriority(
			PRIO_PROCESS,	// PRIO_PROCESS, PRIO_PGRP, PRIO_USER
			0,				// 0이면 현재 프로세스(프로세스 그룹)를 대상으로 함.
			10				// 설정하고자 하는 나이스 값. 나이스 값을 높이려면 root 권한(CAP_SYS_NICE)이 필요함.
			) == -1) // 호출이 성공하면 항상 0을 리턴한다. getpriority와 다르니 유의!
		errexit("setpriority");

	if((ret = getpriority(PRIO_PROCESS, 0)) == -1)
		errexit("getpriority");
	printf("new nice value = %d\n", ret);


}

/*
프로세스 affinity
프로세스를 꾸준히 같은 CPU에 스케줄링할 가능성을 의미함.
어떤 프로세스가 한 CPU에 스케줄링되면 스케줄러는 앞으로도 같은 CPU에서 실행되어야 한다.
프로세스를 다른 프로세스로 이전하는 것은 캐시와 관련한 손실이 발생하기 때문
- CPU마다 캐시는 분리되어 있고 프로세스가 다른 CPU로 옮겨지고 메모리에 새로운 데이터를 쓴다면 이전 캐시의 데이터는 쓸모가 없어진다.
- 특정 데이터는 매순간 어느 한 CPU 캐시에만 들어있다.
이 때문에 스케줄러는 최대한 프로세스를 특정 CPU에 유지하려고 한다.
그러나 한 CPU는 바쁘지만 다른 CPU는 놀고 있다면 덜 바쁜 CPU쪽으로 프로세스를 옮기는 게 합리적이다.
이러한 불균형을 개선하기 위해 언제 프로세스를 옮길지를 결정하는 작업을 로드 밸런싱이라고 하며 SMP머신의 중요 역할이다.
프로세스 affinity는 부모 프로세스로부터 상속받게 되므로 모든 프로세스의 부모인 init 프로세스에서 만약
0번 CPU를 사용하지 않도록 설정하면 CPU#0은 어떤 프로세스도 실행하지 않는다.
*/

#include <sched.h>

// 프로세스의 affinity를 get/set함.
void affinity_control()
{
	// CPU affinity 설정 구조체
	cpu_set_t set;

	// 구조체의 각 CPU값을 모두 reset하는 매크
	CPU_ZERO(&set);
	// 현재 프로세스에 대해 조회함.
	if(sched_getaffinity(0, sizeof(cpu_set_t), &set) == -1)
		errexit("sched_getaffinity");

	// affinity 설정값 확인
	int i;
	// CPU_SETSIZE = 1024
	for (i = 0; i < CPU_SETSIZE; ++i) {
		int cpu;
		// i번 CPU가 set상태인지 확인하는 매크로
		cpu = CPU_ISSET(i, &set);
		printf("CPU #%d = %d\n", i, cpu);
	}


	// affinity 설정
	CPU_ZERO(&set);
	// CPU 0번, 1번을 set한다.
	CPU_SET(0, &set);
	CPU_SET(1, &set);
	// CPU 2번, 3번을 clear한다.
	CPU_CLR(2, &set);
	CPU_CLR(3, &set);

	// 현재 프로세스에 대해 affinity를 set에 설정한 값대로 설정한다.
	if(sched_setaffinity(0, sizeof(cpu_set_t), &set) == -1)
		errexit("sched_getaffinity");

	for (i = 0; i < CPU_SETSIZE; ++i) {
		int cpu;
		// i번 CPU가 set상태인지 확인하는 매크로
		cpu = CPU_ISSET(i, &set);
		printf("CPU #%d = %d\n", i, cpu);
	}
}

/*
실시간 스케줄링 정책
기본 정책(CFS) 이외에 리눅스는 두 가지 실시간 스케줄링 정책을 제공한다.
- SCHED_FIFO
- SCHED_RR(라운드 로빈)
- SCHED_OTHER(비실시간 프로세스(일반 프로세스)의 기본 스케줄링 정책인 표준 스케줄링 정책. nice값을 사용)
모든 프로세스는 nice 값과 무관한 고유의 우선순위를 가진다.
일반 어플리케이션의 경우 항상 0이다.
실시간 프로세스의 값은 1 ~ 99이다.
리눅스 스케줄러는 항상 가장 높은 우선순위의 프로세스를 실행한다.
일반 프로세스는 우선순위가 0이므로 실행 가능한 실시간 프로세스가 항상 일반 프로세스보다 먼저 실행된다.

FIFO 정책(SCHED_FIFO)
더 높은 우선순위의 프로세스가 실행 가능한 상태가 되지 않는 한 계속 실행된다.
타임 슬라이스를 필요로 하지 않기 때문에 FIFO 정책을 따르는 실행 가능한 프로세스가 시스템에서 가장 우선순위가 높다면 항상 실행된다.
FIFO 정책을 따르는 프로세스가 블록되거나 sched_yield를 호출하거나 더 높은 우선순위의 프로세스가 실행 가능한 상태가 될 때까지 블록된다.
프로세스가 블록되면 스케줄러는 이 프로세스를 실행 가능한 프로세스 목록에서 제거한다.
다시 실행 가능한 상태가 되면 같은 우선순위를 가지는 프로세스 목록의 끝에 들어간다.
sched_yield를 호출하면 스케줄러는 이 프로세스를 같은 우선순위의 프로세스 목록의 끝으로 옮겨서 우선순위의 같은 다른 프로세스의 실행이 중단될 때까지 실행되지 않도록 한다.
FIFO 정책을 따르게 되거나, 고유의 우선순위가 바뀌게 되면 같은 우선순위의 프로세스 목록의 가장 앞에 위치하게 된다.
그 결과 새로운 우선순위가 매겨진 FIFO 프로세스는 기존의 실행중이던 같은 우선순위의 프로세스를 선점해서 실행된다.

라운드 로빈 스케줄링 정책(SCHED_RR)
같은 우선순위를 가지는 프로세스에 대한 규칙을 제외하면 FIFO와 동일하다.
각 라운드 로빈 프로세스에 타임 슬라이스를 배분한다.
주어진 타임 슬라이스를 다 소진하면 같은 우선순위의 프로세스 목록에서 다음 프로세스를 실행한다.
해당 우선순위를 가지는 프로세스가 하나뿐이라면 FIFO와 동일하다.
타임 슬라이스를 다 소진하면 실행을 중단하고 같은 우선순위를 가지는 프로세스 목록의 끝으로 이동한다.
라운드 로빈 프로세스는 같은 우선순위를 가지는 프로세스끼리 스케줄된다.

표준 스케줄링 정책(SCHED_OTHER)
비실시간 프로세스의 기본 스케줄링 정책인 표준 스케줄링 정책을 나타낸다.
모든 일반 프로세스는 고유의 우선순위로 0을 가진다.
따라서 FIFO와 RR 프로세스는 일반 프로세스를 선점할 수 있다.
nice값을 사용해서 일반 프로세스 간의 우선순위를 적용한다. nice값은 고유의 우선순위(0)에 영향을 미치지 않는다.

배치 스케줄링 정책(SCHED_BATCH)
일괄 또는 유휴 스케줄링 정책
다른 프로세스가 타임 슬라이스를 모두 소진했더라도 시스템에 실행 가능한 프로세스가 없을 때만 실행된다.
*/

/*
스케줄러와 우선순위 변경.
root 권한(CAP_SYS_NICE)으로 실행되어야 한다.
*/
void sched_control()
{
	int policy;
/*
	현재 프로세스에 적용중인 스케줄러 정책 얻기
	#define SCHED_OTHER		0
	#define SCHED_FIFO		1
	#define SCHED_RR		2
	#define SCHED_BATCH		3
	#define SCHED_IDLE		5
*/
	policy = sched_getscheduler(0);
	printf("old scheduler = %d\n", policy);

	struct sched_param sp;
	sp.sched_priority = 1; // 우선순위 지정

	// 스케줄러 정책(라운드 로빈 스케줄링)과 우선순위 변경
	if(sched_setscheduler(0, SCHED_RR, &sp) == -1)
		errexit("sched_setscheduler");

	policy = sched_getscheduler(0);
	printf("new scheduler = %d\n", policy);

	// 우선순위 값 얻기
	if(sched_getparam(0, &sp) == -1)
		errexit("sched_getparam");
	printf("current priority = %d\n", sp.sched_priority);

	// 우선순위 값만 변경
	sp.sched_priority = 2;
	if(sched_setparam(0, &sp) == -1)
		errexit("sched_getparam");
	printf("new priority = %d\n", sp.sched_priority);


	// 현재 스케줄링 정책의 유효한 우선순위 값 확인
	int min, max;
	if((min = sched_get_priority_min(policy)) == -1) // 1
		errexit("sched_get_priority_min");
	if((max = sched_get_priority_max(policy)) == 01) // 99
		errexit("sched_get_priority_max");
	printf("SCHED_RR priority range is %d - %d\n", min, max);

}

/*
프로세스에 대한 리소스 제한
대상 : 열수 있는 파일의 갯수, 메모리 페이지, 대기 중인 시그널 등..
소프트 제한과 하드 제한
커널은 프로세스에 소프트 제한을 강제한다.
그러나 프로세스는 소프트 제한을 하드 제한값까지 자유롭게 변경할 수 있다.
CAP_SYS_RESOURCE 기능이 제한된 프로세스는 하드 제한보다 낮은 값만을 설정할 수 있다.
권한이 없는 프로세스는 하드 제한값을 올릴 수 없으며 이전에 설정된 값보다 높게 설정할 수 없다.
커널은 이 제한을 init 프로세스에서 설정하고, 자식 프로세스들은 부모인 init 프로세스로부터 상속받기 때문에 모든 프로세스는 init 프로세스의 소프트 제한과 하드 제한을 상속받는다.
# ulimit -aS 값으로 소프트 제한과 하드 제한을 확인할 수 있다.
*/

#include <sys/resource.h> // rlimit
void limit_control()
{
	struct rlimit rlim;

	// 코어 덤프 파일의 최대 용량 확인
	if(getrlimit(RLIMIT_CORE, &rlim) == -1)
		errexit("getrlimit");
	printf("RLIMIT_CORE limit size: soft = %ld, hard = %ld\n", rlim.rlim_cur, rlim.rlim_max);

	// 코어 덤프 파일의 최대 용량 변경
	rlim.rlim_cur = 1024 * 1024 * 32; // 32MB
	rlim.rlim_max = -1;
	if(setrlimit(RLIMIT_CORE, &rlim) == -1)
		errexit("getrlimit");

	// ulimit 명령을 사용해서 코어 덤프 파일 사이즈 변경값 확인
	if(system("ulimit -c") == -1)
		errexit("system");
}

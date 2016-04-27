/*
 * process_func.c
 *
 *  Created on: Apr 27, 2016
 *      Author: ahnmh
 */
#include <stdio.h>
#include <unistd.h>

#include "process_func.h"
#include "tlpi_hdr.h"

static int static_data = 1;

/* fork의 호출 예제 */

void process_func()
{
	int status;
	int stack_data = 100;
	pid_t child_proc;

	switch(child_proc = fork()) {
	case -1:
		errExit("fork()");

		// fork 함수 이후 자식 프로세스의 시작점. child의 경우 0을 리턴한다
	case 0:
		static_data += 5;
		stack_data += 50;
		break;

		// fork함수 이후 부모 프로세 시작점. 부모의 경우 child의 pid를 리턴한다
	default:
/*
		부모가 자식의 종료(exit)를 대기하려면 wait 함수를 호출.
		일반적으로 부모가 fork이후 컨텍스트 스위칭되지 않는다면 부모가 먼저 수행됨.(성능 측면에서 더 우수함)
		아래와 같이 wait함수를 사용하면 자식의 종료를 대기할 수 있다.
		파라미터 status는 자식이 exit()를 수행할 때 전달한 값이 전달된다.
*/
		wait(&status);
		break;
	}

/*
 * 아래는 수행 결과이다.
 * fork() 이후 두 프로세스는 동일한 프로그램 텍스트를 수행하지만 각자의 스택, 데이터, 힙 세그먼트를 갖는다.
 * 자식의 스택, 데이터, 힙 세그먼트는 부모의 복제본으로 주어지므로 각 프로세스는 다른 프로세스에 영향을 주지 않고
 * 스택, 데이터, 힙 세그먼트를 수정할 수 있다.(copy-on-write 기법으로 동작함)
 * 반대로 vfork()의 경우 부모의 스택, 데이터, 힙 세그먼트를 공유해서 서로 영향을 주는 방식임.
	pid = 2590, static_data = 6, stack_data = 150
	pid = 2586, static_data = 1, stack_data = 100

	파일 디스크립터의 경우 자식은 부모의 파일 디스크립터 모두에 대해 복제본을 받는다.
	서로 일치하는 디스크립터는 동일한 여린 파일 스크립터를 가르키게 되므로, 자식이 파일 옵셋을 변경하면
	이 변경은 대응하는 디스크립터를 통해 부모에게도 보인다.
*/
	printf("pid = %d, static_data = %d, stack_data = %d\n", getpid(), static_data, stack_data);

	exit(EXIT_SUCCESS);
}

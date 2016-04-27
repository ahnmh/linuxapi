/*
 * process_exit.c
 *
 *  Created on: Apr 28, 2016
 *      Author: ahnmh
 */
#define _BSD_SOURCE // on_exit

#include <stdio.h>
#include <stdlib.h>

#include "process_func.h"
#include "tlpi_hdr.h"

static void exit_handler1()
{
	printf("exit_handler1\n");
}

static void exit_handler2(int status, void *arg)
{
	int *context = (int*)arg;
	printf("exit_handler2, status = %d, arg = %d\n", status, *context);
}

/*
종료 핸들러를 등록하는 예제
exit 함수 호출시 등록 순서의 역순으로 호출된다.
_exit 시스템콜을 직접호출하면 종료 핸들러가 호출되지 않는다.
*/
void process_exit()
{
	// 파라미터가 없는 종료 핸들러 등록
	if(atexit(exit_handler1) != 0)
		errExit("atexit()");

	int context = 1;

	// exit 함수 호출시 전달하는 값과 사용자 정의 값을 전달할 수 있는 종료 핸들러 등록
	if(on_exit(exit_handler2, &context) != 0)
		errExit("on_exit()");

	exit(EXIT_SUCCESS);
}



/*
 * dltest.c
 *
 *  Created on: May 14, 2016
 *      Author: ahnmh
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include "dltest.h"
#include "tlpi_hdr.h"

void dltest()
{
	void *lib_handle;
	// 얻으려고하는 심볼(함수)의 원형 함수 포인터
	int (*funcp)(int, int);
	const char *err;

/*
	공유 라이브러리를 메모리에 로드하고 핸들을 얻는다.(참조 카운트 +1)
	RTLD_LAZY : 심볼이 코드가 실행되기 전까지 해석되지 않는다.(변수는 바로 해석됨)
	RTLD_NOW : dlopen() 함수이후 해당 함수의 실행 여부와 무관하게 라이브러리의 심볼이 해석된다. 프로그램을 디버깅할 때 유용함.
*/
	lib_handle = dlopen("libtestfunc.so", RTLD_LAZY);
	if(lib_handle == NULL)
		fatal("dlopen: %s", dlerror());

	// dlerror() 함수 초기화
	(void)dlerror();

	// 라이브러리에서 심볼(test_add 함수)을 찾는다.
	funcp = dlsym(lib_handle, "test_add");
	err = dlerror();
	if(err != NULL)
		fatal("dlsym: %s", err);

	int temp;
	if(funcp == NULL)
		printf("test_add() is NULL\n");
	else
		temp = funcp(10, 20);

	// 공유 라이브러리를 메모리 해제하고 참조 카운트 -1
	dlclose(lib_handle);

}

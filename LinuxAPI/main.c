/*
 * main.c
 *
 *  Created on: Mar 31, 2016
 *      Author: root
 */

#include "file_io.h"
#include "process_env.h"
#include "memory_alloc.h"
#include "time_func.h"
#include "user_group.h"
#include "get_limit.h"
#include "file_buffering.h"

int main()
{
//	int ret = EXIT_SUCCESS;

/*
	ret = file_io();
	ret = file_fcntl();
	ret = file_scatter_gather();
	ret = file_ftruncate();
	ret = file_opentempfile();
	ret = file_pread_pwrite();
*/

//	ret = get_enviorn();

//	memory_alloc();

/*
	realloc 테스트
	ptr을 전역변수로 두고 이 주소에 realloc으로 할당한 메모리 주소를
	넘겨주면 매번 free할 필요없이, realloc하다가 마지막에 한번만 free 해줄수 있을 것같음..
	ptr should be NULL at first time.

	int *ptr = NULL;
	realloc_test(&ptr, 10);
	memcpy(ptr, "1234512345", 10);
	realloc_test(&ptr, 15);
	memcpy(ptr, "123451234512345", 15);
	realloc_test(&ptr, 20);
	memcpy(ptr, "12345123451234512345", 20);

	free(ptr);
*/

//	user_group();

//	time_related_func();

//	get_limit();

	file_buffering();


	return 0;
}

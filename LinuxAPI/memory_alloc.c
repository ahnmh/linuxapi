/*
 * memory_alloc.c
 *
 *  Created on: Apr 5, 2016
 *      Author: ahnmh
 */
#include "memory_alloc.h"
#include "tlpi_hdr.h"
#include <stdlib.h>
#include <mcheck.h> // mcheck it needs mcheck library.
#include <malloc.h> // mallinfo

void memory_alloc()
{
//	 setenv, unsetenv sample
	setenv("MALLOC_TRACE", "mallog", 1);

	mtrace();
	int *imem = (int*)malloc(sizeof(int) * 100);

	free(imem);
	muntrace();

	unsetenv("MALLOC_TRACE");
	/* mallog 파일의 내용
	= Start
	@ /home/ahnmh/sample/LinuxAPI/Debug/LinuxAPI:[0x402462] + 0x605850 0x190 ----> Location and Size
	@ /home/ahnmh/sample/LinuxAPI/Debug/LinuxAPI:[0x402472] - 0x605850
	= End*/




/*
	같은 힙메모리에 대해 free를 두번 호출하는 것은 debug 모드에서는
	free가 두번째 호출될 때 문제를 일으키거나 release 모드에서는 그냥 지나갈 수 있지만,
	mcheck 함수를 malloc이전에 호출하면, malloc 함수를 호출하기전에 segmentation fault가 생기면서 해당 문제점을 알아낼 수 있다.
	그러나 오버헤드가 큼.. CPU 점유율이 매우 상승하면서 컴퓨터가 뜨거워짐..
*/
/*	mcheck(NULL);*/

	// 메모리에 대한 통계 정보를 담고 있는 구조체를 리턴함.
	struct mallinfo info = mallinfo();


	struct tests{
		int a;
		int b;
		int c[10];
	};
/*
	 구조체 단위로 메모리 할당시에는 malloc보다 calloc이 간편하다.
	 또한 0으로 초기화해주는 기능도 있다.
*/
	struct tests* test_arr = calloc(100, sizeof(struct tests));
	free(test_arr);

/*
	메모리를 할당할 때, boundary로 지정한 값의 배수 단위의 주소로 정렬된다.
	예를 들어 0x1000인 경우, 65000과 같은 값임.
	boundary는 반드시 2의 거듭제곱이어야 함.
*/
	void *memptr = memalign(0x1000, sizeof(int) * 100);
	printf("address = 0x%8p\n", memptr);

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



}

void realloc_test(void **ptr, size_t size)
{
	void *nptr;
	nptr = realloc(*ptr, size);
	if (nptr == NULL)
		errExit("realloc");
	else
		*ptr = nptr;

}



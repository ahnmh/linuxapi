/*
 * mem_handle.c
 *
 *  Created on: Jul 19, 2016
 *      Author: ahnmh-vw
 */


#define _GNU_SOURCE // mempcpy, memchr, memrchr, memmem
#include "mem.h"
#include <string.h>

void mem_handle_sample(int argc, char *argv[])
{
	int size = 100;
	char *src = malloc(size);
	char *dst = malloc(size);
/*
	memeset
	메모리 블록을 0으로 채울 때 주로 사용함.
	malloc과 memset 대신 calloc을 사용하는 편이 더 빠르다.(커널에서 이미 0으로 초기화된 메모리를 얻어옴)
	bzero 함수는 오래되었으므로 새로운 코드는 반드시 memset을 사용해야 한다.
*/
	memset(src, 0, size); // 또는 memset(src, '\0', size)
	strcpy(src, "sample:test:hello");

/*
	memmove
	src의 처음 n바이트를 dst로 복사하고 dst를 리턴한다.
	중첩되는 메모리 영역을 지원한다.(dst의 일부가 src 안에 존재하는 경우)
	src와 dst가 중첩되는 경우의 예: memcpy(src, src + 5, 10);

*/
//	memmove(dst, src + 5, 10);

/*
	memcpy
	src와 dst가 중첩되는 것을 지원하지 않는다는 점을 제외하면 memmove와 동일하다.
	중첩되면 결과를 알 수 없다.
*/
//	memcpy(dst, src + 5, 10);


/*
	memccpy
	src의 첫 n 바이트 내에서 c를 발견하면 복사를 멈춘다.
	c는 복사되는 문자열에 포함된다.
*/
//	memccpy(dst, src, ':', 10);

/*
	mempcpy
	src의 첫 n 바이트를 dst로 복사한 이후의 dst + n의 위치를 리턴한다.
*/
//	char *new_dst = mempcpy(dst, src, 10);
//	printf("dst = %p, new_dst = %p\n", dst, new_dst);

/*
	memchr
	문자 검색
	src 메모리의 n 바이트 범위에서 문자 c를 탐색하여 위치를 리턴한다.
	뒤에서부터 검색하는 경우 memrchr
*/
//	char *find = memchr(src, ':', 10);
//	printf("src = %p, find = %p\n", src, find);

/*
	memmem
	문자열 검색
*/
	char *keyword = "test";
	char *find = memmem(src, 20, keyword, strlen(keyword));
	printf("src = %p, find = %p\n", src, find);

	printf("src = %s\n", src);
	printf("dst = %s\n", dst);

	free(src);
	free(dst);
}

#include <sys/mman.h>
#include <unistd.h> // getpagesize
void mlock_sample()
{
/*
	mlockall
	전체 프로세스에 대해 락을 거는 함수
	flag: MCL_CURRENT(현재 메모리에 대해), MCL_FUTURE: 향후 할당되는 메모리까지 포함
*/
//	if(mlockall(MCL_CURRENT) == -1)
//		errexit("mlockall");

	// posix_memalign : 페이지 단위로 정렬된 메모리 할당
	char *buf;
	if(posix_memalign((void **)&buf, getpagesize(), 1024) != 0)
		errexit("posix_memalign");

	printf("aligned allocated memory = %p\n", buf);

/*
	mlock
	특정 메모리에 락을 걸어 항상 페이지인 상태로 만든다.
	POSIX 표준은 addr이 메모리 정렬되어야 한다는 조건이 있지만 리눅스는 정렬 주소 기준으로 자름.
*/
	if(mlock(buf, 1024) == -1)
		errexit("mlock");

	// do something
	char *test = "this is test";
	memcpy(buf, test, 10);
	printf("%s\n", buf);

/*
	메모리 락 해제
	락과 언락은 중첩되지 않기 때문에 여러 번 락을 걸어도 한번만 언락하면 해제된다.
*/
	if(munlock(buf, 10) == -1)
		errexit("munlock");

	free(buf);

	// 프로세스의 전체 메모리에 건 락 해제
//	if(munlockall() == -1)
//		errexit("munlockall");
}

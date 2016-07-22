/*
 * mem_alloc.c
 *
 *  Created on: Jul 18, 2016
 *      Author: ahnmh-vw
 */

#define _GNU_SOURCE // posix_memalign
#include <unistd.h> // getpagesize
#include "mem.h"

void mem_alloc()
{
	// calloc: 배열 동적 할당
	// malloc과 다르게 배열 원소를 모두 0으로 초기화하는 방법을 제공한다.
	// 별도로 memset을 호출하는 것보다 빠르다.
	char *x;
	if((x = calloc(100, sizeof(char))) == NULL)
		errexit("calloc");

	printf("allocated memory = %p\n", x);
	free(x);

	// realloc: 이미 할당한 영역을 키우거나 줄임
	char *y;
	// ptr을 NULL로 지정하면 malloc과 동일하게 동작한다.
	if((y = realloc(NULL, sizeof(char) * 20)) == NULL)
		errexit("calloc");

	printf("allocated memory = %p\n", y);

	char *re_y;
	if((re_y = realloc(y, sizeof(char) * 0x30000)) == NULL)
		errexit("calloc");

	printf("re-allocated memory = %p\n", re_y);
/*
	할당 크기가 0x20000(128KB)가 넘어가면 힙에 할당되지 않고, 익명 메모리 맵핑(파일등과 연관되지 않는 매핑)이 사용됨.
	$ ./api3
	allocated memory = 0x1e6c010 --> 첫번째 realloc 호출
	re-allocated memory = 0x7f0fef27f010 --> 두번째 realloc 호출
*/
	free(re_y);

	// posix_memalign : 페이지 단위로 정렬된 메모리 할당
	// 아래를 호출하면 0xf5d000 형태로 페이지 단위로 정렬된 위치에 메모리를 할당한다.
	// 일반 할당 함수인 malloc등을 쓰면, 0x243a010와 같은 형태로 할당된다.
	char *buf;
	if(posix_memalign((void **)&buf, getpagesize(), 1024) != 0)
		errexit("posix_memalign");

	printf("aligned allocated memory = %p\n", buf);

	free(buf);
}

#include <sys/mman.h>
void mem_anonymous_mmap()
{
	void *p;

	p = mmap(
			NULL, // 할당 위치는 지정해줄 필요가 없다.
			0x2000, // 2페이지를 할당한다.
			PROT_READ | PROT_WRITE, // 읽기, 쓰기 권한을 부여한다.
			MAP_ANONYMOUS | MAP_PRIVATE, // 익명 매핑이며 공유하지 않는다.
			-1, // 익명 매핑인 경우 fd는 무시하지만 -1로 지정한다.
			0 // 옵셋도 의미없음
			);

	if(p == MAP_FAILED)
		errexit("mmap");

	printf("allocated anonymous memory map = %p\n", p);

	// 생성된 매핑을 해제한다.
	// 일반적으로 mmap의 리턴값과 mmap 파라미터 len 값을 사용함.
	if(munmap(p, 0x2000) == -1)
		errexit("munmap");
/*
	$ ./api3
	allocated anonymous memory map = 0x7f842f627000
*/
}

#include <malloc.h>
void mem_alloc_control()
{
/*
	mallopt : 메모리 할당 옵션을 변경하는 함수
	반드시 malloc을 호출하기 전에 사용해야 한다.
	M_MMAP_THRESHOLD: 지정한 크기를 넘어가는 할당은 MMAP을 사용한다. 기본값 = 128KB(0x20000)
	M_MMAP_MAX: 동적 메모리 할당을 위해 사용할 수 있는 최대 맵핑 갯수. 기본값 64 x 1024
*/
	if(mallopt(M_MMAP_THRESHOLD, 0x1000) == 0)
		errexit("mallopt");

	char *y;
	if((y = calloc(0x2000, sizeof(char))) == NULL)
		errexit("calloc");

	printf("allocated memory = %p\n", y); // 메모리 할당 위치가 힙 영역이 아님.

	// mallinfo : 메모리 할당 정보 확인
	struct mallinfo minfo;
	minfo = mallinfo();
	printf("data segment size used by malloc = %d\n", minfo.arena);
	printf("count of anonymous map = %d\n", minfo.hblks); // 1
	printf("size of anonymous map = %d\n", minfo.hblkhd); // 0x3000 ??

	// 메모리 할당 정보를 stderr로 출력
	malloc_stats();

	free(y);
}

/*
alloca
스택에 동적 메모리를 할당하는 경우에 사용함.
malloc처럼 할당한 메모리에 대해 해제할 필요없다.
스택 포인터만 증가시키면 되므로 malloc보다 훨씬 빠르다.
그러나 프로그램이 호환성을 유지해야 한다면 alloca를 사용하면 안된다.
alloca를 사용한 strdupa와 같은 함수를 지원한다.
*/
#define _GNU_SOURCE	// strdupa
#include <alloca.h>
#include <fcntl.h>
#include <string.h>
void mem_alloca(int argc, char *argv[])
{
/*
	일반적으로 길이를 알 수 없는 경우,
	char buf[1024] 처럼 스택 버퍼를 크게 잡던가, 또는 malloc 동적 메모리 할당을 사용해야 한다.
	하지만 alloca를 사용하면 스택을 통해 빠르게 그리고 가변 길이의 메모리를 할당할 수 있다.
*/
	// 실행 시 인자를 받아서 /etc/argv[1] 경로의 파일을 여는 예
	const char *etc = "/etc/";
	char *name;
	int fd;

	// 스택에 가변 길이 메모리 할당
	name = alloca(strlen(etc) + strlen(argv[1]) + 1);
	if(!name)
		errexit("alloca");

	strcpy(name, etc);
	strcat(name, argv[1]);

	if((fd = open(name, O_RDONLY)) == -1)
		errexit("open");

	// 할당한 메모리 해제할 필요 없음.
	printf("fd = %d\n", fd);


	// strdupa 사용 예
	char *test = "strdupa sample";
	char *str_s = strdupa(test);
	char *str_h = strdup(test);


/* 출력 결과
	stack = 0x7ffc1d821490 strdupa sample
	heap = 0x201a010 strdupa sample
*/
	printf("stack = %p %s\n", str_s, str_s);
	printf("heap = %p %s\n", str_h, str_h);

	// 힙에 생성한 메모리는 해제과정이 필요함.
	free(str_h);
}

/*
가변 길이 배열
C89에서는 알다시피, char buf[strlen(str)]과 같이 런타임에 배열 길이를 지정할 수 없다.
C99에서는 이러한 동작이 가능하다.
스택에 가변 길이의 메모리를 생성하는 것은 alloca를 사용해서도 가능하지만, 스코프가 전체 함수 영역이기 때문에,
루프 안에서 사용하면 스택 크기가 계속 커진다.
가변 길이 배열로 얻은 메모리는 함수가 끝나는 시점이 아니라 변수가 스코프가 벗어날 때까지만 유지된다.
*/

void mem_variable_len_array(int argc, char *argv[])
{
	const char *etc = "/etc/";
	int fd;

	// 스택에 가변 길이 배열 할당
	char name[strlen(etc) + strlen(argv[1]) + 1];

	strcpy(name, etc);
	strcat(name, argv[1]);

	if((fd = open(name, O_RDONLY)) == -1)
		errexit("open");

	printf("fd = %d\n", fd);
}

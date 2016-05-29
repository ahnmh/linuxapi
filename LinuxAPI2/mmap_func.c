/*
 * mmap_func.c
 *
 *  Created on: May 27, 2016
 *      Author: ahnmh
 */

#define _BSD_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "mmap_func.h"
#include "tlpi_hdr.h"

/* 메모리 매핑
 * 프로세스의 가상 메모리에 파일의 내용을 적재하는 것(메모리 맵 파일)
 * 프로세스의 매핑 메모리는 MAP_SHARED인 경우 다른 프로세스와 공유할 수 있다.
 *
 * 비공개 매핑(MAP_PRIVATE) : 매핑 내용을 수정해도 다른 프로세스에서 이를 볼 수 없으며 수정한 내용은 원본 파일에 직접 반영되지 않음.
 * copy-on-write를 사용해서 프로세스가 페이지 내용 수정을 시도할 때마다 커널은 새로운 프로세스 페이지의 분리된 복사본을 만듬.
 *
 * 상속 관계 : 프로세스를 exec로 수행하면 매핑 정보는 없어지고, fork로 생성된 자식 프로세스는 상속 받는다.
 *
 * 사용 방법 :
 * - 프로세스 비공개 메모리 할당(비공개 익명 매핑 = MAP_PRIVATE|MAP_ANONYMOUS)
 * - 프로세스의 데이터 세그먼트 초기화와 텍스트 내용 초기화(비공개 파일 매핑 = MAP_PRIVATE)
 * - 연관된 프로세스 간에 fork()로 메모리 공유(공개 익명 매핑 = MAP_SHARED|MAP_ANONYMOUS)
 * - 관련 프로세스간에 메모리 공유를 겸한 메모리 맵 I/O(공개 파일 매핑 = MAP_SHARED)
 */

// 비공개 파일 매핑 예제. cat 명령과 유사한 동작을 수행한다.
void mmap_file_privated(int argc, char *argv[])
{
	char *addr;
	int fd;
	struct stat sb;

	fd = open(argv[1], O_RDONLY);

	if (fstat(fd, &sb) == -1)
		errExit("fstat()");

/*
	파일 매핑
	리턴 값 : 매핑 주소
	NULL: 커널이 매핑에 알맞는 주소를 리턴함. 페이지 경계에 할당된다.
	sb.st_size: 매핑 크기. 지정한 값보다 큰 페이지 크기 단위로 할당된다.
	PROT_READ: 메모리 보호 모드. 읽기만 가능
	MAP_PRIVATE: 비공개 매핑. 같은 매핑을 사용하는 프로세스끼리 매핑 내용 수정이 다른 프로세스에서 보이지 않음.
	fd: 매핑하고자 하는 파일
	0: 파일상 매핑 시작 지점. 반드시 페이지 크기의 배수이어야 함. 전체 파일을 매핑하려면 0
*/
	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
		errExit("mmap()");

	if (write(STDOUT_FILENO, addr, sb.st_size) != sb.st_size)
		fatal("write()");

	// 매핑 해제 - 할당한 매핑의 크기를 파라미터로 전달하여 전체 해제를 수행함.
	// 프로세스가 종료되어도 자동적으로 해제된다.
	if(munmap(addr, sb.st_size) == -1)
		errExit("munmap()");
}

// 공개 파일 매핑 예제
#define MEM_SIZE	10
void mmap_file_shared(int argc, char *argv[])
{
	char *addr;
	int fd;

	// 쓰기 권한으로 여는 경우,
	fd = open(argv[1], O_RDWR);

	// 공개 매핑시 PROT_WRITE 권한을 지정한다.
	addr = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
		errExit("mmap()");

	// 파일 디스크립터는 필요없으므로 닫는다
	close(fd);

	printf("current value = %.*s\n", MEM_SIZE, addr);

	if (argv[2]) {
		// 메모리 매핑 영역 수정
		memset(addr, 0, MEM_SIZE);
		strncpy(addr, argv[2], MEM_SIZE);

/*
		매핑된 영역 동기화 -
		addr, length로 지정한 매핑 영역을 원본 파일에 반영하는 함수
		addr, MEM_SIZE는 mmap의 제약 조건과 동일하다(addr은 페이지 정렬되어 있어야 하며 length는 페이지 크기만큼 올림된다)
		MS_SYNC = 동기식 파일 쓰기. 메모리 영역의 모든 페이지 수정이 디스크에 저장될 때까지 호출은 블록된다.
		MS_ASYNC = 비동기식 파일 쓰기. 메모리 영역의 페이지 수정은 이후에 저장(pdflush라는 커널 스레드가 버퍼 플러싱을 수행할때까지)되고
		            해당 파일 영역에서 read()를 수행하는 다른 프로세스는 바로 접근이 가능하다.
		MS_INVALIDATE = 매핑된 데이터의 복사본 캐시를 갱신한다. 즉, 파일에 반영된 수정 내용을 매핑된 메모리에 반영한다.(MS_SYNC와 반대 방향)
*/
		if (msync(addr, MEM_SIZE, MS_SYNC) == -1)
			errExit("msync()");

		printf("new value = %.*s\n", MEM_SIZE, addr);
	}
}

/*
공개 익명 매핑 예제 - 파일을 사용하지 않는 매핑을 통한 부모와 자식 프로세스간(fork) 메모리 공유
일반 동적 할당 변수와 비교
*/
void mmap_anonymous_shared()
{
	int *shared;
	int *local;

/*
	MAP_ANONYMOUS인 경우 파일이 필요하지 않으며 파일 디스크립터는 -1로 지정한다.
	매핑된 파일은 모두 0으로 초기화된다.
	실제로는 /dev/zero를 상대로 매핑하는 것이며 /dev/zero는 0을 돌려주는 가상 디바이스임.
*/
	shared = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(shared == MAP_FAILED)
		errExit("mmap()");
	*shared = 1;

	// 비교를 위한 일반 동적할당 변수
	local = (int *)malloc(sizeof(int));
	*local = 1;

	switch(fork()) {
	case -1:
		_exit(EXIT_FAILURE);
		break;

	case 0:
		(*shared)++; // 자식 프로세스가 공유 변수를 증가시키면 부모와 메모리를 공유하므로 결과값은 부모 프로세스와 동일하다.
		(*local)++; // 일반 동적 할당 변수를 증가시키면 서로 다른 위치를 참조하므로 부모 프로세스에는 반영되지 않는다.
		printf("child shared address = %p, shared = %d, normal = %d\n", shared, *shared, *local);
		munmap(shared, sizeof(int));
		exit(EXIT_SUCCESS);

	default:
		wait(NULL);
		printf("parent shared address = %p, shared = %d, normal = %d\n", shared, *shared, *local);
		munmap(shared, sizeof(int));
		exit(EXIT_SUCCESS);

	}
}

#define LEN	1024
#define SHELL_FMT	"cat /proc/%ld/maps | grep zero"
#define CMD_SIZE	(sizeof(SHELL_FMT) + 20)
void mmap_ops()
{
	char cmd[CMD_SIZE];
	char *addr;
	addr = mmap(NULL, LEN, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (addr == MAP_FAILED)
		errExit("mmap()");

	// 여기서 접근하면 SIGSEGV가 발생함.
//	strcpy(addr, "test");
	// 보호모드 수정 전 매핑 메모리 정보 확인
	snprintf(cmd, CMD_SIZE, SHELL_FMT, (long)getpid());
	system(cmd);

	if (mprotect(addr, LEN, PROT_READ | PROT_WRITE) == -1)
		errExit("mprotect()");

	// 보호모드 수정 후 매핑 메모리 정보 확인
	snprintf(cmd, CMD_SIZE, SHELL_FMT, (long)getpid());
	system(cmd);

	// 가상 메모리의 내용을 물리 메모리에 고정
	if (mlock(addr, LEN) == -1)
		errExit("mlock()");

	// 가상 메모리의 내용을 페이지 아웃 가능하게 함.
	if (munlock(addr, LEN) == -1)
		errExit("munlock()");

}

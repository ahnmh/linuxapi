/*
 * fileio_mmap.c
 *
 *  Created on: Jul 5, 2016
 *      Author: ahnmh-vw
 */
#define _GNU_SOURCE	// mremap

#include <sys/mman.h>
#include <sys/stat.h> // fstat, S_ISREG
#include <string.h>
#include "fileio.h"

void mmap_sample(int argc, char *argv[])
{
	int fd;
	fd = open(argv[1], O_RDWR);
	if(fd == -1)
		errexit("open");

	// 일반 파일인지 검사
	struct stat sb;
	if(fstat(fd, &sb) == -1)
		errexit("fstat");
	if(!S_ISREG(sb.st_mode)) {
		fprintf(stderr, "%s is not a file.\n", argv[1]);
		return;
	}

	char *p;
	p = mmap(
			NULL, 			// addr: 파일이 매핑되길 원하는 주소. 보통 NULL
			sb.st_size, 	// 매핑 크기. 전체 파일을 매핑하고자 하면 이와 같이 파일 크기를 구해와서 전달하면 됨.
			PROT_READ, 		// prot: 읽기 가능 모드로 매핑한다.(파일을 여는 모드와 일치해야 함)
			MAP_SHARED, 	// flags: 같은 파일을 매핑한 모든 프로세스와 매핑을 공유한다.
			fd, 			// 매핑하고자 하는 파일 디스크립터
			0				// offset: 매핑하고자하는 파일의 옵셋
	); // 매핑 주소를 리턴함.
	if(p == MAP_FAILED)
		errexit("mmap");


/*
	prot:
	 - PROT_NONE: 접근이 불가능한 페이지(reserved 영역)
	 - PROT_READ
	 - PROT_WRITE
	 - PROT_EXEC

	flags:
	 - MAP_FIXED: addr 인자를 요구사항으로 취급함.
	 - MAP_PRIVATE: 매핑이 다른 프로세스와 공유되지 않는다. 수정 발생시 copy-on-write 적용됨
	 - MAP_SHARED: 같은 파일을 매핑한 모든 프로세스와 매핑 내용을 공유함. 수정 발생시 페이지를 읽으면 다른 프로세스에서 기록한 내용도 반영됨.

	페이지 크기
	 - addr와 offset 인자는 별도로 지정하는 경우 반드시 페이지 크기의 배수이어야 함.
	 - 페이지는 MMU에서 사용하는 최소 단위이며 메모리 매핑을 구성하는 블록이자 프로세스 주소 공간을 구성하는 블록.
	 - 원래 파일 사이즈가 페이지의 배수가 아니더라도 다음 크기의 페이지 정수배로 확장되며 남는 공간은 0으로 채워진다.
	 - 시스템의 페이지 크기를 구하는 방법
	   long page_size = sysconf(_SC_PAGESIZE); // #include <unistd.h>

*/

	// 파일 디스크립터를 매핑하면 해당 파일의 참조 카운트가 증가한다.
	// 따라서 매핑 이후에 파일 디스크립터를 닫아도 여전히 매핑 주소에 접근할 수 있다.
	// 매핑에 의한 참조 카운트는 매핑을 해제하거나, 프로세스가 종료할 때 감소한다.
	if(close(fd) == -1)
		errexit("close");

	printf("%s\n", p);


/*
	mremap 예제 - 매핑 영역의 크기를 확장하거나 축소한다.
*/
	char *new_p;
	size_t newsize;
	newsize = sb.st_size + 100;
	if((new_p = mremap(
			p, 					// addr: 변경할 주소
			sb.st_size, 		// old_size: 기존 매핑 크기
			newsize, 			// new_size: 변경할 매핑 크기
			MREMAP_MAYMOVE		// flags : 0 또는 MREMAP_MAYMOVE만 가능. 크기 변경 요청을 수행할 때 필요하다면 매핑의 위치를 변경해도 괜찮음을 커널에 알려줌.
			)) == MAP_FAILED)	// 새로운 매핑 주소를 리턴함.
		errexit("mremap");

/*
	매핑의 보호 모드를 쓰기로 변경
	PROT_READ로 열었는데 PROT_WRITE로 변경한다.
	대신 매핑 대상 파일을 열때도 반드시 O_RDWR와 같이 쓰기 가능모드로 열어야 한다.
*/
	if(mprotect(new_p, sb.st_size + 100, PROT_WRITE) == -1)
		errexit("mprotect");

/*
	파일과 매핑의 동기화 - 매핑된 파일에 대한 변경 내용을 실제 디스크에 기록하여 파일과 매핑을 동기화함.
	msync 호출 전에는 매핑이 해제되기 전까지는 매핑된 메모리의 내용이 디스크로 반영된다는 보장이 없다.
	동기화 방식
	- MS_SYNC: 디스크에 모든 페이지를 기록한 후에 리턴함
	- MS_ASYNC: 비동기 방식으로 동기화, msync는 즉시 리턴됨.
	- MS_INVALIDATE: 매핑의 캐시 복사본을 모두 무효화하고 무효화된 매핑에 접근하면 새롭게 동기화된 디스크의 내용을 반영함.
*/
	if(msync(new_p, newsize, MS_ASYNC) == -1)
		errexit("msync");

/*
	매핑 힌트를 커널에 알려주기
	힌트에 따라 커널은 의도한 용도에 맞게 매핑 동작 방식을 최적화한다.
	힌트의 종류
	- MADV_NORMAL
	- MADV_RANDOM: 미리 읽기를 사용하지 않고 매번 물리적인 읽기 과정에서 최소한의 데이터만 읽어옴
	- MADV_SEQUENTIAL: 공격적으로 미리 읽기를 수행
	- MADV_WILLNEED: 커널이 미리 읽기를 활성화하고 주어진 페이지를 메모리로 읽어들임.
	readahread
	- 읽기 요청이 있을 때 그 다음 영역도 미리 읽는 방식
	- 순차적으로 접근하는 파일은 미리 읽기 윈도우가 크면 유리하고 랜덤하게 접근하는 파일의 경우 성능을 소모한다.
	- 커널은 미리 읽기 윈도우 내의 적중률(hit rate)에 따라 미리 읽기 윈도우의 크기를 동적으로 조절한다.
*/

	if(madvise(new_p, newsize, MADV_SEQUENTIAL) == -1)
		errexit("madvise");

	// mremap 테스트
	strcpy(new_p, "remap works?");
	printf("%s\n", new_p);

	// 생성된 매핑을 해제한다.
	// 일반적으로 mmap의 리턴값과 mmap 파라미터 len 값을 사용함.
	// munmap 이후 매핑 영역에 접근하려고 하면 SIGBUS 시그널이 발생함.
	if(munmap(p, sb.st_size) == -1)
		errexit("munmap");

}

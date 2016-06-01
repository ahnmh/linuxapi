/*
 * ipc_posix_shm.c
 *
 *  Created on: Jun 1, 2016
 *      Author: ahnmh-vw
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ipc_posix_func.h"
#include "tlpi_hdr.h"

/*
POSIX 공유 메모리
/dev/shm 디렉토리에 마운트된 전용의 tmpfs 파일 시스템 아래에서 생성되는 파일을 사용하여 공유 메모리 객체를 생성할 수 있다.
커널 지속성이 있으나 시스템이 종료되면 사라짐.
일반적으로 공유 영역에 접근을 동기화하기 위해 세마포어(또는 기타 동기화 방법)을 사용해야 함.
$ ll /dev/shm/
-rw-------  1 ahnmh-vw ahnmh-vw     1000 Jun  1 20:32 posix_shared_memory
*/

void ipc_posix_shm_write()
{
	int flags;
	mode_t perms;
	int shmfd;

	flags = O_CREAT | O_EXCL | O_RDWR; // 접근 권한은 O_RDWR이어야 함.
	perms = S_IRUSR | S_IWUSR;

	// 공유 메모리 객체 생성 - 공유 메모리를 가르키는 파일 디스크립터를 리턴함.
	shmfd = shm_open(POSIX_SHM, flags, perms);
	if(shmfd == -1)
		errExit("shm_open()");

	// 공유 메모리 객체 크기 조정 - 파일 크기 조정하는 함수를 그대로 사용함.
	// 시스템 V 공유 메모리는 shmget을 통해 생성시 고정되나, POSIX 공유 메모리 객체의 경우 파일처럼 크기 조정이 가능하다.
	char *message = "This is sample string.";
	if(ftruncate(shmfd, strlen(message)) == -1)
		errExit("ftruncate()");

	// 프로세스간 공유를 위해 공개 메모리 매핑을 수행한다. 이 때 공유 메모리 객체에 해당하는 파일 디스크립터를 전달한다.
	void *addr = NULL;
	addr = mmap(NULL, strlen(message), PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
	if(addr == MAP_FAILED)
		errExit("mmap()");

	memcpy(addr, message, strlen(message));
}

void ipc_posix_shm_read()
{
	int shmfd;
	char *addr;
	struct stat sb;

	// open을 쓰면 "ENOENT No such file or directory"가 발생하므로 주의할 것!, shm_open을 써야 함.
	shmfd = shm_open(POSIX_SHM, O_RDONLY, 0);
	if(shmfd == -1)
		errExit("shm_open()");

	// 파일 크기를 얻어옴.
	if(fstat(shmfd, &sb) == -1)
		errExit("fstat()");

	// 얻어온 파일 크기를 통해 매핑 크기를 결정한다.
	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, shmfd, 0);
	if(addr == MAP_FAILED)
		errExit("mmap()");

	write(STDOUT_FILENO, addr, sb.st_size);
	printf("\n");

}

void ipc_posix_shm_destroy()
{
	if(shm_unlink(POSIX_SHM) == -1)
		errExit("shm_unlink()");
}

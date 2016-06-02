/*
 * file_lock_flock.c
 *
 *  Created on: Jun 2, 2016
 *      Author: ahnmh-vw
 */
#include <sys/file.h>
#include <stdio.h>
#include <fcntl.h>

#include "file_lock.h"
#include "tlpi_hdr.h"

/*
flock 함수를 사용한 전체 파일 잠금:
여러 프로세스의 파일 동시 접근을 동기화하기 위해 세마포어를 사용할 수 있지만,
커널은 자동적으로 파일을 잠금과 연관짓기 때문에 파일 잠금을 사용하는 방법이 선호된다.
자식은 부모의 파일 디스크립터의 복사본을 획득하고 동일한 열린 파일 디스크립션, 잠금을 가르킨다.
따라서 부모가 얻은 잠금을 해제할 수 있다.

제약 사항:
전체 파일만을 잠글수 있으므로, 여러 프로세스가 동일 파일의 다른 위치에 접근해서 작업하는 것은 불가함.
많은 NFS 구현은 flock을 통해 구현된 잠금을 인식하지 못함.
*/

void file_lock_flock(int argc, char *argv[])
{
	int fd, lock;
	const char *file = "sample";

	switch(getopt(argc, argv, "se")) {
	case 's':
		// 공유 파일 잠금.
		// 다른 프로세스로부터의 잠금 시도는 동일한 공유 파일 잠금일때만 가능
		lock = LOCK_SH;
		break;
	case 'e':
		// 전용 파일 잠금
		// 다른 프로세스로부터의 잠금 시도는 불가능함.
		lock = LOCK_EX;
		break;
	}

	// sample이라는 이름의 파일은 이미 있다고 가정
	fd = open(file, O_RDONLY);
	if(fd == -1)
		errExit("open()");

	// 전체 파일 접근에 lock을 건다.
	// 잠금 옵션 중, LOCK_NB(논블록)이 있는 경우 EWOULDBLOCK으로 즉시 실패한다.
	if(flock(fd, lock) == -1) {
		if(errno == EWOULDBLOCK)
			fatal("file is locked already.\n");
		else
			errExit("flock()");
	}

	// Lock 상태로 10초간 소유한다.
	printf("lock(type = %d) was acquired.\n", lock);
	sleep(10);

	// lock을 해제
	if(flock(fd, LOCK_UN) == -1)
		errExit("flock()");
}



// 파일의 특정 위치에 대해서 잠금을 get / set 하는 함수
static int lock_region(int fd, int cmd, int type, int whence, int start,
		int len)
{
	struct flock fl;

	fl.l_type = type;
	fl.l_whence = whence;
	fl.l_start = start;
	fl.l_len = len;

	return fcntl(fd, cmd, &fl);
}

// 지정한 파일 영역에 대해 비블록킹 모드로 잠근다.
int lock_region_nblock(int fd, int type, int whence, int start, int len)
{
	return lock_region(fd, F_SETLK, type, whence, start, len);
}

// 지정한 파일 영역에 대해 블록킹 모드로 잠근다.
int lock_region_block(int fd, int type, int whence, int start, int len)
{
	return lock_region(fd, F_SETLKW, type, whence, start, len);
}

// 지정한 영역이 Lock 상태이면 해당 Lock을 보유중인 PID를 리턴하고 아니면 0을 리턴함.
pid_t lock_region_is_locked(int fd, int type, int whence, int start, int len)
{
	struct flock fl;

	fl.l_type = type;
	fl.l_whence = whence;
	fl.l_start = start;
	fl.l_len = len;

	if (fcntl(fd, F_GETLK, &fl) == -1)
		return -1;

	return (fl.l_type == F_UNLCK) ? 0 : fl.l_pid;

}

// ./LinuxAPI2 -w (b or n) 0 10
void file_lock_region(int argc, char *argv[])
{
	int fd;
	const char *file = "sample";

	fd = open(file, O_RDWR);
	if(fd == -1)
		errExit("open()");

	int start, len;
	start = strtol(argv[3], NULL, 10);
	len = strtol(argv[4], NULL, 10);
	pid_t pid;

	switch(getopt(argc, argv, "w:r:")) {
	case 'w':
		pid = lock_region_is_locked(fd, F_WRLCK, SEEK_SET, start, len);
		if(!pid)
			printf("region is free now.\n");
		else
			printf("region is locked by pid = %d.\n", pid);

		if(strcmp(optarg, "b") == 0) {
			lock_region_block(fd, F_WRLCK, SEEK_SET, start, len);
			printf("region was locked!\n");
		}
		else if(strcmp(optarg, "n") == 0) {
			lock_region_nblock(fd, F_WRLCK, SEEK_SET, start, len);
			if(errno == EAGAIN)
				errExit("region was locked by others.\n");

		}
		break;
	case 'r':
		pid = lock_region_is_locked(fd, F_RDLCK, SEEK_SET, start, len);
		if(!pid)
			printf("region is free now.\n");
		else
			printf("region is locked by pid = %d.\n", pid);

		if(strcmp(optarg, "b") == 0) {
			lock_region_block(fd, F_RDLCK, SEEK_SET, start, len);
			printf("region was locked!\n");
		}
		else if(strcmp(optarg, "n") == 0) {
			lock_region_nblock(fd, F_RDLCK, SEEK_SET, start, len);
			if(errno == EAGAIN)
				errExit("region was locked by others.\n");
		}
		break;
	}

	pause();

}

/*
시스템 전체 파일 잠금 보기
$ cat /proc/locks
1: POSIX  ADVISORY  WRITE 7029 08:01:917933 0 0
2: POSIX  ADVISORY  WRITE 7029 08:01:917756 0 0
3: POSIX  ADVISORY  READ  3865 08:01:917672 128 128
4: POSIX  ADVISORY  READ  3865 08:01:917670 1073741826 1073742335
5: POSIX  ADVISORY  READ  4360 08:01:917672 128 128
6: POSIX  ADVISORY  READ  4360 08:01:917670 1073741826 1073742335
7: POSIX  ADVISORY  READ  4255 08:01:917672 128 128
8: POSIX  ADVISORY  READ  4255 08:01:917670 1073741826 1073742335
9: FLOCK  ADVISORY  WRITE 3170 00:13:1103 0 EOF
10: FLOCK  ADVISORY  WRITE 2902 00:16:4 0 EOF
11: FLOCK  ADVISORY  WRITE 2819 00:13:1092 0 EOF
12: POSIX  ADVISORY  READ  9438 08:01:917672 128 128
13: POSIX  ADVISORY  READ  9438 08:01:917670 1073741826 1073742335
...

필드 설명 :
잠금 번호
flock을 사용한 잠금인 경우 FLOCK
fcntl을 사용한 잠금인 경우 POSIX
권고 잠금인 경우 ADVISORY, 의무 잠금인 경우 MANDATORY
잠금 모드 READ, WRITE
잠금을 소유한 PID
파일을 가진 파일 디바이스 Major:Minor와 파일의 i-node 값
잠금의 시작, 끝 위치. FLOCK인 경우 무조건 전체 잠금이므로 0, EOF

3번 파일 잠금 소유 프로세스 확인
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ ps -p 3865
  PID TTY          TIME CMD
 3865 ?        00:01:57 compiz

major:minor 디바이스 번호로 파일 디바이스 찾기. 패턴검색을 수행하는 awk 명령을 사용한다. $6, $7q 변수는 각각 출력의 6번째 7번째 문자열을 의미한다.
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ ls -li /dev | awk '$6 == "8," && $7 ==1'
368 brw-rw----  1 root disk      8,   1 Jun  1 20:00 sda1

디바이스 sda1과 마운트된 디렉토리는 찾기 : /
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ mount | grep sda1
/dev/sda1 on / type ext4 (rw,errors=remount-ro)

/(root) 디렉토리에서 i-node 번호를 통해 최종적으로 잠금이 걸린 파일을 찾을 수 있다.
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ sudo find / -mount -inum 917670
/home/ahnmh-vw/.local/share/zeitgeist/activity.sqlite
*/

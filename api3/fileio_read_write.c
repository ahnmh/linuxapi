/*
 * fileio_read_write.c
 *
 *  Created on: Jun 24, 2016
 *      Author: ahnmh-vw
 */

#include "fileio.h"

/*
표준 파일 디스크립터
- 표준 입력(0) = STDIN_FILENO 일반적으로 키보드와 같은 터미널의 입력 장치에 연결됨.
- 표준 출력(1) = STDOUT_FILENO 터미널의 출력 장치에 연결됨.
- 표준 에러(2) = STDERR_FILENO
*/

void fileio_read_write(int argc, char *argv[])
{
	const char *file = "test";
	int fd;

	char buf[BUF_SIZE];
	ssize_t numread;

	// O_CREAT 플래그를 부여하여 파일을 생성할 때 반드시 접근 권한을 지정해야 함.
	// 그렇지 않은 경우, O_RDWR 플래그를 사용했음에도 쓰기권한이 생성되지 않는다.
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	switch(getopt(argc, argv, "crw")) {
	case 'c':
		fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, mode);
		/*
			// 또는 creat 함수를 사용해도 똑같음
			fd = creat("test", mode);
		*/
		if(fd == -1)
			errexit("open");

		break;

	case 'r':
		fd = open(file, O_RDONLY);
		if(fd == -1)
			errexit("open");

/*
		파일 입출력 힌트 제공
		힌트의 종류
		- POSIX_FADV_NORMAL: 일반 모드. 적당한 미리 읽기 수행
		- POSIX_FADV_RANDOM: 미리 읽기를 수행하지 않고 매번 읽기마다 최소한의 데이터만 읽는다
		- POSIX_FADV_SEQUENTIAL: 커널은 공격적인 미리 읽기를 수행. 미리 읽기 윈도우 크기를 2배로 늘림.
		- POSIX_FADV_WILLNEED: 커널이 미리 읽기를 활성화하여 주어진 페이지 전체를 메모리로 읽어들임.
		- POSIX_FADV_DONTNEED: 커널은 주어진 범위 내의 데이터 중 캐싱 중인 데이터를 페이지 캐시에서 제거함.
*/
		if(posix_fadvise(
				fd,
				0, 						// offset: 힌트를 지정할 파일 옵셋. 전체 파일 대상인 경우 0
				0, 						// len: 힌트를 지정할 파일의 옵셋위치에서부터 길이. 전체 파일 대상인 경우 0
				POSIX_FADV_SEQUENTIAL	// advice: 지정한 범위 내에 있는 데이터에서 낮은 주소에서 높은 주소로 순차 접근한다.
				) == -1)
			errexit("posix_fadvise");

		// 파일 읽기
		char *idx = buf;
		int len = BUF_SIZE;
		// 반복문을 사용해서 지정한 크기의 전체 바이트 읽기를 보장한다.
		while(len != 0 && (numread = read(fd, idx, len)) != 0) {
			if(numread == -1) {
				if(errno == EINTR) // 읽기 시도 이전에 시그널에 인터럽트 된 경우 읽기 재시도
					continue;
				errexit("read"); // 그 이외는 에러 처리
			}

			len -= numread;
			idx += numread;
		}

		*idx = '\0';
		printf("%s\n", buf);

		break;

	case 'w':
		// append 모드로 쓰려면 O_APPEND
		fd = open(file, O_WRONLY/*|O_APPEND*/);
		// 표준 입력에서 읽어서 파일로 쓴다.
		if((numread = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
			// 파일 옵셋을 맨 앞으로 이동하여 나중에 쓴 데이터가 앞에 나오도록 설정,
			// 단, O_APPEND 플래그가 지정되면 해당 플래그에 의해 무조건 옵셋이 맨뒤로 가기 때문에 적용되지 않음.
			off_t ret;
			if((ret = lseek(fd, 0, SEEK_SET)) == -1)
				errexit("lseek");
			if(write(fd, buf, numread) != numread)
				errexit("write");
		}

		// 버퍼에 쓰여진 쓰기 데이터를 디스크로 즉시 쓰기(메타데이터는 갱신하지 않음)
		// open 함수 호출 시 O_SYNC 플래그를 사용하거나, sync 함수를 호출할 수도 있지만 fdatasync가 성능상 가장 유리함.
		if(fdatasync(fd) == -1)
			errexit("fdatasync");

		break;

	}

	// 열려 있는 파일 디스크립터 fd에 연관된 파일과의 매핑을 해제
	// 파일을 닫기 전에 디스크에 확실히 기록하려면 동기식 입출력 방법을 사용해야 함.
	if(close(fd) == -1)
		errexit("close");

}

/*
open 함수에 지정 가능한 플래그
O_APPEND: append 모드. 매번 쓰기 작업 직전에 파일 옵셋을 파일 끝을 가르키도록 갱신.
O_ASYNC: 특정 파일(stdin, 소켓)에서 읽기나 쓰기가 가능해질 때 시그널이 발생함(SIGIO)
O_CLOEXEC: 새 프로세스가 실행되면 파일은 자동으로 닫힌다.새 프로세스가 생성되면서 기존의 파일 디스크립터를 복사해서 생기는 경쟁을 사전에 방지함.
O_CREAT: 새로 만듬
O_EXCL: O_CREAT와 함께 사용하면 파일이 이미 있는 경우 open호출이 실패함. 파일 생성시 경쟁 상태를 회피함.
O_NONBLOCK: 파일을 논블로킹 모드로 연다.
O_SYNC: 파일을 동기식 입출력용으로 연다. 데이터를 물리적으로 디스크에 쓰기 전까지 쓰기 동작이 완료되지 않는다.(버퍼에 쓰여진 쓰기 데이터를 디스크로 즉시 쓰기)
         O_SYNC 플래그를 명시하지 않은 디폴트 동작은 데이터가 커널 버퍼에 저장되면 완료된다.
O_TRUC: 파일이 존재하고 일반 파일이며 flags 인자에 쓰기가 가능하도록 명시되어 있으면 파일 길이를 0으로 설정함.
O_DIRECT: 페이지 캐시를 우회해서 사용자 영역 버퍼에서 디스크 장치로 바로 입출력을 수행함. 단, 입출력 요청 크기, 버퍼 정렬, 파일 옵셋은 디바이스의 섹터 크기(512)의 정수배가 되어야 함.
*/

/* stat 명령을 통해 생성한 파일 test를 확인
ahnmh-vw@ubuntu:~/workspace/linuxapi/api3/Debug$ stat test
  File: ‘test’
  Size: 0         	Blocks: 0          IO Block: 4096   regular empty file
Device: 801h/2049d	Inode: 1056941     Links: 1
Access: (0664/-rw-rw-r--)  Uid: ( 1000/ahnmh-vw)   Gid: ( 1000/ahnmh-vw)
Access: 2016-06-24 10:38:00.164259428 +0900
Modify: 2016-06-24 10:38:00.164259428 +0900
Change: 2016-06-24 10:38:00.164259428 +0900
 Birth: -

*/

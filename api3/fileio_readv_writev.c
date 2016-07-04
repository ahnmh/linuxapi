/*
 * fileio_readv_writev.c
 *
 *  Created on: Jul 4, 2016
 *      Author: ahnmh-vw
 */
#include <sys/types.h> // iovec
#include <string.h>
#include <sys/uio.h> // readv, writev

#include "fileio.h"

/*
벡터 입출력
: 데이터가 미리 정의된 구조체의 여러 필드에 걸쳐 분리되어 있는 경우 사용함
: 시스템 콜의 횟수를 줄여주며 아토믹함.
*/
void fileio_readv_writev()
{
	// iovec = 세그먼트라고 하는 독립적으로 분리된 버퍼를 제공하며 세그먼트의 집합을 벡터라고 함.
	struct iovec iov[3];
	ssize_t numwrite;
	int fd;

	char *buf[] = {
			"Hyunsoo Kim\n",
			"Daeho Lee\n",
			"Shinsoo Choo\n"
	};

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	fd = open("test", O_WRONLY | O_CREAT | O_TRUNC, mode);
	if(fd == -1)
		errexit("open");

	int i;
	// 각 버퍼의 시작 주소와 크기를 iovec에 지정함.
	for(i = 0; i < 3; i++) {
		iov[i].iov_base = buf[i];
		iov[i].iov_len = strlen(buf[i]) + 1;
	}

	// count의 최대값은 IO_MAX = 1024
	// 최적의 카운트 : 8 이하면 동적할당하지 않고 프로세스의 커널 스택을 사용하므로 성능이 좋아짐.
	numwrite = writev(fd, iov, 3);
	if(numwrite == -1)
		errexit("writev");

	printf("write bytes = %ld\n", numwrite);

	if(close(fd))
		errexit("close");


	// 읽기 예제
	ssize_t numread;
	fd = open("test", O_RDONLY);
	if(fd == -1)
		errexit("open");

	// 읽어올 정확한 길이를 미리 알고 있어야 한다. 즉, iov_len = BUF_SIZE와 같이 지정하면 안된다.
	// 이 샘플처럼 데이터 크기가 가변 길이 일때는 별로임.
	char readbuf[3][BUF_SIZE];
	for (i = 0; i < 3; ++i) {
		iov[i].iov_base = readbuf[i];
		iov[i].iov_len = strlen(buf[i]) + 1;
	}

	numread = readv(fd, iov, 3);
	if(numread == -1)
		errexit("writev");

	for (i = 0; i < 3; ++i)
		printf("%s", readbuf[i]);

	if(close(fd))
		errexit("close");

}

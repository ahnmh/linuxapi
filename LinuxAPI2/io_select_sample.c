/*
 * io_select_sample.c
 *
 *  Created on: Jun 21, 2016
 *      Author: ahnmh-vw
 */

#include <sys/time.h>
#include <sys/select.h>
#include "tlpi_hdr.h"
#include <fcntl.h>

void io_select_sample()
{
	// readfds : 입력이 가능한지를 확인할 파일 디스크립터 집합
	// writefds : 출력이 가능한지를 확인할 파일 디스크립터 집합
	fd_set readfds, writefds;
	int ready; // select는 파라미터로 전달한 집합 중 몇개의 디스크립터가 I/O가 가능한지를 리턴한다.
	int nfds; // I/O가 가능한지 확인하려는 디스크립터의 값 + 1
	int fd;

	// select 함수가 최대 대기하는 시간.
	// tv_sec = 0, tv_usec = 0인 경우, 기다리지 않고 리턴하며, NULL로 지정하면 무한대로 기다린다.
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	nfds = 0;
	FD_ZERO(&readfds); // 입력 확인 디스크립터 집합 초기화 매크로
	FD_ZERO(&writefds); // 출력 확인 디스크립터 집합 초기화 매크로

	fd = 0;
	nfds = fd + 1;
	FD_SET(fd, &readfds); // fd를 입력 확인 디스크립터 집합에 추가하는 매크로
	FD_SET(fd, &writefds); // fd를 출력 확인 디스크립터 집합에 추가하는 매크로

	// 지정한 입력, 출력, 예외 디스크립터(NULL) 집합중 I/O가 가능한 디스크립터가 발생하면 리턴한다.
	ready = select(nfds, &readfds, &writefds, NULL, &timeout);
	if (ready == -1)
		errExit("select()");

	// select가 리턴했을 때 입력, 출력이 가능한 디스크립터 집합의 갯수
	printf("ready = %d\n", ready);

	// 디스크립터 번호와 가능한 I/O 속성 확
	for (fd = 0; fd < nfds; fd++)
		printf("%d: %s%s\n", fd, FD_ISSET(fd, &readfds) ? "r" : "", FD_ISSET(fd, &writefds) ? "w" : "");


}

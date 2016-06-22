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

/*
select, poll
레벨 트리거 통지 : 지정한 파일 디스크립터에 대해 I/O 시스템 호출이 가능한 경우 알림
예를 들어, 파이프의 읽기 엔드포인트에 해당하는 디스크립터를 select나 poll이 감시하고 있는 상황에서 쓰기 엔드포인트에 데이터를 쓰면 select나 poll이 리턴된다.
일반 파일, 터미널, 가상 터미널, 파이프, FIFO, 소켓, 몇몇 문자 관련 디바이스의 파일 디스크립터를 감시할 수 있다.
*/

void alt_io_select()
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

/*
	fd = 0은 stdin을 의미하며, stdin에 read가 가능할 때(사용자 입력이 감지되는 경우) select가 리턴된다.
	write 가능 여부를 감지하면 언제나 리턴된다.
*/

	fd = 0;
	nfds = fd + 1;
	FD_SET(fd, &readfds); // fd를 입력 확인 디스크립터 집합에 추가하는 매크로
//	FD_SET(fd, &writefds); // fd를 출력 확인 디스크립터 집합에 추가하는 매크로

	// FD_SET의 반대는 FD_CLR

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


#include <time.h>
#include <poll.h>
void alt_io_poll()
{
	int ready;
	int (*pfds)[2]; // 파이프 1개는 pfd[2]이므로((0 = 읽기, 1 = 쓰기) 여러 파이프를 사용하기 위해 포인터로 가르킨다.
	int numpipes = 10; // 파이프 갯수는 10개
	int i;

	// 파이프 10개 생성
	pfds = calloc(numpipes, sizeof(int [2]));
	for (i = 0; i < numpipes; ++i) {
		if(pipe(pfds[i]) == -1)
			errExit("pipe()");
	}

	// pollfd 구조체 배열은 poll 함수가 감시할 파일 디스크립터 목록을 기술한다.
	// pollfd 구조체 필드
	// fd : 파이프의 0번 인덱스는 읽기를 의미하고 각 파이프(10개)의 읽기 엔드포인트들을 감시 대상 디스크립터로 fd에 설정하고
	// event : 해당 디스크립터들의 감시 이벤트(events)로서 읽기(POLLIN)를 지정한다.
	// revent : 실제 이벤트가 발생하면 해당 이벤트에 해당하는 bit가 set됨.

	// 이벤트 종류
	// POLLIN = 읽기 가능
	// POLLOUT = 쓰기 가능
	// POLLERR = 에러 발생
	// POLLHUP = 장애 발생
	// POLLNVAL = 파일 디스크립터가 열리지 않음
	struct pollfd *pollFd;
	pollFd = calloc(numpipes, sizeof(struct pollfd));
	for (i = 0; i < numpipes; ++i) {
		pollFd[i].fd = pfds[i][0];
		pollFd[i].events = POLLIN;

	}

	// 파이프의 쓰기 엔드포인트에 데이터를 써주면 읽기가 가능해진다.
	if(write(pfds[5][1], "a", 1) == -1)
		errExit("write(): %d");
	if(write(pfds[6][1], "a", 1) == -1)
		errExit("write(): %d");

	// write에 의해 읽기 엔드포인트에 이벤트가 발생했으므로,
	// POLLIN 이벤트가 발생한 구조체 갯수를 리턴한다.
	// 이벤트가 발생한 구조체는 .revent 필드가 set된다.
	// timeout이 0이므로 이벤트가 발생하지 않아도 대기없이 리턴함.
	ready = poll(pollFd, numpipes, 0);
	if(ready == -1)
		errExit("ready()");

	printf("poll() returned: %d\n", ready);
}

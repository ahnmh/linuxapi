/*
 * fileio_epoll.c
 *
 *  Created on: Jul 4, 2016
 *      Author: ahnmh-vw
 */

#include <sys/epoll.h>
#include "fileio.h"

#define MAX_EVENTS 5

/*
epoll
: poll과 select는 실행할 때마다 전체 파일 디스크립터를 요구한다. 커널은 검사해야 할 모든 파일 리스트를 다 살펴본다.
: epoll은 실제로 검사하는 부분과 검사할 파일 디스크립터를 등록하는 부분을 분리하여 병목 현상을 회피한다.
*/
void epoll_sample()
{
	int epfd;
	// 새로운 epoll 인스턴스를 생성하고 인스턴스를 가르키는 파일 디스크립터(실제 파일과는 무관함)를 리턴함.
	// epoll_create1이 최신버전이다.
	// 파라미터는 flags이며 현재 EPOLL_CLOSEXEC만 가능
	epfd = epoll_create1(0);
	if(epfd == -1)
		errexit("epoll_create1");

	// FIFO 열기
	int fd;
	fd = open("test", O_RDONLY);
	if(fd == -1)
		errexit("open()");

	// 감시할 파일 디스크립터와 감시할 이벤트를 추가함
	struct epoll_event event;
	int ret;
	event.data.fd = fd; // 이벤트가 실제 발생할 때 사용자에게 반환하는 사용자 데이터 필드
	event.events = EPOLLIN; // 감시할 이벤트는 읽기
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
	if(ret == -1)
		errexit("epoll_ctl");

	// 이벤트가 발생하면 기록될 이벤트 구조체
	struct epoll_event evlist[MAX_EVENTS];
	int numevent;

	// timeout = -1이면 무한 대기
	numevent = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
	if(numevent == -1)
		errexit("epoll_wait");

	// 발생한 이벤트 확인
	int i;
	ssize_t numread;
	char buf[BUF_SIZE];
	for (i = 0; i < numevent; ++i) {
		printf("event = %d, on fd = %d\n", evlist[i].events, evlist[i].data.fd);
		numread = read(fd, buf, BUF_SIZE);
		if(numread == -1)
			errexit("read");
		buf[numread] = '\0';
		printf("read = %ld, %s\n", numread, buf);

	}

	close(fd);
	close(epfd); // epoll 인스턴스를 가르키는 핸들도 반드시 close해주어야 함.
}

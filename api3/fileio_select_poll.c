/*
 * fileio_select_poll.c
 *
 *  Created on: Jun 28, 2016
 *      Author: ahnmh-vw
 */

#define _XOPEN_SOURCE 600 // pselect

#include <sys/time.h>
#include <sys/select.h> // pselect
#include <signal.h> // SIGINT
#include <poll.h> // poll
#include "fileio.h"

#define MAX_BUF 1000 // 한번의 read로 읽을 수 있는 최대 바이트 수
#define FD_COUNT 2 // 감시 대상 파일 디스크립터(예제에서는 FIFO를 가르킴)의 수


/*
1. 터미널 A에서 FIFO를 생성하여 쓰기 엔드포인트를 생성한다.
ahnmh-vw@ubuntu:~/workspace/linuxapi/api3/Debug$ mkfifo test
ahnmh-vw@ubuntu:~/workspace/linuxapi/api3/Debug$ cat > test

2. 터미널 B에서 프로그램을 실행
ahnmh-vw@ubuntu:~/workspace/linuxapi/api3/Debug$ ./api3
FIFO opened on fd 3 ----> 쓰기 엔드포인트가 열림으로써 fd = open 함수가 리턴됨

3. 터미널 A에서 쓰기
12345

4. 터미널 B의 출력
read bytes = 6, 12345
*/

void select_sample()
{
	int fd;
	// FIFO를 연다.
	// 이 동작은 클라이언트가 FIFO가 O_WRONLY 모드로 open 할때까지(쓰기 endpoint가 생길때까지) 블록된다.
	// 예를 들면 $ cat > test1 과 같은 명령
	// 이 샘플은 FIFO의 읽기 endpoint이므로 반드시 O_RDONLY 플래그를 사용해서 FIFO를 열어야 한다.
	fd = open("test", O_RDONLY);
	if(fd == -1)
		errexit("open()");
	printf("FIFO opened on fd %d\n", fd);

	fd_set readfds;
	struct timeval tv;
	int ready;

	while(1) {
		FD_ZERO(&readfds); // 읽기 감시 대상 파일 디스크립터 집합 초기화
		FD_SET(fd, &readfds); // 읽기 감시 대상 파일 디스크립터 설정

		// timeout은 5초 - select 리턴이후 잔여시간이 기록되므로 계속 reset 해주어야 함.
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		// select는 읽기 감시 수행
		ready = select(fd + 1, &readfds, NULL, NULL, &tv);
		if(ready == -1)
			errexit("select");
		else if(ready == 0) {
			printf("time elapsed.\n");
			return;
		}

		// select가 리턴되었을 때, FIFO 파일 디스크립터 읽기가 가능한지 확인
		if(FD_ISSET(fd, &readfds)) {
			char buf[MAX_BUF];
			ssize_t numread;
			numread = read(fd, buf, MAX_BUF);
			if(numread == -1)
				errexit("read");

			if(numread) {
				buf[numread] = '\0';
				printf("read bytes = %ld, %s\n", numread, buf);
			}
		}
	}
}

void pselect_sample()
{
	int fd;
	fd = open("test", O_RDONLY);
	if(fd == -1)
		errexit("open()");
	printf("FIFO opened on fd %d\n", fd);

	fd_set readfds;
	struct timespec ts; // timeval 대신 timespec을 사용함.
	ts.tv_sec = 5;
	ts.tv_nsec = 0;

	// 블록할 시그널 구성
	sigset_t blockmask;
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SIGINT);

	int ready;

	while(1) {
		FD_ZERO(&readfds); // 읽기 감시 대상 파일 디스크립터 집합 초기화
		FD_SET(fd, &readfds); // 읽기 감시 대상 파일 디스크립터 설정

		// pselect에 의해 대기중일때 SIGINT를 무시하도록 설정한다.
		// pselect는 timespec 구조체를 timeout으로 받으며 루프를 돌때마다 새로 설정할 필요가 없다.
		ready = pselect(fd + 1, &readfds, NULL, NULL, &ts, &blockmask);
		if(ready == -1)
			errexit("select");

		if(ready == 0) {
			printf("time elapsed.\n");
			return;
		}

		if(FD_ISSET(fd, &readfds)) {
			char buf[MAX_BUF];
			ssize_t numread;
			numread = read(fd, buf, MAX_BUF);
			if(numread == -1)
				errexit("read");

			if(numread) {
				buf[numread] = '\0';
				printf("read bytes = %ld, %s\n", numread, buf);
			}
		}
	}
}

void poll_sample()
{
	int fd;
	fd = open("test", O_RDONLY);
	if(fd == -1)
		errexit("open()");
	printf("FIFO opened on fd %d\n", fd);

	// 감시하고자 하는 단일 파일 디스크립터를 명시
	// 구조체 배열을 통해 여러개의 파일 디스크립터를 감시할 수 있다.
	struct pollfd fds;
	int ready;

	fds.fd = fd; // 감시 대상 디스크립터
	fds.events = POLLIN; // 감시 이벤트는 읽기

	while(1) {
		// pollfd구조체로 지정한 디스크립터의 읽기 감시
		// pollfd 구조체 갯수는 1개
		// 5000 = timeout은 5초
		ready = poll(&fds, 1, 5000);

		if(ready == -1)
			errexit("select");

		if(ready == 0) { // timeout
			printf("time elapsed.\n");
			return;
		}

		// poll 함수 리턴시 읽기 이벤트 발생 여부 확인
		if(fds.revents & POLLIN) {
			char buf[MAX_BUF];
			ssize_t numread;
			numread = read(fd, buf, MAX_BUF);
			if(numread == -1)
				errexit("read");

			if(numread) {
				buf[numread] = '\0';
				printf("read bytes = %ld, %s\n", numread, buf);
			}
		}
	}
}

/*
poll의 장점
- 감시해야할 파일 디스크립터의 숫자가 큰 경우 select보다 더 효율적임.
- 파일 디스크립터가 연속적이지 않은 경우 select보다 더 효율적임.
- select는 파일 디스크립터 집합을 반환하는 집합이 재구성되므로 연속 호출시 리셋이 필요하다.
- select는 timeout 인자를 초기화해야 한다.
대신 select가 이식성이 더 높아서 더 많이 사용된다.
*/

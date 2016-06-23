/*
 * io_select_sample.c
 *
 *  Created on: Jun 21, 2016
 *      Author: ahnmh-vw
 */

#include <sys/time.h>
#include <sys/select.h>
#include "tlpi_hdr.h"

/*
select, poll
레벨 트리거 통지 : 지정한 파일 디스크립터에 대해 I/O 시스템 호출이 가능한 경우 알림
예를 들어, 파이프의 읽기 엔드포인트에 해당하는 디스크립터를 select나 poll이 감시하고 있는 상황에서 쓰기 엔드포인트에 데이터를 쓰면 select나 poll이 리턴된다.
일반 파일, 터미널, 가상 터미널, 파이프, FIFO, 소켓, 몇몇 문자 관련 디바이스의 파일 디스크립터를 감시할 수 있다.
주로 읽기와 쓰기 동작을 감시하는데,
읽기의 경우 읽을 수 있는 데이터가 있는지,
쓰기의 경우 쓸 수 있는 충분한 공간이 있는지에 대해 감시함.

일반 파일의 파일 디스크립터는 항상 읽기와 쓰기가 가능한 상태로 간주(즉, select나 poll에 감시를 걸면
블록되지 않고 항상 리턴됨) 되며 이유는 다음과 같다.
- read를 호출하면 데이터, EOF, 에러 중 하나를 즉시 리턴한다.
- write를 호출하면 즉시 데이터를 전송하거나 에러가 발생한다.

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
	// 파라미터로 전달되는 fd_set은 결과값이 기록되기 때문에 만일 select 함수를 재수행하려는 경우 fd_set을 리셋해야 한다.
	ready = select(nfds, &readfds, &writefds, NULL, &timeout);
	if (ready == -1)
		errExit("select()");

	// select가 리턴했을 때 입력, 출력이 가능한 디스크립터 집합의 갯수
	printf("ready = %d\n", ready);

	// 디스크립터 번호와 가능한 I/O 속성 확인
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


/*
signal 기반 I/O
에지 트리거 통지 : 파일 디스크립터에 I/O가 발생하면 커널이 시그널 SIGIO를 보냄.
프로세스는 시그널이 전달되기 전까지 다른 job을 수행할 수 있게 된다.
(시그널을 받으면 콜백 호출과 같이 구현하면 될 듯...)
시그널 기반 I/O에서는 커널이 감시할 파일 디스크립터 목록을 기억할 수 있으므로 실제 I/O 이벤트가 발생했을 때만
프로그램에 시그널을 발생시키므로 성능이 select, poll보다 좋다.
*/

#include <signal.h>
#include <fcntl.h>
#define BUF_SIZE 5
static volatile sig_atomic_t got_sig_io = 0;


static void sigio_handler(int sig)
{
	got_sig_io = 1;
}

void alt_io_signal()
{
	int flags;
	struct sigaction sa;

	// 시그널 핸들러 설치
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART; // 반드시 SA_RESTART 플래그를 설정하여 재시작되도록 해야 함.
	sa.sa_handler = sigio_handler;
	if (sigaction(SIGIO, &sa, NULL) == -1)
		errExit("sigaction()");

	// 지정한 파일 디스크립터의 I/O가 가능할 때 시그널을 수신할 파일 디스크립터의 소유자를 설정(일반적으로 호출 프로세스)
	// 파일 디스크립터의 I/O가 준비되었다는 시그널을 하나의 프로세스 또는 프로세스 그룹의 모든 프로세스가 받도록 설정할 수 있다.
	if (fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1)
		errExit("fcntl()");

	// 논블로킹 I/O를 활성화(O_NONBLOCK), 시그널 기반 I/O를 활성화(O_ASYNC)
	flags = fcntl(STDIN_FILENO, F_GETFL);
	if (fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1)
		errExit("fcntl()");

	char buf[BUF_SIZE + 1];
	while (1) {
		sleep(1);
		// SIGIO 시그널이 발생하면 시그널 핸들러에 의해 if 이하 진입
		// stdin 디스크립터에 대해 동작하므로 입력창에 데이터를 쓰고 엔터를 누르는 순간에 해당함.
		if (got_sig_io) {
			got_sig_io = 0;

			// 시그널 기반 I/O와 같이 에지 트리거 방식일 때, 다음 시그널이 발생하기전 모든 데이터를 읽어들여야 한다.
			// 모두 읽어들이는 것을 시뮬레이션하기 위해 버퍼 사이즈를 고의로 적게 설정하여 여러번 read를 돌게함.
			while (read(STDIN_FILENO, buf, BUF_SIZE) > 0) {
				if(buf[0] == '\n')
					break;
				buf[5] = '\0';
				printf("read = %s\n", buf);
			}
		}
	}

}


/*
epoll API
이벤트폴
감시해야 하는 파일 디스크립터 수가 많을 때 select, poll에 비해 성능이 좋다.
epoll API는 레벨 트리거, 에지 트리거 통지를 모두 지원한다.
시그널 기반 I/O에 비해 시그널을 처리하는 복잡성이 없으며 어떤 종류의 이벤트를 감시할 것인지에 대한 유연성이 더 좋다.
(시그널 기 I/O의 경우, 읽기 감지를 수행할 것인지, 쓰기 감지를 수행할 것인지 지정하는 부분이 없음)
리눅스 전용
파이프, FIFO, 소켓, POSIX 메시지 큐, 터미널, 디바이스를 감시할 수 있으나, 정규 파일의 디스크립터는 불가함.
*/

#include <sys/epoll.h>
#define MAX_BUF 1000 // 한번의 read로 읽을 수 있는 최대 바이트 수
#define FD_COUNT 2 // 감시 대상 파일 디스크립터(예제에서는 FIFO를 가르킴)의 수
#define MAX_EVENTS 5 // epoll 호출을 통해 리턴할 수 있는 최대 이벤트 수

void alt_io_epoll()
{
	int epfd; // epoll 인스턴스를 가르키는 파일 디스크립터.
	int ready; // 발생한 이벤트 갯수
	int fd; // 감시할 파일 디스크립터
	int numopendfds;

	struct epoll_event ev;
	struct epoll_event evlist[MAX_EVENTS];
	char buf[MAX_BUF];

	// epoll 인스턴스 생성
	// epoll 인스턴스는 감시할 파일 디스크립터 목록을 저장하고(관심 목록) I/O 수행 준비가 된 파일 디스크립터 목록을 알려준다.(준비 목록)
	// 리턴되는 디스크립터는 epoll 인스턴스를 가르키는 디스크립터임.
	// size: 감시할 예정인 파일 디스크립터의 수
	epfd = epoll_create(2);

	// 감시 대상 파일을 epoll에 지정한다.
	char filename[6];
	int i;
	for (i = 1; i <= FD_COUNT; ++i) {
		sprintf(filename, "test%d", i);

		// FIFO를 연다.
		// 이 동작은 클라이언트가 FIFO가 O_WRONLY 모드로 open 할때까지(쓰기 endpoint가 생길때까지) 블록된다.
		// 예를 들면 $ cat > test1 과 같은 명
		// 이 샘플은 FIFO의 읽기 endpoint이므로 반드시 O_RDONLY 플래그를 사용해서 FIFO를 열어야 한다.
		fd = open(filename, O_RDONLY);
		if(fd == -1)
			errExit("open()");
		printf("Opened \"%s\" on fd %d\n", filename, fd);

		// EPOLLET는 에지 트리거 방식으로 동작하도록 설정함.
		ev.events = EPOLLIN/* | EPOLLET */; // 감시 이벤트는 읽기 가능 여부
		ev.data.fd = fd; // epoll_wait가 리턴될 때 해당 프로세스로 전달한 컨텍스트. 보통 이벤트가 발생한 파일 디스크립터를 지정한다.
		// EPOLL_CTL_ADD: 파일 디스크립터 fd를 epoll 인스턴스 epfd의 관심 목록에 추가한다.
		if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
			errExit("epoll_ctl()");
	}

	numopendfds = 2;
	while(numopendfds > 0) {
		// 이벤트 발생 대기
		// 준비 상태인 파일 디스크립터 정보를 evlist로 리턴함.
		// maxevents: evlist 배열의 갯수
		// timeout: -1(무한대기)
/*
		epoll은 EPOLLET 플래그를 지정하지 않으면 기본적으로 레벨 트리거 방식으로 동작한다.
		즉, 쓰기가 발생한 이후 read를 통해 읽어가지 않으면 epoll_wait는 항상 리턴된다.(레벨 트리거)
		EPOLLET 플래그를 지정하면 read를 통해 읽어가지 않더라도 다음 쓰기가 발생할 때까지 블록된다.(에지 트리거)
*/
		ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		if(ready == -1) {
			if(errno == EINTR) // 시그널에 의해 인터럽트된 경우 재실행
				continue;
			else
				errExit("epoll_wait()");
		}

		printf("ready = %d\n", ready);

		// 리턴된 이벤트 확인
		int j;
		ssize_t numread;
		for (j = 0; j < ready; ++j) {
			printf("fd = %d, events: %s%s%s\n", evlist[j].data.fd,
					(evlist[j].events & EPOLLIN) ? "EPOLLIN " : "",
					(evlist[j].events & EPOLLHUP) ? "EPOLLHUP " : "",
					(evlist[j].events & EPOLLERR) ? "EPOLLERR " : "");

			// 읽기 가능 이벤트인 경우 읽기 시도
			if(evlist[j].events & EPOLLIN) {
				numread = read(evlist[j].data.fd, buf, MAX_BUF);
				printf("\tread %ld bytes = %.*s\n", numread, numread, buf);
			}
			else if(evlist[j].events & (EPOLLHUP | EPOLLERR)) {
				printf("\tclosing fd %d\n", evlist[j].data.fd);
				if(close(evlist[j].data.fd) == -1)
					errExit("close()");
				numopendfds--;
			}
		}

	}

	printf("All file descriptors was closed\n");

}

/*
프로그램 사용 방법
1. epoll 감시 샘플을 실행한 뒤,
2. FIFO를 2개 생성한다.
$ mkfifo test1 test2
각각 쓰기 endpoint를 먼저 연다.
$ cat > test1
^Z
[1]+  Stopped                 cat > test1
$ cat > test2
^Z
[2]+  Stopped                 cat > test2

<감시 샘플의 출력>
$ ./LinuxAPI2
Opened "test1" on fd 4
Opened "test2" on fd 5

3. FIFO에 데이터를 쓴다.
ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ fg 1
cat > test1
12345

<감시 샘플의 출력>
ready = 1
fd = 4, events: EPOLLIN
	read 6 bytes = 12345

ahnmh-vw@ubuntu:~/workspace/linuxapi/LinuxAPI2/Debug$ fg 2
cat > test2
abcde

<감시 샘플의 출력>
ready = 1
fd = 5, events: EPOLLIN
	read 6 bytes = abcde

4. FIFO 입력을 Ctrl + C로 종료하면,
<감시 샘플의 출력>
ready = 1
fd = 4, events: EPOLLHUP
	closing fd 4
ready = 1
fd = 5, events: EPOLLHUP
	closing fd 5
All file descriptors was closed


*/

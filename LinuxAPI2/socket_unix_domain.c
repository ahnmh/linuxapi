/*
 * socket_unix_domain.c
 *
 *  Created on: Jun 3, 2016
 *      Author: ahnmh-vw
 */

#include "socket_unix_domain.h"
#include <sys/wait.h>

#define BACKLOG 5

/*
유닉스 도메인 스트림(데이터의 경계가 없는 = TCP 방식) 소켓 서버
소켓 생성에 대해 주의할 점
- 이미 존재하는 경로명으로는 소켓을 생성할 수 없다.
- 절대 경로명으로 소켓을 연결해서 파일 시스템의 고정된 주소(경로)를 할당해야 한다.
- 소켓은 하나의 경로명으로 결속할 수 있다. 반대로 경로명도 오직 하나의 소켓에 결속될 수 있다.
- 소켓이 필요 없는 경우, unlink로 경로명을 제거할 수 있다.

ahnmh-vw@ubuntu:/tmp$ ll
...
srwxrwxr-x  1 ahnmh-vw ahnmh-vw    0 Jun  4 10:41 unix_domain_socket=

아래는 단방향 소켓 통신 예제임. 클라이언트 -> 서만 가능.
클라이언트가 서버로부터 메시지를 받으려면 클라이언트도 소켓에 자신의 주소(고유의 경로)를 bind해야 한다.
*/
void socket_unix_domain_stream_server()
{
	// 유닉스 도메인 소켓에서 주소(경로)를 지정할 때 사용하는 구조체
	struct sockaddr_un addr;
	int server_fd, client_fd;
	ssize_t numread;
	char buf[BUF_SIZE];

	// 유닉스 도메인 / 스트림 소켓을 생성
	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

	// 기존에 열려 있는 경로가 있으면 삭제.
	// 소켓은 이미 존재하는 경로를 사용할 수 없다.
	if(remove(SOCKET_PATH) == -1 && errno !=ENOENT)
		errExit("remove()");

	// 서버 소켓 주소를 지정함.
	// 유닉스 도메인 소켓 주소를 지정하는 구조체 socaddr_un 에는 소켓 주소에 경로명을 포함한다.
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX; // 유닉스 도메인
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1); // 마지막 자리는 '\0'으로 남겨둔다.

	// 서버 소켓과 주소 바인딩
	// bind 함수는 유닉스 도메인, 인터넷 도메인 모두 사용되는데, addr 파라미터는 표준 형태인 sockaddr로 형변환해야 함.
	if(bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
		errExit("bind()");

	// 서버 소켓으로 들어오는 연결 요청을 감시
	if(listen(server_fd, BACKLOG) == -1)
		errExit("listen()");

	while(1) {
		// 클라이언트 연결 요청을 수락하고 클라이언트와의 통신용으로 사용할 새로운 소켓이 생성된다.
		// 서버 소켓은 추가적인 클라이언트 요청을 수락하는데 계속 사용된다.
		// 상대방 소켓의 주소를 알 필요없는 경우, addr과 addrlen은 NULL, 0
		client_fd = accept(server_fd, NULL, NULL);
		if(client_fd == -1)
			errExit("accept()");

		// 클라이언트 통신 소켓으로부터 BUF_SIZE만큼 읽어들인 후 루프에 의해 재시도하게 되며
		// 버퍼가 비게되면 버퍼에 데이터가 있을 때까지 블록함.
		// 프로그램을 종료하거나 close를 호출하면 소켓이 닫히고, 이때
		// 상대방이 read를 시도하면 EOF(0)을 받고 루프 탈출
		while((numread = read(client_fd, buf, BUF_SIZE)) > 0)
			// 읽은 데이터를 stdout에 쓴다.
			if(write(STDOUT_FILENO, buf, numread) != numread)
				fatal("failed write");

		if(numread == -1)
			errExit("read()");

		close(client_fd);
	}

	/*	소켓 사용이 완료되면 unlink 함수를 통해 소켓 경로를 제거할 수 있음.*/

}

// 유닉스 도메인 스트림 소켓 클라이언
void socket_unix_domain_stream_client()
{
	struct sockaddr_un addr;
	int socket_fd;
	ssize_t numread;
	char buf[BUF_SIZE];

	// 클라이언트 소켓 생성
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

	// 서버 주소 지정
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

	// 서버 소켓과 연결
	if(connect(socket_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
		errExit("connect");

	// stdout으로부터 데이터를 읽는다.
	while((numread = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
		// 읽은 데이터를 소켓에 쓴다.
		if(write(socket_fd, buf, numread) != numread)
			fatal("failed write");

	if(numread == -1)
		errExit("read()");

}


/*
유닉스 도메인 데이터그램(데이터의 경계가 있는 = UDP 방식) 소켓 서버
유닉스 도메인 데이터그램 소켓인 경우, 네트워크 전송과 다르게 커널 내에서 수행되므로 안정적이다.
모든 메시지는 순서대로 전달되고 중복이 발생하지 않는다.

아래는 양방향 소켓 통신 예제임. 서버 <-> 클라이언트
클라이언트가 서버로부터 메시지를 받기 위해 클라이언트도 소켓에 자신의 주소(고유의 경로)를 bind한다.
*/

void socket_unix_domain_datagram_server()
{
	struct sockaddr_un server_addr, client_addr;
	int server_fd;
	ssize_t numbytes;
	socklen_t len;
	char buf[BUF_SIZE];

	server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(server_fd == -1)
		errExit("socket()");

	if(remove(SOCKET_PATH) == -1 && errno !=ENOENT)
		errExit("remove()");

	// 서버 소켓 주소를 지정함.
	memset(&server_addr, 0, sizeof(struct sockaddr_un));
	server_addr.sun_family = AF_UNIX; // 유닉스 도메인
	strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

	// 서버 소켓과 주소 바인딩
	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1)
		errExit("bind()");

	while(1) {
		len = sizeof(struct sockaddr_un);
		// 서버 소켓으로 들어오는 데이터를 수신한다.
		// 동시에 클라이언트 주소 정보를 별도로 저장한다.
		numbytes = recvfrom(server_fd, buf, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &len);

		if(numbytes == -1)
			errExit("recvfrom()");

		// 클라이언트 주소(경로) 정보 확인
		printf("server recieved. sized = %ld, client address = %s\n", numbytes, client_addr.sun_path);

		// 클라이언트 주소 정보를 이용해 loopback 메시지를 클라이언트에 재전송
		if(sendto(server_fd, buf, numbytes, 0, (struct sockaddr *)&client_addr, len) != numbytes)
			errExit("sendto()");
	}

}

void socket_unix_domain_datagram_client()
{
	struct sockaddr_un server_addr, client_addr;
	int socket_fd;
	ssize_t numbytes;
	char buf[BUF_SIZE];

	socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(socket_fd == -1)
		errExit("socket()");

	// 클라이언트 소켓 주소를 지정함.
	// 주소명에 클라이언트의 pid를 포함해 유일한 주소를 갖게 함.
	memset(&client_addr, 0, sizeof(struct sockaddr_un));
	client_addr.sun_family = AF_UNIX; // 유닉스 도메인
	snprintf(client_addr.sun_path, sizeof(client_addr.sun_path), "%s_client.%d", SOCKET_PATH, getpid());

	// 클라이언트 소켓 주소와 소켓을 바인드
	// 클라이언트도 서버로부터 메시지를 수신하기 위해 자신의 주소(경로)를 소켓에 바인딩한다.
	if(bind(socket_fd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_un)) == -1)
		errExit("bind()");

	// 서버에 메시지를 전달하기 위해 서버 소켓 주소를 지정함.
	memset(&server_addr, 0, sizeof(struct sockaddr_un));
	server_addr.sun_family = AF_UNIX; // 유닉스 도메인
	strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

	while(1) {
		// stdin으로부터 메시지를 읽어들인다.
		if((numbytes = read(STDIN_FILENO, buf, BUF_SIZE)) == -1)
			errExit("read()");

		// stdin으로 읽어들인 만큼 서버에 메시지를 보낸다.
		if(sendto(socket_fd, buf, numbytes, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) != numbytes)
			errExit("sendto()");

		// 서버로부터 loopback 메시지를 받는다.
		numbytes = recvfrom(socket_fd, buf, BUF_SIZE, 0, NULL, NULL);
		if(numbytes == -1)
			errExit("recvfrom()");

		buf[numbytes] = '\0';
		printf("loopback message from server: %s\n", buf);

	}

/*	소켓 사용이 완료되면 unlink 함수를 통해 소켓 경로를 제거할 수 있음.*/
}

/*
socketpair 함수 :
- 하나의 프로세스로 한 쌍의 소켓을 만들어 부모 - 자식을 연결하는 방법
- socket()을 두번 호출하고 bind -> listen, connect, accept 호출하는 과정을 하나의 함수로 구현한다.
- 유닉스 도메인에서만 사용 가능하다.
- socket pair는 파이프와 유사하게 사용된다.
- 수동으로 socket pair를 사용하는 것 대비 특정 주소(경로)와 연결하지 않으므로
  다른 프로세스는 소켓을 볼 수 없으므로 보안상의 취약점이 해결된다.
*/
void socket_pair_sample(int argc, char *argv[])
{
	int sockfd[2];
	ssize_t numread;
	pid_t child_proc;
	char buf[BUF_SIZE];

	// socket pair 생성. 유닉스 도메인에서만 사용 가능하다
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1)
		errExit("socketpair()");

	switch(child_proc = fork()) {
	case -1:
		errExit("fork()");

	// fork 함수 이후 자식 프로세스
	case 0:
		// 자식은 0번 소켓을 사용
		if(close(sockfd[1]) == -1)
			errExit("close() - child");

		while (1) {
			numread = read(sockfd[0], buf, BUF_SIZE);

			if(numread == -1)
				errExit("read() - child");

			// EOF를 만나면 read는 0을 리턴한다.
			if(numread == 0) {
				break; // EOF
			}

			if(write(STDOUT_FILENO, buf, numread) != numread)
				fatal("parent - partial / failed write");
		}

		// EOF를 만나서 루프를 탈출하면 개행 문자 삽입
		write(STDOUT_FILENO, "\n", 1);

		// 파이프의 읽기 endpoint를 닫는다.
		if(close(sockfd[0]) == -1)
			errExit("close");

		break;

	// fork함수 이후 부모 프로세스
	default:
		// 부모는 1번 소켓을 사용
		if(close(sockfd[0]) == -1)
			errExit("close() - parent");

		// 쓰기 수행
		int len = strlen(argv[1]);
		if(write(sockfd[1], argv[1], len) != len)
			fatal("parent - partial / failed write");

/*
		소켓을 닫는다. 이 때 EOF가 기록되며 자식의 read는 EOF를 보게 된다.
		단 EOF를 볼 수 있는 것은 소켓의 style이 SOCK_STREAM인 경우에 한한다.
		만일, stype이 SOCK_DGRAM인 경우, EOF를 받을 수 없다.
		이 경우, 자식의 read함수는 EOF를 받을 수 없으므로 현재 구조에서는 블록 상태를 유지하게 됨.
*/
		if(close(sockfd[1]) == -1)
			errExit("close");

		// 자식의 종료를 기다림.
		wait(NULL);

		break;
	}

	exit(EXIT_SUCCESS);

}

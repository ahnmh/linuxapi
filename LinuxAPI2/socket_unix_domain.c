/*
 * socket_unix_domain.c
 *
 *  Created on: Jun 3, 2016
 *      Author: ahnmh-vw
 */

#include "socket_unix_domain.h"
#define BACKLOG 5

/*
유닉스 도메인 스트림(데이터의 경계가 없는 = TCP 방식) 소켓 서버
소켓 생성에 대해 주의할 점
- 이미 존재하는 경로명으로는 소켓을 생성할 수 없다.
- 절대 경로명으로 소켓을 연결해서 파일 시스템의 고정된 주소(경로)를 할당해야 한다.
- 소켓은 하나의 경로명으로 결속할 수 있다. 반대로 경로명도 오직 하나의 소켓에 결속될 수 있다.
- 소켓이 필요 없는 경우, unlink로 경로명을 제거할 수 있다.
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
	if(remove(SERVER_SOCKET_PATH) == -1 && errno !=ENOENT)
		errExit("remove()");

	// 서버 소켓 주소를 지정함.
	// 유닉스 도메인 소켓 주소를 지정하는 구조체 socaddr_un 에는 소켓 주소에 경로명을 포함한다.
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX; // 유닉스 도메인
	strncpy(addr.sun_path, SERVER_SOCKET_PATH, sizeof(addr.sun_path) - 1); // 마지막 자리는 '\0'으로 남겨둔다.

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
	strncpy(addr.sun_path, SERVER_SOCKET_PATH, sizeof(addr.sun_path) - 1);

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

/*
 * socket_echo_client_example.c
 *
 *  Created on: Jun 11, 2016
 *      Author: ahnmh-vw
 */



#include <netdb.h>
#include "socket_echo.h"

static int connector(const char *host, const char *service)
{
	int cfd = -1;
	struct addrinfo hint, *result, *rp;
	memset(&hint, 0, sizeof(struct addrinfo));
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL; // socaddr* 타입
	hint.ai_next = NULL;
	hint.ai_socktype = SOCK_STREAM; // TCP
	hint.ai_family = AF_UNSPEC; // IPv4, IPv6 모두 가능

	if(getaddrinfo(host, service, &hint, &result) != 0)
		errExit("getaddrinfo()");

	for(rp = result; rp != NULL; rp = rp->ai_next) {
		cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(cfd == -1)
			continue;

		if(connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		// 여기로 넘어오면 연결 실패. 다음 주소로 재시도
		close(cfd);
	}

	freeaddrinfo(result);

	return cfd;
}

void socket_echo_client()
{
	int client_sock;
	ssize_t numread;
	char buf[BUF_SIZE];

	client_sock = connector("localhost", "echo");
	if(client_sock == -1)
		errExit("connector()");

/*
	클라이언트 동작:
	부모 프로세스가 stdin에 쓴 메시지를 읽어서
	자식 프로세스가 서버에 보낸 다음 echo로 다시 받는다.
*/
	switch(fork()) {
	case -1:
		break;

	// 자식은 소켓으로부터 echo 메시지를 읽어들인 후 printf로 출력하는 역할을 수행
	case 0:
		while(1) {
			numread = read(client_sock, buf, BUF_SIZE);
			// error 또는 EOF인 경우 종료
			if(numread <= 0)
				break;

			buf[numread] = '\0';
			printf("read bytes = %ld, message = %s", numread, buf);
		}
		break;

	// 부모는 stdin으로부터 메시지를 읽어서 소켓으로 써주는 역할을 수행
	default:
		while(1) {
			numread = read(STDIN_FILENO, buf, BUF_SIZE);
			// error 또는 EOF인 경우 종료
			if(numread <= 0)
				break;

			if(write(client_sock, buf, numread) != numread)
				errExit("write()");
		}

		// 소켓에 close를 호출하면 양방향 통신 채널을 모두 닫는다.
		// 반면에 shutdown 함수는 특정 소켓의 지정한 채널만 닫는다.
		// SHUT_WR : 쓰기 채널을 닫음. 서버가 모든 가용 데이터를 읽은 상태라면 EOF가 발생함.
		// 쓰기 채널만 닫히기 때문에 계속 읽어올수 있다.
		if(shutdown(client_sock, SHUT_WR) == -1)
			errExit("shutdown()");

		break;
	}

}

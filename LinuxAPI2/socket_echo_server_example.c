/*
 * socket_echo_server.c
 *
 *  Created on: Jun 10, 2016
 *      Author: ahnmh-vw
 */

#define _BSD_SOURCE
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include "socket_echo.h"

#define BACKLOG 5
#define SERVICE_NAME "echo"


/*
echo 서비스는 포트 번호 7번인데 사용하려면 프로그램이 반드시 root 권한으로 실행되어야 함.
관리자 권한으로 열려 있는 모든 소켓 정보와 사용중인 프로세스를 확인해보면,
LinuxAPI2 프로세스가 리스닝 소켓(0.0.0.0), 클라이언트 통신 소켓(127.0.0.1)을 사용중임을 알 수 있음.

$ sudo netstat -pantu
[sudo] password for ahnmh-vw:
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
tcp        0      0 127.0.1.1:53            0.0.0.0:*               LISTEN      3219/dnsmasq
tcp        0      0 127.0.0.1:631           0.0.0.0:*               LISTEN      4157/cupsd
tcp        0      0 0.0.0.0:7               0.0.0.0:*               LISTEN      4934/LinuxAPI2
tcp        0      0 127.0.0.1:7             127.0.0.1:48362         ESTABLISHED 4937/LinuxAPI2
tcp        0      0 127.0.0.1:48362         127.0.0.1:7             ESTABLISHED 4935/LinuxAPI2
tcp6       0      0 ::1:631                 :::*                    LISTEN      4157/cupsd
tcp6       1      0 ::1:39278               ::1:631                 CLOSE_WAIT  2989/cups-browsed
udp        0      0 0.0.0.0:5353            0.0.0.0:*                           2730/avahi-daemon:
udp        0      0 0.0.0.0:9740            0.0.0.0:*                           3219/dnsmasq
udp        0      0 0.0.0.0:32511           0.0.0.0:*                           3219/dnsmasq
udp        0      0 0.0.0.0:48923           0.0.0.0:*                           2730/avahi-daemon:
udp        0      0 127.0.1.1:53            0.0.0.0:*                           3219/dnsmasq
udp        0      0 0.0.0.0:68              0.0.0.0:*                           3089/dhclient
udp        0      0 0.0.0.0:26930           0.0.0.0:*                           3219/dnsmasq
udp        0      0 0.0.0.0:631             0.0.0.0:*                           2989/cups-browsed
udp        0      0 0.0.0.0:11070           0.0.0.0:*                           3219/dnsmasq
udp        0      0 0.0.0.0:4958            0.0.0.0:*                           3089/dhclient
udp        0      0 0.0.0.0:35819           0.0.0.0:*                           3219/dnsmasq
udp6       0      0 :::5353                 :::*                                2730/avahi-daemon:
udp6       0      0 :::42985                :::*                                2730/avahi-daemon:
udp6       0      0 :::33072                :::*                                3089/dhclient
*/


static void signal_handler(int sig)
{
	// WNOHANG: 변경된 자식 프로세스가 없으면 0을 리턴
	// -1 : wait any
	// 따라서 종료된 자식이 있는 경우(waitpid가 pid값을 리턴하므로) 루프를 돌아 다른 자식을 확인하고,
	// 그렇지 않으면 리턴이 0이므로 루프를 탈출함.
	// *process_wait_by_signal.c 참조
	while(waitpid(WAIT_ANY, NULL, WNOHANG) > 0)
		continue;
}

static int listener(char *service, int backlog)
{
	int lfd = -1;
	struct addrinfo hint, *result, *rp;
	memset(&hint, 0, sizeof(struct addrinfo));
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_family = AF_UNSPEC;
	hint.ai_flags = AI_PASSIVE;

	// AI_PASSIVE(wildcard 주소)인 경우 host는 NULL로 지정한다.
	if(getaddrinfo(NULL, service, &hint, &result) != 0)
		errExit("getaddrinfo()");

	// 소켓을 성공적으로 생성하고 bind할 수 있는 주소 구조체가 나올때까지 리턴된 리스트를 검색한다.
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(lfd == -1)
			continue;

		if(bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		else
			errMsg("bind()");

		// 여기로 넘어오면 연결 실패. 다음 주소로 재시도
		close(lfd);
	}

	if(listen(lfd, backlog) == -1)
		errExit("listen()");

	freeaddrinfo(result);

	return lfd;

}

static void echo(int cfd)
{
	ssize_t numread;
	char buf[BUF_SIZE];
	while((numread = read(cfd, buf, BUF_SIZE)) > 0)
		if(write(cfd, buf, numread) != numread)
			errExit("write()");

	if(numread == -1)
		errExit("read()");

}

void socket_echo_server()
{
	int listen_sock, client_sock;
	struct sigaction sa;

/*
	시그널 핸들러 설정
	자식이 종료될 때 좀비가 되지 않도록 시그널 핸들러에서 waitpid 함수를 호출함.
*/
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = signal_handler;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		errExit("sigaction()\n");

	listen_sock = listener(SERVICE_NAME, 5);
	if(listen_sock == -1)
		errExit("listener()");

//	struct sockaddr_in client_addr;
//	socklen_t client_addr_size;
	while(1) {
		// 주소 구조체 받아오는게 왜 안될까....
//		client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_size);
		client_sock = accept(listen_sock, NULL, NULL);
		if(client_sock == -1)
			errExit("accept()");

//		// 클라이언트 정보 출력
//		char host[NI_MAXHOST];
//		char service[NI_MAXSERV];
//		if(getnameinfo((struct sockaddr *)&client_addr, client_addr_size, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
//			printf("client address = %s, port = %s\n", host, service);

		switch(fork()) {
		// 오류
		case -1:
			close(listen_sock);
			errExit("fork()");
			break;
		// 자식 - 각 클라이언트의 요청을 처리한다.
		case 0:
			// 연결 수락을 수행하는 리스닝소켓은 자식에게 불필요하다.(부모가 처리할 것이므로)
			close(listen_sock);
			echo(client_sock);
			break;
		// 부모
		default:
			// 클라이언트 소켓은 부모에게 불필요하다.(자식이 처리할 것이므로)
			close(client_sock);
			break;

		}

	}
}

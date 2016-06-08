/*
 * socket_inet_domain.c
 *
 *  Created on: Jun 7, 2016
 *      Author: ahnmh-vw
 */

#include "socket_inet_domain.h"
#define BACKLOG 5

/*
인터넷 도메인 스트림 소켓 :
TCP 기반. 안정적인 양방향 바이트 스트림 채널을 제공함.
네트워크의 경우 포트 번호와 IP 주소 사용시 big endian을 사용하므로 변환이 필요함.
호스트 바이트 순서를 네트워크 바이트 순서로 변환한 값을 리턴하는 함수:
htons, htonl
네트워크 바이트 순서를 호스트 바이트 순서로 변환한 값을 리턴하는 함수:
ntohs, ntohl

*/
void socket_inet_domain_stream_server()
{
	struct sockaddr_in addr;
	struct sockaddr_in client_addr;
	int server_fd, client_fd;
	ssize_t numread;
	char buf[BUF_SIZE];

	// 인터넷 도메인 스트림 소켓 생성
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd == -1)
		errExit("socket()");

/*
	sockaddr_in : IPv4 소켓 주소를 지정하는 구조체
	INADDR_ANY = 와일드카드 주소(0.0.0.0):
	지정한 아무 주소에나 연결된다는 의미, 서버의 경우 어떤 호스트의 주소로 들어오는 연결이라도 다 받아들인다는 의미
*/
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET; // IPv4 도메인
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // 32bit(long) - htonl - 주소
//	addr.sin_addr.s_addr = inet_addr("0.0.0.0"); 문자열 주소를 사용하여 주소를 지정하는 방법
	addr.sin_port = htons(PORT_NUMBER); //16bit(short) - htons - 포트 번호

	if(bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
		errExit("bind()");

	if(listen(server_fd, BACKLOG) == -1)
		errExit("listen()");

	while(1) {
		socklen_t client_addr_size;
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
		if(client_fd == -1)
			errExit("accept()");

		// IPv4 문자열 주소를 저장하는 공간. INET_ADDRSTRLEN(=16) 크기만큼 정의하면 된다.
		char ch_client_addr[INET_ADDRSTRLEN];
		// in_addr 구조체에 저장된 주소를 문자열 주소로 변환해주는 함수. IPv4, IPv6 모두 사용 가능하다.
		if(inet_ntop(AF_INET, &client_addr.sin_addr, ch_client_addr, INET_ADDRSTRLEN) == NULL)
			fatal("inet_ntop()");
		printf("client address = %s, port = %d\n", ch_client_addr, client_addr.sin_port);
		// inet_ntop 이전에 사용되던 함수. IPv4만 지원한다.
		// printf("client address = %s, port = %d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

		// 연결된 클라이언트 소켓으로부터 데이터를 읽어온다. 실제 데이터가 있을 때까지 블록됨
		while((numread = read(client_fd, buf, BUF_SIZE)) > 0)
			// 읽은 데이터를 stdout에 쓴다.
			if(write(STDOUT_FILENO, buf, numread) != numread)
				fatal("failed write");

		if(numread == -1)
			errExit("read()");

		close(client_fd);
	}
}

void socket_inet_domain_stream_client()
{
	struct sockaddr_in server_addr;
	int socket_fd;
	ssize_t numread;
	char buf[BUF_SIZE];

	// 클라이언트 소켓 생성
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	// 서버 주소 지정
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUMBER);
	// 문자열 주소로부터 in_addr 구조체 주소를 얻는 함수. IPv4, IPv6 모두 가능
	if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
		fatal("inet_pton()");

	// IPv4 전용 함수
	// if(inet_aton("127.0.0.1", &server_addr.sin_addr) <= 0)
	//		fatal("inet_pton()");

	// 서버 소켓과 연결
	if(connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1)
		errExit("connect()");

	// stdout으로부터 데이터를 읽는다.
	while((numread = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
		// 읽은 데이터를 소켓에 쓴다.
		if(write(socket_fd, buf, numread) != numread)
			fatal("write()");

	if(numread == -1)
		errExit("read()");

	close(socket_fd);
}

void socket_inet_domain_datagram_server()
{
	struct sockaddr_in server_addr, client_addr;
	int server_fd;
	ssize_t numbytes;
	socklen_t len;
	char buf[BUF_SIZE];

	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(server_fd == -1)
		errExit("socket()");

	// 서버 소켓 주소를 지정함.
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 32bit(long) - htonl - 주소
	server_addr.sin_port = htons(PORT_NUMBER); //16bit(short) - htons - 포트 번호

	// 서버 소켓과 주소 바인딩
	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1)
		errExit("bind()");

	while(1) {
		len = sizeof(struct sockaddr_in);
		// 서버 소켓으로 들어오는 데이터를 수신한다.
		// 동시에 클라이언트 주소 정보를 별도로 저장한다.
		numbytes = recvfrom(server_fd, buf, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &len);

		if(numbytes == -1)
			errExit("recvfrom()");

		// IPv4 문자열 주소를 저장하는 공간. INET_ADDRSTRLEN(=16) 크기만큼 정의하면 된다.
		char ch_client_addr[INET_ADDRSTRLEN];
		// in_addr 구조체에 저장된 주소를 문자열 주소로 변환해주는 함수. IPv4, IPv6 모두 사용 가능하다.
		if(inet_ntop(AF_INET, &client_addr.sin_addr, ch_client_addr, INET_ADDRSTRLEN) == NULL)
			fatal("inet_ntop()");

		// 클라이언트 주소(경로) 정보 확인
		printf("server recieved. sized = %ld, client address = %s, port = %d\n", numbytes, ch_client_addr, client_addr.sin_port);

		// 클라이언트 주소 정보를 이용해 loopback 메시지를 클라이언트에 재전송
		if(sendto(server_fd, buf, numbytes, 0, (struct sockaddr *)&client_addr, len) != numbytes)
			errExit("sendto()");
	}

}

void socket_inet_domain_datagram_client()
{
	struct sockaddr_in server_addr;
	int socket_fd;
	ssize_t numbytes;
	char buf[BUF_SIZE];

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_fd == -1)
		errExit("socket()");

/*
	인터넷 도메인 소켓 사용시 아래와 같이,
	클라이언트가 bind 함수를 호출하지 않아도 서버는 recvfrom 호출을
	이용해 클라이언트의 소켓 주소와 포트 번호를 얻어올 수 있다.

	// 클라이언트 소켓 주소를 지정함.
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	client_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUMBER);
	// 문자열 주소로부터 in_addr 구조체 주소를 얻는 함수. IPv4, IPv6 모두 가능
	if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
		fatal("inet_pton()");

	// 클라이언트 소켓 주소와 소켓을 바인드
	// 클라이언트도 서버로부터 메시지를 수신하기 위해 자신의 주소(경로)를 소켓에 바인딩한다.
	if(bind(socket_fd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in)) == -1)
		errExit("bind()");
*/

	// 서버에 메시지를 전달하기 위해 서버 소켓 주소를 지정함.
	// 서버 주소 지정
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUMBER);
	// 문자열 주소로부터 in_addr 구조체 주소를 얻는 함수. IPv4, IPv6 모두 가능
	if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
		fatal("inet_pton()");

	// AF_INET이던 AF_UNIX이던 소켓으로부터 읽고 쓰는 부분은 동일하다.
	while(1) {
		// stdin으로부터 메시지를 읽어들인다.
		if((numbytes = read(STDIN_FILENO, buf, BUF_SIZE)) == -1)
			errExit("read()");

		// stdin으로 읽어들인 만큼 서버에 메시지를 보낸다.
		if(sendto(socket_fd, buf, numbytes, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) != numbytes)
			errExit("sendto()");

		// 서버로부터 loopback 메시지를 받는다.
		numbytes = recvfrom(socket_fd, buf, BUF_SIZE, 0, NULL, NULL);
		if(numbytes == -1)
			errExit("recvfrom()");

		buf[numbytes] = '\0';
		printf("loopback message from server: %s\n", buf);

	}
}

void socket_inet_domain_stream_server_v2()
{
	struct sockaddr_in client_addr;
	int server_fd, client_fd;
	ssize_t numread;
	char buf[BUF_SIZE];

/*
	getaddrinfo
	: 호스트 주소("www.naver.com")와 서비스명("http")을 통해 IP주소와 포트 번호를 포함하는 소켓 주소 구조체 리스트를 리턴함.
	: sockaddr구조체를 포함하여 소켓을 생성하고 bind하는데 필요한 정보를 한번에 리턴한다.
	: 프로토콜 독립적이다.(IPv4, IPv6 모두 사용 가능함)
	: addrinfo 구조체의 링크드 리스트를 동적으로 할당하고 result가 리스트의 시작 부분을 가르키도록 설정함.
	: 인자 host, service, hints에 해당하는 호스트와 서비스 조합은 여러 개가 될 수 있으므로 result는 여러 구조체를 포함하는 리스트를 리턴함.
	: 호스트 주소가 숫자 형식의 문자열이 아닌 경우, DNS를 거치므로 오래 걸릴 수 있다.
	: 서비스명은 /etc/services 파일을 참조하여 매핑된 포트 번호를 얻는데 사용한다.
	 (특정 포트에 대해 TCP, UDP가 모두 서비스 가능한 경우.. /etc/services 참조)
	: 파라미터
		1. host 주소 문자열("www.naver.com" 또는 "X.X.X.X")
		2. service 포트 문자열("http", "ftp", 또는 "50000")
		3. hints : addrinfo 리스트 중에서 hints로 지정된 값을 보유하고 있는 아이템을 선택적으로 리턴한다.
					아래의 경우, 리스트 중 ai_sockettype = SOCK_STREAM인 소켓, ai_family = AF_UNSPEC(IPv4, IPv6)모두 가능한 소켓을 의미함.
*/
	struct addrinfo hint, *result, *rp;
	memset(&hint, 0, sizeof(struct addrinfo));
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL; // socaddr* 타입
	hint.ai_next = NULL;
	hint.ai_socktype = SOCK_STREAM; // TCP
	hint.ai_family = AF_UNSPEC; // IPv4, IPv6 모두 가능
	// AI_PASSIVE = getaddrinfo 함수가 wildcard 주소(기다리는 소켓)
	// AI_NUMERICSERV = 포트 번호는 숫자 문자열 형태("50000")로 받음. 아닌 경우, "http", "ftp"와 같이 지정한다.
	hint.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

	// AI_PASSIVE(wildcard 주소)인 경우 host는 NULL로 지정한다.
	if(getaddrinfo(NULL, CH_PORT_NUMBER, &hint, &result) != 0)
		errExit("getaddrinfo()");

	// 소켓을 성공적으로 생성하고 bind할 수 있는 주소 구조체가 나올때까지 리턴된 리스트를 검색한다.
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		server_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(server_fd == -1)
			continue;

		if(bind(server_fd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;

		close(server_fd);
	}

/*	나머지 부분은 기존 코드와 동일함.
 * */

	if(listen(server_fd, BACKLOG) == -1)
		errExit("listen()");

	// getaddrinfo는 addrinfo 구조체를 동적 할당하므로 해제하는 함수 호출이 필요함.
	freeaddrinfo(result);

	while(1) {
		socklen_t client_addr_size;
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
		if(client_fd == -1)
			errExit("accept()");

/*
		getnameinfo
		: getaddrinfo와 반대되는 기능을 수행한다.
		: 소켓 주소 구조체(IP주소와 포트 번호가 들어있음)를 받아서 호스트명("www.naver.com")과 서비스명 문자열("http", "ftp", ...)을 리턴한다.
		: 필요없는 경우 둘 중 하나는 NULL로 지정가능.
		: 소켓 주소 구조체는 accept, recvfrom, getsockname, getpeername 함수를 통해 얻는다.
		: 해석할 수 없는 경우 그냥 숫자 주소("X.X.X.X")와 문자열("50000")을 그대로 리턴한다.
		: flags 파라미터
			NI_NUMERICHOST = 호스트 주소를 구할 때 DNS를 거치므로 오래 걸릴 수 있으므로 숫자 형식의 문자열("X.X.X.X")을 리턴하도록 강제함.
			NI_NUMERICSERV = 숫자 포트 번호를 사용하도록 강제함. 따라서 서비스명을 구하기 위해 /etc/service 파일을 검색하지 않는다. 포트 번호가 서비스명에 대응하지 않는 경우 유용함.
*/
		char host[NI_MAXHOST]; // 클라이언트가 127.0.0.1인 경우 "localhost"가 리턴될 것임.
		char service[NI_MAXSERV];
		if(getnameinfo((struct sockaddr *)&client_addr, client_addr_size, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
			printf("client address = %s, port = %s\n", host, service);

		while((numread = read(client_fd, buf, BUF_SIZE)) > 0)
			if(write(STDOUT_FILENO, buf, numread) != numread)
				fatal("failed write");

		if(numread == -1)
			errExit("read()");

		close(client_fd);
	}
}

void socket_inet_domain_stream_client_v2()
{
	int socket_fd;
	ssize_t numread;
	char buf[BUF_SIZE];

	struct addrinfo hint, *result, *rp;
	memset(&hint, 0, sizeof(struct addrinfo));
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL; // socaddr* 타입
	hint.ai_next = NULL;
	hint.ai_socktype = SOCK_STREAM; // TCP
	hint.ai_family = AF_UNSPEC; // IPv4, IPv6 모두 가능
	hint.ai_flags = AI_NUMERICSERV;

	// getaddrinfo 함수를 사용하여, 주소, 서비스 문자열을 통해 connect 할 수 있는 IP주소와 포트 번호를 얻어온다.
	if(getaddrinfo("localhost", CH_PORT_NUMBER, &hint, &result) != 0)
		errExit("getaddrinfo()");

	// 소켓을 성공적으로 생성하고 connect할 수 있는 주소 구조체가 나올때까지 리턴된 리스트를 검색한다.
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(socket_fd == -1)
			continue;
		// "localhost" -> 127.0.0.1, "50002" -> 50002 의 값으로 rp->ai_addr로 구해지며 해당 값으로 connect 함.
		if(connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		close(socket_fd);
	}

	freeaddrinfo(result);

	// stdout으로부터 데이터를 읽는다.
	while((numread = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
		// 읽은 데이터를 소켓에 쓴다.
		if(write(socket_fd, buf, numread) != numread)
			fatal("write()");

	if(numread == -1)
//		errExit("read()");

	close(socket_fd);
}

// 호스트 이름("www.naver.com")으로 IP를 구하는 gethostbyname 함수 호출 예제
// IP주소를 통해 호스트 이름을 구하는 함수는 gethostbyaddr
void gethostbyname_example(int argc, char *argv[])
{
	struct hostent *h;
	h = gethostbyname(argv[1]); // "www.naver.com"
	if(h == NULL) {
		fprintf(stderr, "gethostbyname(): %s\n", hstrerror(h_errno));
		errExit("gethostbyname()");
	}

	printf("canonical name = %s\n", h->h_name);

	char **pa;
	// alias는 h_aliases[0], h_aliases[1], ..가 각각 가르키는 곳에 저장되어 있고 h_aliases[n]이 NULL일때까지 여러개가 존재할 수 있다.
	for(pa = h->h_aliases; *pa != NULL; pa++)
		printf("alias = %s\n", *pa);

	char **pp;
	char str[INET_ADDRSTRLEN];
	// IP주소는 h_addr_list[0], h_addr_list[1], ..가 각각 가르키는 곳에 저장되어 있고 h_addr_list[n]이 NULL일때까지 여러개가 존재할 수 있다.
	// 포인터 크기는 8byte인 것에 주의할 것.
	for(pp = h->h_addr_list; *pp != NULL; pp++) {
/*
		// inet_ntoa를 사용하는 예제. inet_ntop를 사용하는 것이 낫다.
		struct in_addr *address = (struct in_addr *)*pp;
		printf("address = %s\n", inet_ntoa(*address)); // 호스트의 공식 이름
*/
		// inet_ntop를 통해 문자열로 변환한다.
		printf("address = %s\n", inet_ntop(h->h_addrtype, *pp, str, INET_ADDRSTRLEN)); // 호스트의 공식 이름
	}
}

// 서비스명("http")으로 포트 번호를 구하는 getservbyname 함수 호출 예제
// 포트 번호를 통해 서비스를 구하는 함수는 getservbyport
void getservbyname_example(int argc, char *argv[])
{
	struct servent *s;
	s = getservbyname(argv[1], NULL); // "http"
	if(s == NULL) {
		fprintf(stderr, "gethostbyname(): %s\n", hstrerror(h_errno));
		errExit("gethostbyname()");
	}

	printf("service name = %s\n", s->s_name);

	char **pa;
	for(pa = s->s_aliases; *pa != NULL; pa++)
		printf("alias = %s\n", *pa);

	printf("port number = %d\n", ntohs(s->s_port));
	printf("protocol = %s\n", s->s_proto);
}

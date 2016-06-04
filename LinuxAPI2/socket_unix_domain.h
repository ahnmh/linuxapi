/*
 * socket_unix_domain.h
 *
 *  Created on: Jun 3, 2016
 *      Author: ahnmh-vw
 */

#ifndef SOCKET_UNIX_DOMAIN_H_
#define SOCKET_UNIX_DOMAIN_H_

#include <sys/un.h> // sockaddr_un
#include <sys/socket.h>
#include "tlpi_hdr.h"

// 소켓 경로명을 사용되는 디렉토리는 접근과 쓰기가 가능해야 함.
#define SOCKET_PATH "/tmp/unix_domain_socket"
#define BUF_SIZE 100

void socket_unix_domain_stream_server();
void socket_unix_domain_stream_client();

void socket_unix_domain_datagram_server();
void socket_unix_domain_datagram_client();

void socket_pair_sample(int argc, char *argv[]);

#endif /* SOCKET_UNIX_DOMAIN_H_ */

/*
 * socket_inet_domain.h
 *
 *  Created on: Jun 7, 2016
 *      Author: ahnmh-vw
 */

#ifndef SOCKET_INET_DOMAIN_H_
#define SOCKET_INET_DOMAIN_H_

#define _BSD_SOURCE

#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_pton

#include "tlpi_hdr.h"

#define BUF_SIZE 100
#define PORT_NUMBER 50002

void socket_inet_domain_stream_server();
void socket_inet_domain_stream_client();

void socket_inet_domain_datagram_server();
void socket_inet_domain_datagram_client();

#include <netdb.h> // addrinfo
#define CH_PORT_NUMBER "50002"
void socket_inet_domain_stream_server_v2();
void socket_inet_domain_stream_client_v2();

#endif /* SOCKET_INET_DOMAIN_H_ */

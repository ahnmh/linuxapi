/*
 * socket_echo.h
 *
 *  Created on: Jun 10, 2016
 *      Author: ahnmh-vw
 */

#ifndef SOCKET_ECHO_H_
#define SOCKET_ECHO_H_

#include <sys/socket.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 100

void socket_echo_server();
void socket_echo_client();

#endif /* SOCKET_ECHO_H_ */

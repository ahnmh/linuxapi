/*
 * fileio.h
 *
 *  Created on: Jun 24, 2016
 *      Author: ahnmh-vw
 */

#ifndef FILEIO_H_
#define FILEIO_H_

#include <stdio.h>
#include <fcntl.h> // open, creat
#include <unistd.h> // read
#include <errno.h>
#include <string.h>
#include "helper.h"

#define BUF_SIZE 1000

void fileio(int argc, char *argv[]);

void select_sample();
void pselect_sample();
void poll_sample();
void epoll_sample();

void fileio_stdio_read();
void fileio_stdio_write();
void fileio_stdio_write_lock();

void fileio_readv_writev();

void mmap_sample(int argc, char *argv[]);

void fileio_aio_write();

void get_inode();
void get_physical_block_number();

#endif /* FILEIO_H_ */

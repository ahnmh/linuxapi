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
#include "helper.h"

#define BUF_SIZE 1000

void fileio(int argc, char *argv[]);

void select_sample();
void pselect_sample();
void poll_sample();

void fileio_stdio_read();
void fileio_stdio_write();
void fileio_stdio_write_lock();

#endif /* FILEIO_H_ */

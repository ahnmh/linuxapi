/*
 * file_func.h
 *
 *  Created on: Apr 24, 2016
 *      Author: ahnmh
 */

#ifndef FILE_FUNC_H_
#define FILE_FUNC_H_

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

int file_open_read_write();
int file_fcntl();
int file_scatter_gather();
int file_ftruncate();
int file_opentempfile();
int file_pread_pwrite();

void file_mount();
void file_stat();
int file_buffering();
void file_event();


#endif /* FILE_FUNC_H_ */

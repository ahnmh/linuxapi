/*
 * file_directory.h
 *
 *  Created on: Jul 16, 2016
 *      Author: ahnmh
 */
#ifndef FILE_DIRECTORY_H_
#define FILE_DIRECTORY_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> // opendir, readdir
#include <string.h>
#include <errno.h> // errno
#include "helper.h"

void get_stat(int argc, char *argv[]);
void chn_access(int argc, char *argv[]);
void chn_owner(int argc, char *argv[]);
void working_diretory();
void make_directory();
void traversal_directory(int argc, char *argv[]);
void ioctl_example(int argc, char *argv[]);
void inotify_example(int argc, char *argv[]);

#endif /* FILE_DIRECTORY_H_ */


#ifndef __FILE_IO__
#define __FILE_IO__

#include "tlpi_hdr.h"
#include <fcntl.h>
//#include <sys/stat.h>
#include <sys/uio.h> // struct iovec
#include <sys/syscall.h> // syscall

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif



int file_open_read_write();
int file_fcntl();
int file_scatter_gather();
int file_ftruncate();
int file_opentempfile();
int file_pread_pwrite();


#endif

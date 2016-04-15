/*
 * file_stat.c
 *
 *  Created on: Apr 14, 2016
 *      Author: ahnmh
 */

#include "file_stat.h"
#include "tlpi_hdr.h"
#include <stdio.h>
#include <sys/stat.h> // stat, chmod
#include <fcntl.h> // open
#include <unistd.h> // chown
#include <sys/time.h> // gettimeofday, utimes
#include <linux/fs.h> // FS_IOC_GETFLAGS, FS_IOC_SETFLAGS
#include <sys/ioctl.h> // ioctl


void file_stat()
{
	int fd;
	int ret;
	char *file = "test";

	fd = open(file, O_RDONLY|O_CREAT);
	if(fd == -1)
		errExit("open()");

/*
 * change file time(touch), NULL means current time.
 */

	ret = utimes(file, NULL);

	// change file time with microsecond unit.
	struct timeval tv[2];
	// get current time
	ret = gettimeofday(&tv[0], NULL);
	ret = gettimeofday(&tv[1], NULL);
	// set current time to file access time(stat.st_atime), modified time(stat.st_mtime)
	ret = utimes(file, &tv[2]);


/*
 * Change owner
 */
	// get uid, gid
	struct stat fstat;
	ret = stat(file, &fstat);
	if(ret == -1)
		errExit("stat()");

	// if don't want to change ownership set parameter to -1
	ret = chown(file, fstat.st_uid, fstat.st_gid);
	if(ret == -1)
		errExit("chown()");


/*
 * 	Change file permission
 */
	mode_t mode;
	// chmod u-w, o+r <file>
	mode = (fstat.st_mode | S_IROTH) & ~ S_IWUSR;
	if(chmod(file, mode) == -1)
		errExit("chmod()");

	// rwxr-xr-x
	mode = 0755;
	if(chmod(file, mode) == -1)
		errExit("chmod()");



/*	i-node flag
 * */

	int attr;
	// get i-node flag(lsattr)
	if(ioctl(fd, FS_IOC_GETFLAGS, &attr) == -1)
		errExit("ioctl()");
	// read only flag
	attr+=FS_IMMUTABLE_FL;
	// set i-node flag(chattr)
	if(ioctl(fd, FS_IOC_SETFLAGS, &attr) == -1)
		errExit("ioctl()");


}

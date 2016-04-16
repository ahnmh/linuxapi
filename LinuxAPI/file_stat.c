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

	struct stat filestat;
	ret = stat(file, &filestat); // == terminal command: stat <filename>
	// belows are also possible
	//	ret = fstat(fd, &filestat);

/*
	Device Id
	/dev 위치에서 ls -l 명령을 통해 보면 아래와 같이 Major Id, Minor Id를 확인할 수 있다.
	...
	brw-rw----  1 root disk      8,   1 Apr 14 08:17 sda1
	...
*/
	// i-node number
	printf("i-node number=%d\n", filestat.st_ino);
	// 아래와 같이 파일이 위치한 Device Id는 sda1임을 알 수 있다.
	int maj = major(filestat.st_dev); // 8
	int min = minor(filestat.st_dev); // 1
	printf("major id=%d, minor id=%d\n", maj, min);

	// how to check st_mode field - in this case, it is regular file.
	if(S_ISREG(filestat.st_mode))
		printf("It is regular file.\n");
	if(S_ISDIR(filestat.st_mode))
		printf("It is directory.\n");
	if(S_ISCHR(filestat.st_mode))
		printf("It is character device.\n");
	if(S_ISBLK(filestat.st_mode))
		printf("It is block device.\n");
	if(S_ISFIFO(filestat.st_mode))
		printf("It is FIFO(named pipe).\n");
	if(S_ISLNK(filestat.st_mode))
		printf("It is symbolic link.\n");
	if(S_ISSOCK(filestat.st_mode))
		printf("It is socket.\n");
	if(S_IRUSR & filestat.st_mode)
		printf("it is readable.\n");
	if(S_IWUSR & filestat.st_mode)
		printf("It is writable.\n");
	if(S_IXUSR & filestat.st_mode)
		printf("It is executable.\n");

	printf("file size=%d, file block=%d, Preferred block size for system IO=%d\n",
			filestat.st_size, filestat.st_blocks, filestat.st_blksize);

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

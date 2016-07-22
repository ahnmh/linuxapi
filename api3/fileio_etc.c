/*
 * fileio_etc.c
 *
 *  Created on: Jul 8, 2016
 *      Author: ahnmh-vw
 */

#include "fileio.h"
#include <sys/stat.h> // fstat, S_ISREG
#include <linux/fs.h> // FIBMAP

void get_inode()
{
	int fd;
	fd = open("test", O_RDWR);
	if(fd == -1)
		errexit("open");

	struct stat st;
	if(fstat(fd, &st) == -1)
		errexit("fstat");

	int inode = st.st_ino;
	printf("fd = %d, inode number = %d\n", fd, inode);

	close(fd);
}

#include <sys/ioctl.h>

// 논리 블록 번호로 물리 블록 번호를 구하는 함수
// 반드시 root 권한으로 실행해야 한다.
void get_physical_block_number()
{
	int fd;
	fd = open("test", O_RDWR);
	if(fd == -1)
		errexit("open");

	struct stat st;
	if(fstat(fd, &st) == -1)
		errexit("fstat");

	int logical_block = st.st_blocks;
	int i, j;
	for (i = 0; i < logical_block; i++) {
		j = i;
		if(ioctl(fd, FIBMAP, &j) == -1)
			errexit("ioctl");

		printf("logical = %d, physical = %d\n", i, j);
	}
	close(fd);
}

/*
ahnmh-vw@ubuntu:~/workspace/linuxapi/api3/Debug$ sudo ./api3
[sudo] password for ahnmh-vw:
fd = 3, inode number = 1056853
logical = 0, physical = 1616638
logical = 1, physical = 0
logical = 2, physical = 0
logical = 3, physical = 0
logical = 4, physical = 0
logical = 5, physical = 0
logical = 6, physical = 0
logical = 7, physical = 0

ahnmh-vw@ubuntu:~/workspace/linuxapi/api3/Debug$ stat test
  File: ‘test’
  Size: 1000      	Blocks: 8          IO Block: 4096   regular file
Device: 801h/2049d	Inode: 1056853     Links: 1
Access: (0664/-rw-rw-r--)  Uid: ( 1000/ahnmh-vw)   Gid: ( 1000/ahnmh-vw)
Access: 2016-07-08 14:18:36.676601234 +0900
Modify: 2016-07-07 09:02:31.056144616 +0900
Change: 2016-07-07 09:02:31.056144616 +0900
 Birth: -
*/




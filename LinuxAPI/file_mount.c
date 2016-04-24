/*
 * file_mount.c
 *
 *  Created on: Apr 13, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include <sys/mount.h>
#include <sys/statvfs.h> // statvfs

#include "file_func.h"
#include "tlpi_hdr.h"


void file_mount()
{
	char *source = "/dev/sdc1";
	char *target = "/mnt/sdcard";
	char *ftype = "vfat";

	int ret;
/*
	mount 함수가 fail 나는 경우 :
	1) root 권한으로 실행하지 않는 경우
	2) 마운트 포인트가 현재 사용중인 경우
*/
	ret = mount(source, target, ftype, MS_RDONLY, NULL);
	if(ret == -1)
		errExit("mount()");

	// get mount information
	struct statvfs statfs;
	ret = statvfs(target, &statfs);
	if(ret == -1)
		errExit("statvfs()");


	// umount
	ret = umount(target);
	if(ret == -1)
		errExit("umount()");

}

/*
 * file_event.c
 *
 *  Created on: Apr 18, 2016
 *      Author: ahnmh
 */
#include "tlpi_hdr.h"
#include "file_func.h"
#include <stdio.h>
#include <sys/inotify.h>
#include <fcntl.h>

#define MAX_PATH 256
#define MAX_BUFFER 4096
#define MAX_FILE 2

struct fwatch{
	int watchd;
	char filename[MAX_PATH];
};

static struct fwatch fw[MAX_FILE];



static void fevent_func(struct inotify_event *fevent)
{
	printf("watch descriptor: %d, ", fevent->wd);

	int i;
	for(i = 0; i < MAX_FILE; i++) {
		if(fevent->wd == fw[i].watchd) {
			printf("file name = %s\n", fw[i].filename);

			// Do something for each event
			if(fevent->mask & IN_ACCESS)
				printf("\tevent: IN_ACCESS\n");
			if(fevent->mask & IN_ATTRIB)
				printf("\tevent: IN_ATTRIB\n");
			if(fevent->mask & IN_CLOSE_NOWRITE)
				printf("\tevent: IN_CLOSE_NOWRITE\n"); // 읽기만 하고 닫기
			if(fevent->mask & IN_CLOSE_WRITE)
				printf("\tevent: IN_CLOSE_WRITE\n"); // 쓴 다음 닫기
			if(fevent->mask & IN_CREATE)
				printf("\tevent: IN_CREATE\n");
			if(fevent->mask & IN_DELETE)
				printf("\tevent: IN_DELETE\n");
			if(fevent->mask & IN_DELETE_SELF)
				printf("\tevent: IN_DELETE_SELF\n"); // 해당 파일 삭제
			if(fevent->mask & IN_IGNORED)
				printf("\tevent: IN_IGNORED\n"); // 감시가 제거됨(파일이 삭제 경우)
			if(fevent->mask & IN_ISDIR)
				printf("\tevent: IN_ISDIR\n");
			if(fevent->mask & IN_MODIFY)
				printf("\tevent: IN_MODIFY\n");
			if(fevent->mask & IN_MOVE_SELF)
				printf("\tevent: IN_MOVE_SELF\n");
			if(fevent->mask & IN_MOVED_FROM)
				printf("\tevent: IN_MOVED_FROM\n");
			if(fevent->mask & IN_MOVED_TO)
				printf("\tevent: IN_MOVED_TO\n");
			if(fevent->mask & IN_OPEN)
				printf("\tevent: IN_OPEN\n");
			if(fevent->mask & IN_Q_OVERFLOW)
				printf("\tevent: IN_Q_OVERFLOW\n");
			if(fevent->mask & IN_UNMOUNT)
				printf("\tevent: IN_UNMOUNT\n");


			break;
		}
	}
}

void file_event()
{
	int i;
	int cnt = 2;
	const char *directory = "event";

	strcpy(fw[0].filename, "file1");
	strcpy(fw[1].filename, "file2");

	char nfd_buf[MAX_BUFFER];

	if(chdir(directory) == -1)
		errExit("chdir()");

	int notify_fd = inotify_init();

	// 2개의 파일에 대해 이벤트를 watch한다.
	for (i = 0; i < cnt; ++i) {
		fw[i].watchd = inotify_add_watch(notify_fd, fw[i].filename, IN_ALL_EVENTS);
	}

	int read_cnt = 0;
	struct inotify_event *fevent;

	while(1)
	{
		// 이벤트를 읽는다. 이벤트가 생기면 블록킹이 해제된다.
		if((read_cnt = read(notify_fd, nfd_buf, MAX_BUFFER)) == -1) {
			errMsg("read()");
			break;
		}

		// signaled by event
		char *p;
		for(p = nfd_buf; p < nfd_buf + read_cnt;) {
			fevent = (struct inotify_event*)p;
			fevent_func(fevent);

			p += sizeof(struct inotify_event) + fevent->len;
		}

	}

	for (i = 0; i < cnt; ++i) {
		inotify_rm_watch(notify_fd, fw[i].watchd);
	}

	close(notify_fd);
}

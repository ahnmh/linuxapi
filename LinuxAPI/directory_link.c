/*
 * directory_link.c
 *
 *  Created on: Apr 17, 2016
 *      Author: ahnmh
 */
// 아래 define 은 항상 맨 먼저 나와야 한다.
#define _XOPEN_SOURCE 700 // nftw()와 S_IFSOCK 선언, openat()

#include "directory_link.h"
#include "tlpi_hdr.h"

#include <stdio.h>
#include <unistd.h> // link
#include <fcntl.h> // O_RDONLY | O_CREAT
#include <limits.h> // PATH_MAX
#include <sys/stat.h>
#include <dirent.h> // opendir, readdir, closedir
#include <libgen.h> // dirname, basename

#include <ftw.h>


static int dirfunc(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
	// 파일 모드
	switch(sbuf->st_mode & S_IFMT) {
	case S_IFREG:
		printf("file: ");break;
	case S_IFDIR:
		printf("directory: ");break;
	case S_IFCHR:
		printf("chracter device: ");break;
	case S_IFBLK:
		printf("block device: ");break;
	case S_IFLNK:
		printf("symbolic link: ");break;
	default:
		printf("---");break;
	}

	// inode
	printf("%7ld ", (long)sbuf->st_ino);
	// ftwb->base: 전체 경로 문자열중 현재 파일이나 디렉토리 이름의 시작점에 해당하는 옵셋
	printf("%s\n", &pathname[ftwb->base]);

/*
	0을 리턴하면 다음 파일로 실행을 계속한다.
	0이 아닌 값을 리턴하는 경우 실행을 멈추고, nftw()도 해당 값을 리턴한다.
*/
	return 0;
}

void directory_link()
{

	/*
	 * link
	 */

	int fd;
	char *file = "test";
	fd = open(file, O_RDWR|O_CREAT);
	if(fd == -1)
		errExit("open()");

	// Make hard link
	if(link("test", "test_link") == -1)
		errMsg("link()");

	// Remove hard link
	if(unlink("test_link") == -1)
		errMsg("unlink()");

	// rename
	if(rename("test", "test_new") == -1)
		errMsg("rename()");

	// Make symbolic link
	if(symlink("test_new", "test_slink") == -1)
		errMsg("symlink()");

	// Get link name of symbolic link
	char buf[PATH_MAX];
	int len = 0;
	if((len = readlink("test_slink", buf, PATH_MAX - 1)) == -1)
		errMsg("readlink()");

	// get full path name of symbolic link
	if(realpath("test_slink", buf) == NULL)
		errMsg("readpath()");
	printf("Path of link: %s\n", buf);

	// get current directory
	char *currentdir = NULL;
	if((currentdir = getcwd(currentdir, PATH_MAX)) == NULL)
		errMsg("getcwd()");
	printf("current directory = %s\n", currentdir);
	free(currentdir);

	char *directory = "dtest";
	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

	// mkdir
	if(mkdir(directory, mode) == -1)
		errMsg("mkdir()");

	char cmd[100];
	char *fname1 = "file1";
	char *fname2 = "file2";
	char *fname3 = "file3";
	snprintf(cmd, 100, "touch %s/%s %s/%s %s/%s", directory, fname1, directory, fname2, directory, fname3);
	system(cmd);

	/* Directory scanning
	 *
	 */

	DIR *dir_stream;
	struct dirent *dir_entry;

	dir_stream = opendir(directory);
	if(dir_stream == NULL) {
		errMsg("opendir()");
		return;
	}

	while(1) {
		errno = 0;
		dir_entry = readdir(dir_stream);
		if(dir_entry == NULL)
			break;


		// do not print "." and ".."
		if(strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
			continue;

		printf("[%ld] %s\n", dir_entry->d_ino, dir_entry->d_name);
		// we can get inode info from stat function
	}
	if(closedir(dir_stream) == -1)
		errMsg("closedir()");


/*	nftw()
 *  하위 디렉토리를 돌면서 특정 함수를 실행해주는 함수
 *  자기 자신 포함하여 하위 디렉토리 및 파일까지 모두 순회한다.
 * */

	int flags = 0;
	if(nftw(directory, dirfunc, 10, flags) == -1)
		errMsg("nftw()");

/*
	// rmdir - directory should be empty before call
	if(rmdir("dtest") == -1)
		errMsg("rmdir()");

	// remove: remove file or directory.
	// remove calls unlink for file and rmdir for directory internally.
	if(remove("test_slink") == -1)
		errMsg("remove()");
*/

/*
	// chdir - change current working directory
	if(chdir(directory) == -1)
		errExit("chdir()");
*/

/*	디렉토리 fd를 파라미터로 받는 파일 관련 함수
 *   - 디렉토리에 해당하는 fd를 파라미터로 받아서 해당 fd가 가르키는 경로를 상대 경로의 출발점으로 지정할 수 있다.
 *   - 이렇게 하는 경우, 스레드마다 서로 다른 상대 경로 지점을 가질 수 있다!!!
 *   - 예제의 디렉토리 구조는 아래와 같다.
 *    [dtest]	->	[dir1]
 *    test4			test1
 *					test2
 *	 	 	 	 	test3
 *	 - 이와 같이 디렉토리 fd를 파라미터로 받는 파일 I/O 함수들이 많이 존재한다: openat, faccessat, fchmodat, fchownat, fsatat, ...
 */

	int dirfd = open("dtest", O_RDONLY);
	if(dirfd == -1)
		errExit("open()");

	// 디렉토리 fd는 dtest를 가르키며 따라서 상대 경로는 /dtest가 된다.
	int fd1 = openat(dirfd, "file1", O_RDONLY);
	if(fd1 == -1)
		errExit("openat()");

	// AT_FDCWD는 현재 경로(cwd)를 의미하며 따라서 디렉토리 fd는 '.'를 가르킨다.
	int fd2 = openat(AT_FDCWD, "file4", O_RDONLY);
	if(fd2 == -1)
		errExit("openat()");



/*	파일의 패스와 이름을 파싱하는 함수
 * */
	char *fullpath = "/usr/include/glib-2.0/glib.h";
	char *path, *name;
	// 문자열 복사
	path = strdup(fullpath);
	name = strdup(fullpath);

	path = dirname(path); // 파라미터로 들어가는 문자열은 수정되기 때문에 정적 문자열이 바로 올수 없다.
	name = basename(name); // 파라미터로 들어가는 문자열은 수정되기 때문에 정적 문자열이 바로 올수 없다.
	printf("Path: %s\n", path);
	printf("Name: %s\n", name);

	free(path);
	free(name);

}

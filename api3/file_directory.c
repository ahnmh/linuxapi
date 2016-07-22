/*
 * file_directory.c
 *
 *  Created on: Jul 16, 2016
 *      Author: ahnmh
 */

#include "file_directory.h"
#include <stdlib.h>
#include <fcntl.h>

void get_stat(int argc, char *argv[])
{
	struct stat sb;

	// stat - 파일의 메타데이터를 얻는 함수
	// * 파일의 메타데이터에는 파일의 이름 정보가 없다.
	if(stat(argv[1], &sb) == -1)
		errexit("stat");

	printf("file size = %ld bytes\n", sb.st_size); // 파일 사이즈
	/*
		Device Id
		/dev 위치에서 ls -l 명령을 통해 보면 아래와 같이 Major Id, Minor Id를 확인할 수 있다.
		...
		brw-rw----  1 root disk      8,   1 Apr 14 08:17 sda1
		...
	*/
	int maj = major(sb.st_dev); // 8
	int min = minor(sb.st_dev); // 1
	printf("devic node : major = %d, minor = %d\n", maj, min); // 디바이스 노드
	printf("inode = %ld\n", sb.st_ino);	// inode
	// 파일 타입
	switch(sb.st_mode & S_IFMT) {
	case S_IFBLK:
		printf("block device node\n");
		break;
	case S_IFCHR:
		printf("chracter device node\n");
		break;
	case S_IFDIR:
		printf("directory\n");
		break;
	case S_IFIFO:
		printf("FIFO\n");
		break;
	case S_IFLNK:
		printf("symbolic link\n");
		break;
	case S_IFREG:
		printf("regular file\n");
		break;
	case S_IFSOCK:
		printf("socket\n");
		break;
	}
	// 접근 권환 확인
	(sb.st_mode & S_IRUSR) ? printf("r") : printf("-");
	(sb.st_mode & S_IWUSR) ? printf("w") : printf("-");
	(sb.st_mode & S_IXUSR) ? printf("x") : printf("-");
	(sb.st_mode & S_IRGRP) ? printf("r") : printf("-");
	(sb.st_mode & S_IWGRP) ? printf("w") : printf("-");
	(sb.st_mode & S_IXGRP) ? printf("x") : printf("-");
	(sb.st_mode & S_IROTH) ? printf("r") : printf("-");
	(sb.st_mode & S_IWOTH) ? printf("w") : printf("-");
	(sb.st_mode & S_IXOTH) ? printf("x") : printf("-");
	printf("\n");

	printf("hard link = %ld\n", sb.st_nlink); // 하드링크의 갯수. 무조건 1 이상.
	printf("uid = %d\n", sb.st_uid); // 파일의 소유자 ID
	printf("gid = %d\n", sb.st_gid); // 파일의 그룹 ID
	printf("block size = %ld bytes\n", sb.st_blksize); // 효율적인 파일 입출력을 위한 적절한 블록 크기 = 4096
	printf("block count = %ld\n", sb.st_blocks); // 파일 시스템 블록 수
	printf("access time = %ld\n", sb.st_atim.tv_sec); // 마지막으로 접근한 시간
	printf("modified time = %ld\n", sb.st_mtim.tv_sec); // 마지막으로 수정한 시간
	printf("correct time = %ld\n", sb.st_ctim.tv_sec); // 마지막으로 메타데이터를 변경한 시간
}

void chn_access(int argc, char *argv[])
{
	struct stat sb;
	if(stat(argv[1], &sb) == -1)
		errexit("stat");

	mode_t mode;
	// chmod u-w, o+w <file>
	mode = (sb.st_mode | S_IWOTH) & ~ S_IWUSR;
	if(chmod(argv[1], mode) == -1)
		errexit("chmod");

/*
	// rwxr-xr-x
	mode = 0755;
	if(chmod(argv[1], mode) == -1)
		errexit("chmod");
*/
}

// CAP_CHOWN 권한(root)가 필요함.
void chn_owner(int argc, char *argv[])
{
	struct stat sb;
	if(stat(argv[1], &sb) == -1)
		errexit("stat");

	// owner나 group에 -1을 지정하면 그 값은 설정되지 않는다.
	// group, owner가 0이면 root를 의미함.
	if(chown(argv[1], 0, 0) == -1)
		errexit("chown");

}

void working_diretory()
{
	char *cwd;
	// 현재 디렉토리 확인
	// 버퍼 = NULL, 사이즈 = 0 으로 지정하면 getcwd 함수 내부에서 동적 메모리를 할당한 다음
	// 현재 작업 디렉토리를 저장하고 포인터를 리턴한다.
	if((cwd = getcwd(NULL, 0)) == NULL)
		errexit("getcwd");
	printf("current working directory = %s\n", cwd);
	// 사용후에 반드시 메모리 해제해야 함.
	free(cwd);

	// 현재 디렉토리 변경
	if(chdir("/") == -1)
		errexit("chdir");

	// 작업 디렉토리를 변경했다가 현재 디렉토리로 돌아오는 가장 좋은 방법
	// 현재 디렉토리의 파일 디스크립터를 생성한다.
	int fd;
	fd = open(".", O_RDONLY);
	if(fd == -1)
		errexit("open");

	// 현재 디렉토리 변경
	if(chdir("/") == -1)
		errexit("chdir");

	// fchdir을 통해 생성했던 파일 디스크립터를 통해 원래 디렉토리로 돌아온다.
	if(fchdir(fd) == -1)
		errexit("fchdir");

	// 파일 디스크립터를 닫는다.
	close(fd);
}

void make_directory()
{
	// rwxrwxr-x
	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH;

	// 디렉토리 생성
	if(mkdir("test_dir", mode) == -1)
		errexit("mkdir");

	// 디렉토리 삭제
	// 디렉토리는 반드시 비어 있어야 한다. rm -r 처럼 재귀적으로 삭제할 수 있는 API는 없다.
	// 수동으로 파일시스템에서 DFS를 수행한 후 말단에서부터 모든 파일과 디렉토리를 삭제하면서 올라와야 함.
	if(rmdir("test_dir") == -1)
		errexit("rmdir");

/*
	 참고
	 - 파일 삭제: unlink
	 - 디렉토리 삭제: rmdir
	 - 디렉토리 또는 파일 삭제: remove
*/
}

void traversal_directory(int argc, char *argv[])
{
	int ret = 0;
	// 디렉토리 스트림
	DIR *dir;
	// 디렉토리 엔트리 구조체
	// 디렉토리 스트림이 가르키는 디렉토리의 각 파일을 가르킴.
	struct dirent *entry;

	dir = opendir(".");
	// readdir은 파일이 없을 때나, 함수가 실패할 때 모두 NULL을 리턴하므로,
	// 함수가 실패할 때를 가려내기 위해 errno을 활용한다.
	errno = 0;

	// readdir - 디렉토리 스트림으로부터 디렉토리 엔트리 하나를 읽어온다.
	// readdir이 NULL을 리턴할 때까지 읽는다.
	while((entry = readdir(dir)) != NULL) {
		if(strcmp(entry->d_name, argv[1]) == 0) {
			printf("found: %s\n", argv[1]);
			ret = 1;
			break;
		}
	}

	// errno가 0이 아니고, entry 값이 NULL이면 readdir가 에러를 리턴했다는 의미임.
	if(errno && !entry)
		errexit("readdir");

	if(ret == 0)
		printf("not found: %s\n", argv[1]);

	// 디렉토리 스트림 닫기
	closedir(dir);
}

// CD-ROM을 여는 ioctl 예제 - root 권한 필요 없음
#include <sys/ioctl.h>
#include <linux/cdrom.h>
void ioctl_example(int argc, char *argv[])
{
	int fd;
	// CD-ROM 디바이스(/dev/cdrom)을 읽기 전용으로 연다
	// O_NONBLOCK은 드라이브에 미디어가 없더라도 트레이를 연다는 의미임.
	fd = open(argv[1], O_RDONLY|O_NONBLOCK);
	if(fd == -1)
		errexit("open");

	// /dev/cdrom에 CDROMEJECT 명령을 보냄
	if(ioctl(fd, CDROMEJECT,0) == -1)
		errexit("ioctl");

	if(close(fd) == -1)
		errexit("close");

}

/*
파일 이벤트 모니터링
- inotify인터페이스를 통해 파일이 옮겨지거나 읽히거나 쓰여지거나 삭제되는 등의 이벤트를 모니터링 할 수 있다.
*/
#include <sys/inotify.h>

void event_func(struct inotify_event *event)
{
	printf("watch descriptor: %d, ", event->wd);
	printf("mask = %d\n", event->mask);
	printf("dir = %d\n", event->mask & IN_ISDIR); // 이벤트가 발생한 대상이 디렉토리인가?

	// 이벤트에 이름이 있는 경우 출력한다.
	// inotify 인스턴스가 디렉토리를 감시하는 경우, 해당 감시 대상 디렉토리에서 이벤트가 발생한 대상의 파일이름이 저장된다.
	// 감시 대상이 일반 파일인 경우 len 값은 0임.(이 경우 event->name이 NULL이라도 inotify_add_watch에서 넘긴 파일의 이름으로 알 수 있다)
	if(event->len)
		printf("file name = %s\n", event->name);

	// Do something for each event
	if(event->mask & IN_ACCESS) // 읽기
		printf("\tevent: IN_ACCESS\n");
	if(event->mask & IN_ATTRIB) // 메타데이터 변경(소유자, 권한, 확장 속성)
		printf("\tevent: IN_ATTRIB\n");
	if(event->mask & IN_CLOSE_NOWRITE) // 읽기만 하고 닫기
		printf("\tevent: IN_CLOSE_NOWRITE\n");
	if(event->mask & IN_CLOSE_WRITE) // 쓰기 이후 닫기
		printf("\tevent: IN_CLOSE_WRITE\n");
	if(event->mask & IN_CREATE) // 감시 중인 디렉토리에 파일이 생성됨
		printf("\tevent: IN_CREATE\n");
	if(event->mask & IN_DELETE) // 파일이 감시중인 디렉토리에서 삭제됨
		printf("\tevent: IN_DELETE\n");
	if(event->mask & IN_DELETE_SELF) // 감시 대상 객체 자체가 삭제됨
		printf("\tevent: IN_DELETE_SELF\n");
	if(event->mask & IN_IGNORED) // 감시가 제거됨(감시 중인 파일이 지워져도 발생함)
		printf("\tevent: IN_IGNORED\n");
	if(event->mask & IN_ISDIR) // 영향을 받는 객체가 디렉토리임을 나타냄.
		printf("\tevent: IN_ISDIR\n");
	if(event->mask & IN_MODIFY) // 쓰기
		printf("\tevent: IN_MODIFY\n");
	if(event->mask & IN_MOVE_SELF) // 감시 대상 객체가 이동함
		printf("\tevent: IN_MOVE_SELF\n");
	if(event->mask & IN_MOVED_FROM) // 파일이 감시 중인 디렉토리에서 이동했다.
		printf("\tevent: IN_MOVED_FROM\n");
	if(event->mask & IN_MOVED_TO) // 파일이 감시 중인 디렉토리로 이동해 왔다.
		printf("\tevent: IN_MOVED_TO\n");
	if(event->mask & IN_OPEN) // 파일이 열림
		printf("\tevent: IN_OPEN\n");
	if(event->mask & IN_Q_OVERFLOW) // inotify 큐에 오버플로우가 발생함.
		printf("\tevent: IN_Q_OVERFLOW\n");
	if(event->mask & IN_UNMOUNT) // 감시 대상 객체가 들어있는 디바이스가 마운트 해제되었다. 감시가 해제되어 IN_IGNORED 이벤트가 발생된다.
		printf("\tevent: IN_UNMOUNT\n");
}

void inotify_example(int argc, char *argv[])
{
	int fd;
	// inotify_init1 - inotify를 초기화하고 초기화된 인스턴스를 나타내는 파일 디스크립터를 리턴함.
	// 적용 가능한 플래그
	// - IN_CLOEXEC: 생성된 감시 디스크립터는 exec 계열 함수 호출시 자동으로 닫힘.
	// - IN_NONBLOCK: 생성된 감시 디스크립터는 논블록킹 타입임.
	fd = inotify_init1(0);
	if(fd == -1)
		errexit("inotify_init1");

	// 새로운 감시 등록 - 모든 이벤트에 대해 감시
	// 가능한 감시는 event_func 함수 참조
	int wd; // watch descriptor
	wd = inotify_add_watch(fd, argv[1], IN_ALL_EVENTS);
	if(wd == -1)
		errexit("inotify_add_watch");

	// inotify 이벤트 읽기 - inotify 디스크립터를 읽으면 된다.
	// 이벤트가 발생하면 inotify_event 구조체 데이터가 inotify 디스크립터가 가르키는 파일에 저장됨.
	char buf[BUFSIZ];
	ssize_t len, i = 0;

	struct inotify_event *event;

	// 이벤트를 지속적으로 감시하고자 하는 경우 아래 부분을 루프로 싸야함.

	// 저장된 inotify_event들의 전체 길이
	if((len = read(fd, buf, BUFSIZ)) == -1)
		errexit("read");

	// 저장된 이벤트를 모두 확인한다.
	while(i < len) {
		event = (struct inotify_event *)&buf[i];
		event_func(event);

		// 다음 이벤트 읽기
		// inotify_event 구조체의 마지막 필드는 name이고 길이가 0인 배열이다.(name[0])
		// 파일 이름이 들어가는 name 필드는 바로 위의 len 필드가 그 길이값을 가진다.
		// 따라서 가변 크기의 구조체이므로 다음 inotify_event의 시작 위치를 아래와 같이 계산한다.
		i += sizeof(struct inotify_event) + event->len;
	}

	// 이벤트 감시 삭제
	if(inotify_rm_watch(fd, wd) == -1)
		errexit("inotify_rm_watch");

	// inotify 디스크립터 삭제
	close(fd);
}

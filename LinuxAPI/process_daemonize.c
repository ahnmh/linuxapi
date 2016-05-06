/*
 * process_daemonize.c
 *
 *  Created on: May 6, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include <stdarg.h> // 가변 파라미터 함수
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "process_daemonize.h"
#include "tlpi_hdr.h"


static int become_daemon(int flags)
{
	// 프로세스 리더인 부모를 종료시키고 자식은 init로 편입되어 백그라운드 프로세스가 된다.
	// 부모로부터 프로세스 그룹을 분리
	switch(fork()) {
	case -1:
		return -1;
	// 자식인 경우 계속 진행
	case 0:
		break;
	// 부모는 종료
	default:
		printf("[parent] pid = %d, pgid = %d, sid = %d\n", getpid(), getpgid(0), getsid(0));
		printf("parent pid = %d is terminated\n", getpid());
		_exit(EXIT_SUCCESS);
	}

	printf("[child] pid = %d, pgid = %d, sid = %d\n", getpid(), getpgid(0), getsid(0));

	// 자식 프로세스를 새로운 세션의 리더가 되어 부모의 제어 터미널과의 연관성을 제거한다.
	// 부모로부터 세션 분리
	if(setsid() == -1)
		return -1;

	// 손자를 생성하고 자식을 종료시켜서 손자는 세션 리더가 아니게 됨.
	// 따라서 손자는 절대 제어 터미널을 재획득할 수 없게 됨.
	switch(fork()) {
	case -1:
		return -1;
	// 손자인 경우 계속 진행
	case 0:
		break;
	// 자식은 종료
	default:
		printf("child pid = %d is terminated\n", getpid());
		_exit(EXIT_SUCCESS);
	}

	printf("[grandchild] pid = %d, pgid = %d, sid = %d\n", getpid(), getpgid(0), getsid(0));

	// 손자의 mask를 지움으로써 파일과 디렉토리를 자유롭게 생성할 권한을 가진다.
	if(!(flags & BD_NO_UMASK0))
		umask(0);

	// CWD를 루트 디렉토리로 변경
	if(!(flags & BD_NO_CHDIR))
		chdir("/");

	// 모든 열린 파일을 종료
	int fd, maxfds;
	if(!(flags & BD_NO_CLOSE_FILES)) {
		maxfds = sysconf(_SC_OPEN_MAX); // 프로세스의 최대 FD 한도값
		if(maxfds == -1) // 얻을 수 없는 경우
			maxfds = BD_MAX_CLOSE;

		for (fd = 0; fd < maxfds; ++fd)
			close(fd);
	}

	// 표준 입출력, 에러를 /dev/null로 리다이렉션한다( 0>/dev/null, 1>/dev/null 2>/dev/null)
	if(!(flags & BD_NO_REOPEN_STD_FDS)) {
		// 파일 디스크립터 0을 얻기 위해 표준입력(=0)을 닫는다.
		close(STDIN_FILENO);
		// /dev/null을 열고 파일 디스크립터를 얻는다. 위에서 표준입력(0)이 닫혔기 때문에 fd는 0이어야 한다.
		fd = open("/dev/null", O_RDWR);

		// 0번 디스크립터를 얻지 못하면 종료
		if(fd != STDIN_FILENO)
			return -1;
		// 이제, STDIN_FILENO는 /dev/null이다.

		// STDOUT_FILENO(=1)도 /dev/null로 리다이렉션한다.
		if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
			return -1;
		// STDERR_FILENO(=2)도 /dev/null로 리다이렉션한다.
		if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
			return -1;
	}

//	// 데몬의 정보 확인을 위해 만들어진 데몬이 60초 동안 생존해 있도록 한다.
//	sleep(60);

	return 0;
}


/* 테스트 내용
$ ./LinuxAPI
[parent] pid = 3534, pgid = 3534, sid = 3359
parent pid = 3534 is terminated
[child] pid = 3535, pgid = 3534, sid = 3359
child pid = 3535 is terminated
[grandchild] pid = 3536, pgid = 3535, sid = 3535

>>> 데몬의 정보
$ ps -C LinuxAPI -o "pid ppid pgid sid tty command"
  PID  PPID  PGID   SID TT       COMMAND
 3536  1188  3535  3535 ?        ./LinuxAPI

>>> 데몬 부모의 PID
$ ps aux
...
ahnmh     1188  0.0  0.1  41548  4248 ?        Ss   08:54   0:00 init --user
...

>>> 데몬과 init의 관계
$ pstree
ahnmh@ahnmh-amor2:~/sample/LinuxAPI/Debug$ pstree
init─┬─ModemManager───2*[{ModemManager}]
     ├─NetworkManager─┬─dhclient
     │                └─3*[{NetworkManager}]
     ├─accounts-daemon───2*[{accounts-daemon}]
     ├─acpid
     ├─avahi-daemon───avahi-daemon
     ├─bluetoothd
     ├─colord───2*[{colord}]
     ├─cron
     ├─cups-browsed
     ├─cupsd───dbus
     ├─dbus-daemon
     ├─6*[getty]
     ├─irqbalance
     ├─kerneloops
     ├─lightdm─┬─Xorg───{Xorg}
     │         ├─lightdm─┬─init─┬─LinuxAPI
     │         │         │      ├─at-spi-bus-laun─┬─dbus-daemon
     │         │         │      │                 └─3*[{at-spi-bus-laun}]

*/

// become_daemon 함수로 실제 데몬 프로그램 제작
// log를 남기고 SIGHUP 시그널이 수신되면 로그 파일을 새로 연다.
static const char *LOG_FILE = "/tmp/LinuxAPI_deamon.log";
static volatile sig_atomic_t hup_signaled = 0;

static int fd;

static void hup_signal_handler(int sig)
{
	hup_signaled = 1;
}

static int log_open(const char *log)
{
	int openflags;
	mode_t fileperms;

	// 로그 파일 없으면 생성, 이어쓰기
	openflags = O_CREAT | O_RDWR | O_APPEND;
	// rw-rw-rw-
	fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	fd = open(log, openflags, fileperms);
	if (fd == -1)
		errExit("open()");
//		return -1;

	char *buf = "log open\n";
	if (write(fd, buf, strlen(buf)) == -1)
		return -1;

	return 0;
}

// 로그 쓰기 - 가변 파라미터 함수
static int log_message(const char *msgform, ...)
{
	if(fd == -1)
		return 0;

	char buf[4096] = {0};

	va_list vl;
	va_start(vl, msgform);
	vsprintf(buf, msgform, vl);
	va_end(vl);

	size_t wlen = (size_t)strlen(msgform);
	if (write(fd, buf, wlen) == -1)
		return -1;

	if (write(fd, "\n", strlen("\n")) == -1)
		return -1;

	return 0;
}

static int log_close()
{
	if(fd == -1)
		return 0;

	char *buf = "log close\n";
	if (write(fd, buf, strlen(buf)) == -1)
		return -1;

	close(fd);
	return 0;
}

// 데몬으로 백그라운드에서 5초에 한번씩 로그를 남긴다.
// SIGHUP 시그널(pkill -SIGHUP LinuxAPI를 받으면, 로그를 닫고 새로 기록을 시작한다.
// SIGHUP 시그널을 보내기 전에, $ mv <로그> <로그>.old와 같이 로그 파일을 이동하면,
// fd는 여전히 <로그>.old를 가르키므로 <로그>.old를 기록하고 있다가, SIGHUP이후에 <로그>에 다시 기록을 시작함.
void process_daemonize()
{
	const int SLEEP_TIME = 5;
	int count = 0;
	int unslept;

	// SIGHUP 시그널 핸들러 등록
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART; // sleep으로 인한 블록 상태에서 시그널 수신 후 계속 실행
	sa.sa_handler = hup_signal_handler;
	if(sigaction(SIGHUP, &sa, NULL) == -1)
		errExit("sigaction()\n");

	if(become_daemon(0) == -1)
		errExit("become_daemon()");

	// 로그파일 오픈
	log_open(LOG_FILE);

	unslept = SLEEP_TIME;

	while(1) {
		// 시그널에 의해 인터럽트되면 0보다 큰 값(남은 수면 시간)을 리턴한다.
		unslept = sleep(unslept);

		// SIGHUP 시그널이 전달되면 로그를 닫고 새로 시작함.
		if(hup_signaled) {
			log_close();
			log_open(LOG_FILE);
			hup_signaled = 0;
		}

		// 5초에 한번씩 로그를 남긴다.
		if(unslept == 0) {
			count++;
			log_message("count = %d", count);
			unslept = SLEEP_TIME;
		}
	}

}


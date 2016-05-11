/*
 * user_setuserid.c
 *
 *  Created on: May 11, 2016
 *      Author: ahnmh
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "user_func.h"
#include "tlpi_hdr.h"
/*
그냥 실행하면,
$ ./LinuxAPI
real id = 1000
effective id = 1000
real group = 1000
effective group = 1000

sudo 모드로 실행하면,
$ sudo ./LinuxAPI
real id = 0
effective id = 0
real group = 0
effective group = 0

set-user-ID 프로그램으로 실행하면
$ sudo chown root:root ./LinuxAPI
$ sudo chmod u+s LinuxAPI
$ ./LinuxAPI
real id = 1000
effective id = 0 --> 유효 사용자 ID만 바뀐다.
real group = 1000
effective group = 1000

*/
void user_getuserid()
{
//	printf("real id = %d\n", getuid());
//	printf("effective id = %d\n", geteuid());
//	printf("real group = %d\n", getgid());
//	printf("effective group = %d\n", getegid());

	int ruid, euid, suid;
	getresuid(&ruid, &euid, &suid);
	printf("real id = %d\n", ruid); // 1000
	printf("effective id = %d\n", euid); // 0
	printf("saved set-user-ID = %d\n", suid); // 0

}

/*
아래와 같이 set-user-ID 프로그램 상태인 경우에만 effective id를 0 -> 1000, 1000 -> 0으로 변경할 수 있다.
$ sudo chown root:root ./LinuxAPI
$ sudo chmod u+s LinuxAPI
*/

void user_setuserid()
{
	int euid = geteuid();

//	// 특권 포기: effective id가 real id로 설정됨(0 -> 1000)
//	if(seteuid(getuid()) == -1)
//		errExit("seteuid");
//	printf("effective id after change = %d\n", geteuid());
//
//	// 특권 회복: 저장해두었던 effective id를 재설정(1000 ->0)
//	if(seteuid(euid) == -1)
//		errExit("seteuid");
//	printf("effective id after recover = %d\n", geteuid());


	// 특권 포기: effective id가 real id로 설정됨(0 -> 1000)
	if(setresuid(-1, getuid(), -1) == -1)
		errExit("seteuid");
	printf("effective id after change = %d\n", geteuid());

	// 특권 회복: 저장해두었던 effective id를 재설정(1000 ->0)
	if(setresuid(-1, euid, -1) == -1)
		errExit("seteuid");
	printf("effective id after recover = %d\n", geteuid());

	// 특권을 영원히 제거
	if(setresuid(getuid(), getuid(), -1) == -1)
		errExit("seteuid");
	printf("effective id after remove = %d\n", geteuid());

}

/*
$ sudo chown root:root ./LinuxAPI
$ sudo chmod u+s LinuxAPI
$ ./LinuxAPI
real id = 1000
effective id = 0
real group = 1000
effective group = 1000
effective id after change = 1000
effective id after recover = 0
effective id after remove = 1000
*/


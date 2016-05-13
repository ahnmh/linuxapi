/*
 * capability.c
 *
 *  Created on: May 12, 2016
 *      Author: ahnmh
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE

#include <stdio.h>

/*
libcap-dev 라이브러리가 반드시 설치되어야 한.
$ pkg-config --libs libcap
-L/lib64 -lcap
*/
#include <sys/capability.h>

//#include <unistd.h>
#include <shadow.h>
#include "capability_test.h"
#include "tlpi_hdr.h"

/*
프로그램은 실행 전에 반드시 아래와 같이
setcap 명령에 의해 cap_dac_read_search 능력에 대해 permitted 상태가 되어야 한다.
$ sudo setcap "cap_dac_read_search=p" ./LinuxAPI
$ getcap ./LinuxAPI
./LinuxAPI = cap_dac_read_search+p

이후 아래 코드에서처럼 해당 능력에 대해 effective 상태를 부여함으로써
root의 권한이 아님에도 불구하고 root 권한으로 만든 파일에 접근이 가능하다.

CAP_DAC_READ_SEARCH :
이 능력을 얻으면 파일 읽기 권한 검사와 디렉토리 읽기와 실행 권한 검사를 생략


*/
static int get_cap(int capability, int setting)
{
	// get capabilities
	cap_t caps;
	caps = cap_get_proc();
	if(caps == NULL)
		return -1;

	cap_value_t capList[1];
	capList[0] = capability;
	// 현재 capability 값에 유효 권한으로서 CAP_DAC_READ_SEARCH을 Set한다.
	// 변경할 capability는 capList에 들어있고 1개이다.
	if(cap_set_flag(caps, CAP_EFFECTIVE, 1, capList, setting) == -1) {
		cap_free(caps);
		return -1;
	}

	// cap_set_proc으로 effective 상태로 권한을 설정해야 한다,.
	// (permitted 상태만으로는 접근이 불가)
	if(cap_set_proc(caps) == -1) {
		cap_free(caps);
		return -1;
	}

	cap_free(caps);

	return 0;
}

static int del_cap()
{
	cap_t empty;
	int s;

	empty = cap_init();
	if(empty == NULL)
		return -1;

	s = cap_set_proc(empty);
	if(cap_free(empty) == -1)
		return -1;

	return s;
}

void set_capability()
{

	if(get_cap(CAP_DAC_READ_SEARCH, CAP_SET) == -1)
		errExit("get_cap()");
/*
	 getspnam 함수는 shadow 파일의 정보를 읽어오는데 아래와 같이 root 권한으로 실행되어야만 한다.
	 비특권으로 실행해도 능력을 부여받으면 접근이 가능함.
	 ahnmh@ahnmh-amor2:/etc$ ll passwd shadow
	 -rw-r--r-- 1 root root   1867 Mar 10 20:36 passwd
	 -rw-r----- 1 root shadow 1071 Mar 10 20:36 shadow
*/
	struct spwd *shadow;
	shadow = getspnam("ahnmh");
	if(shadow == NULL)
		errExit("getspnam()");

	printf("name = %s\n", shadow->sp_namp); // 로그인 이름
	printf("password = %s\n", shadow->sp_pwdp); // encrypted password

	// 사용 후 능력 해제 - 바로 적용 안됨..
	del_cap();
//	if(get_cap(CAP_DAC_READ_SEARCH, CAP_CLEAR) == -1)
//		errExit("get_cap()");

}

/*
setcap 명령을 사용할 때,
능력에 대해 permitted 상태와 동시에 effective 상태를 부여하려면 아래와 같이 사용한다,
$ sudo setcap "cap_dac_read_search=pe" ./LinuxAPI
*/





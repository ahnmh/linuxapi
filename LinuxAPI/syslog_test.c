/*
 * syslog_test.c
 *
 *  Created on: May 11, 2016
 *      Author: ahnmh
 */

#include <syslog.h>
#include "syslog_test.h"


void syslog_test()
{
/*
	LOG_PID : 로그에 pid를 기록
	LOG_CONS : 시스템 로거로 전달되는 에러가 있는 경우 시스템 콘솔(/dev/console)로 메시지를 쓴다.
	LOG_USER : 사용자 프로세스가 생성한 메시지(기본 설정)
*/
	openlog("LinuxAPI", LOG_PID|LOG_CONS, LOG_USER);

/*	LOG_ERR 이상만 기록하도록 마스크를 적용한다.*/
	setlogmask(LOG_UPTO(LOG_ERR));

	int error = -1;
/*
	LOG_LEVEL 값 :
	LOG_EMERG
	LOG_ALERT
	LOG_CRIT
	LOG_ERR
	LOG_WARNING
	LOG_NOTICE
	LOG_INFO
	LOG_DEBUG
*/
	syslog(LOG_USER|LOG_INFO, "test information");
	// facility값을 명시하지 않으면 openlog에서 지정한 값으로 설정됨.(LOG_USER)
	syslog(LOG_ERR, "test error = %d", error);



	closelog();
}


/*
/var/log/syslog에 아래와 같이 기록됨
May 11 06:08:56 ahnmh-samsung LinuxAPI[2876]: test error = -1
May 11 06:08:56 ahnmh-samsung LinuxAPI[2876]: test information
*/

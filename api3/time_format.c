/*
 * time_timer.c
 *
 *  Created on: Jul 26, 2016
 *      Author: ahnmh-vw
 */

#include "time_timer.h"

void get_time()
{
	// time_t: long int 타입(시간을 나타내는 타입 중 가장 단순)
	time_t t;

	// 기원 시간(1970년 1월 1일(목) 09시) 이후 경과한 s를 기록하는 함수
	if(time(&t) == -1)
		errexit("time");
	printf("sec = %ld\n", t);

	// timeval: s와 us단위까지 표시할 수 있는 구조체
	struct timeval tv;
	if(gettimeofday(&tv, NULL) == -1) // tz 파라미터는 리눅스의 경우 항상 NULL
		errexit("gettimeofday");
	printf("sec = %ld\n", tv.tv_sec);
	printf("usec = %ld\n", tv.tv_usec);

	// timespec: s와 ns단위의 정밀도를 제공하는 구조체
/*
	표준 시계의 종류
	- CLOCK_REALTIME: 시스템 전역에서 사용하는 실시간 시계.
	- CLOCK_MONOTONIC: 시스템 시동과 같이 불특정 시작 시점부터 경과한 시간
	- CLOCK_PROCESS_CPUTIME_ID: 프로세서 수준에서 지원되는 각 프로세스용 고해상도 시계
	- CLOCK_THREAD_CPUTIME_ID: 프로세스 스레드마다 지원되는 각 스레드용 시계
*/
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == -1)
		errexit("clock_gettime");
	printf("sec = %ld\n", ts.tv_sec);
	printf("nsec = %ld\n", ts.tv_nsec);

	// 부팅 이후 경과한 시간
	if(clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
		errexit("clock_gettime");
	printf("sec = %ld\n", ts.tv_sec);
	printf("nsec = %ld\n", ts.tv_nsec);
}

void set_time()
{
	time_t t = 1;
	// 기원 시간 + 1초로 현재 시간 재설정
	if(stime(&t) == -1)
		errexit("stime");

	// settimeofday

	// clock_settime

}

#include <string.h>
void time_format()
{
	// time_t를 구할 때 time(&t) 대신 이렇게 쓸수도 있다.
	time_t t = time(NULL);

	// time_t를 ASCII로 변
	char *str_ctime = ctime(&t);
	printf("%s", str_ctime);

	// time_t를 tm으로 변환
	struct tm *tm_utc, *tm_local;
	tm_utc = gmtime(&t);		// GMT
	tm_local = localtime(&t);	// 로컬 시간대(KST)
	printf("year=%d, month=%d, day=%d, hour=%d, minute=%d, second=%d\n",
			tm_utc->tm_year + 1900,	// -1900한 값을 구함.
			tm_utc->tm_mon, 		// 0 ~ 11
			tm_utc->tm_mday,		// 1 ~ 31
			tm_utc->tm_hour,		// 0 ~ 23
			tm_utc->tm_min,			// 0 ~ 59
			tm_utc->tm_sec			// 0 ~ 60(60: 윤초인 경우에만)
			);
	printf("year=%d, month=%d, day=%d, hour=%d, minute=%d, second=%d\n",
			tm_local->tm_year + 1900,
			tm_local->tm_mon,
			tm_local->tm_mday,
			tm_local->tm_hour,
			tm_local->tm_min,
			tm_local->tm_sec
			);

	// tm을 ASCII로 변환
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));
	// Sun Aug 26 15:00:00 3916
	tm.tm_year = 2016 - 1900; // 2016년
	tm.tm_mon = 7; // 8월
	tm.tm_mday = 26;
	tm.tm_hour = 15;
	tm.tm_min = 0;
	tm.tm_sec = 0;

	char *chtime;
	chtime = asctime(&tm);
	printf("%s", chtime);

	// tm을 time_t로 변환
	time_t tn = mktime(&tm);
	printf("tm to time_t = %ld\n", tn);

	// 스레드 세이프한 시간 변환 함수
	// 각 함수 끝에 '_r'이 들어간 함수를 사용하면 된다.
}

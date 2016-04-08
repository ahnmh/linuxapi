/*
 * time_func.c
 *
 *  Created on: Apr 7, 2016
 *      Author: root
 */

#include "time_func.h"
#include <stdio.h>


void time_related_func()
{
	time_t ti = time(NULL);

	// it's more correct than time because it returns usec by field of timeval.
	struct timeval tv;
	int result = gettimeofday(&tv, NULL);

	char *chtime = ctime(&ti);
	printf("%s", chtime);

	struct tm *tm_utc, *tm_local;
	tm_utc = gmtime(&ti);		// GMT
	tm_local = localtime(&ti);	// KST

	char *chasctime_utc = asctime(tm_utc);

	// customize time-format
	static char buf[1000];
	size_t s = strftime(buf, 1000, "%H:%M:%S", tm_local);
	printf("%s\n", buf);


/*	get current timezone*/
	tzset();
	printf("%s\n", tzname[0]); // KST
	printf("%s\n", tzname[1]); // KST


/*
 * 	 change time zone to TZ
	 enviornment variable TN should be declared.
	 This value is one of /usr/share/zoneinfo
*/
	int ret = setenv("TZ", ":Pacific/Auckland", 1); // overwrite

	struct tm *tm_local2;
	time_t ti2 = time(NULL);
	tm_local2 = localtime(&ti2);	// NZST

	s = strftime(buf, 1000, "%c", tm_local2);
	printf("%s\n", buf);

	ret = unsetenv("TZ");


	// set locale - set current locale if not null, locale is modified accoring to the parameter.
	char *cur_locale, *set_locale;
	if (setlocale(LC_ALL, "") == NULL) // for all of the locale
		errExit("setlocale()");
	else
		// get current locale
		cur_locale = setlocale(LC_ALL, NULL); // en_US.UTF-8

	// does not work....
	set_locale = setlocale(LC_ALL, "de_DE");


	// get process's consumed CPU time(user + kernel)
	clock_t contime = clock();
	long contime_s = contime / CLOCKS_PER_SEC;


}

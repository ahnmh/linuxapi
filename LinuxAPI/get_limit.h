/*
 * get_limit.h
 *
 *  Created on: Apr 11, 2016
 *      Author: ahnmh
 */

#ifndef GET_LIMIT_H_
#define GET_LIMIT_H_

//#include <limits.h>
#include <unistd.h> // sysconf, pathconf
#include <sys/utsname.h> // uname

void get_limit();

#endif /* GET_LIMIT_H_ */

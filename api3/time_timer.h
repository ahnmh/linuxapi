/*
 * time_timer.h
 *
 *  Created on: Jul 26, 2016
 *      Author: ahnmh-vw
 */

#ifndef TIME_TIMER_H_
#define TIME_TIMER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // sleep

#include <time.h>
#include <sys/time.h> // gettimeofday

#include "helper.h"

void get_time();
void set_time();
void time_format();

#endif /* TIME_TIMER_H_ */

/*
 * timer_sleep.c
 *
 *  Created on: Apr 24, 2016
 *      Author: ahnmh
 */
#define _POSIX_C_SOURCE 199309 // nanosleep

#include "timer_func.h"
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "tlpi_hdr.h"

void signal_handler(int sig)
{
	printf("Interrupted!\n");
	return;
}

void timer_sleep()
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = signal_handler;
	if(sigaction(SIGINT, &sa, NULL) == -1)
		errExit("sigaction\n");

	struct timespec ts, remain;
	ts.tv_sec = 5;
	ts.tv_nsec = 0;

	printf("Process will be sleep...\n", remain.tv_sec);
	if(nanosleep(&ts, &remain) == -1) {
		printf("remain time: %d", remain.tv_sec);
	}

}

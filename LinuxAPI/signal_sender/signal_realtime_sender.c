/*
 * signal_realtime_sender.c
 *
 *  Created on: Apr 22, 2016
 *      Author: ahnmh
 */
#define _POSIX_C_SOURCE 199309
#include <stdio.h>
#include <signal.h>
#include "signal_realtime_sender.h"

void signal_sender(int pid, int sig)
{
	union sigval sv;
	sv.sival_int = sig;

	if(sigqueue(pid, sig, sv) == -1)
		perror("sigqueue\n");
}


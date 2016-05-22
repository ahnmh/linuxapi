/*
 * signal_sender.c
 *
 *  Created on: Apr 22, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include <stdlib.h>
#include "signal_realtime_sender.h"

int main(int argc, char *argv[])
{
	int pid = atoi(argv[1]);
	int sig = atoi(argv[2]);

	signal_sender(pid, sig);

	printf("signal %d has been sent.\n", sig);

	return 0;
}

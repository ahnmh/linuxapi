/*
 * signal_sender.c
 *
 *  Created on: Apr 22, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include "signal_realtime_sender.h"

int main(int argc, int *argv[])
{
	int pid = atoi(argv[1]);

	signal_sender(pid);

	return 0;
}

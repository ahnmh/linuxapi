/*
 * helper.c
 *
 *  Created on: Jun 28, 2016
 *      Author: ahnmh-vw
 */

#include "helper.h"

#include <stdio.h>
#include <stdlib.h> // exit

void errexit(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}



/*
 * process_manage.h
 *
 *  Created on: Jul 10, 2016
 *      Author: ahnmh
 */

#ifndef PROCESS_MANAGE_H_
#define PROCESS_MANAGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "helper.h"

void fork_exec_wait();
void system_sample();

#endif /* PROCESS_MANAGE_H_ */

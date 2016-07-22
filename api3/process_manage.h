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
void make_daemon();

void nice_control();
void priority_control();

void affinity_control();
void sched_control();
void limit_control();

#endif /* PROCESS_MANAGE_H_ */

/*
 * process_func.h
 *
 *  Created on: Apr 27, 2016
 *      Author: ahnmh
 */

#ifndef PROCESS_FUNC_H_
#define PROCESS_FUNC_H_

#include <sys/types.h>
#include <sys/wait.h>

void process_func();
void process_sync_by_signal();
void process_exit();
void process_wait();
void process_waitpid();
void process_exec();
void process_clone();
void process_setpgid();
void process_priority();
void process_scheduler();
void process_resources();

#endif /* PROCESS_FUNC_H_ */

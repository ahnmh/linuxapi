/*
 * pthread_example.h
 *
 *  Created on: Jul 15, 2016
 *      Author: ahnmh-vw
 */

#ifndef PTHREAD_EXAMPLE_H_
#define PTHREAD_EXAMPLE_H_
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "helper.h"

void create();
void exit_cancel();

void mutex_example();
void mutex_cond_example();

#endif /* PTHREAD_EXAMPLE_H_ */

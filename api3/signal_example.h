/*
 * signal_example.h
 *
 *  Created on: Jul 22, 2016
 *      Author: ahnmh-vw
 */

#ifndef SIGNAL_EXAMPLE_H_
#define SIGNAL_EXAMPLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "helper.h"

void signal_legacy();
void signal_send();
void signal_helper();
void signal_block();
void signal_suspend();
void signal_sigaction();

#endif /* SIGNAL_EXAMPLE_H_ */

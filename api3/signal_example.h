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

#endif /* SIGNAL_EXAMPLE_H_ */

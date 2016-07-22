/*
 * mem.h
 *
 *  Created on: Jul 18, 2016
 *      Author: ahnmh-vw
 */

#ifndef MEM_H_
#define MEM_H_

#include <stdio.h>
#include <stdlib.h>
#include "helper.h"

void mem_alloc();
void mem_anonymous_mmap();
void mem_alloc_control();

void mem_alloca(int argc, char *argv[]);
void mem_variable_len_array(int argc, char *argv[]);

void mem_handle_sample(int argc, char *argv[]);
void mlock_sample();

#endif /* MEM_H_ */

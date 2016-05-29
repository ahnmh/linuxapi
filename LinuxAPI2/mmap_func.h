/*
 * mmap_func.h
 *
 *  Created on: May 27, 2016
 *      Author: ahnmh
 */

#ifndef MMAP_FUNC_H_
#define MMAP_FUNC_H_

void mmap_file_privated(int argc, char *argv[]);
void mmap_file_shared(int argc, char *argv[]);
void mmap_anonymous_shared();
void mmap_ops();

#endif /* MMAP_FUNC_H_ */

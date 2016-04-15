/*
 * process_env.c
 *
 *  Created on: Apr 5, 2016
 *      Author: ahnmh
 */
#include "process_env.h"
#include "tlpi_hdr.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> // getenv

// global symbols for segment(section)
extern char etext, edata, end;
// global symbol for enviornment variables. it's pointing start address of total enviornment variables.
extern char **environ;


int get_enviorn()
{
	int ret;
	char *env = getenv("SHELL");
	char **ep = environ;


	printf("PID=%d\n", getpid());
	printf("PPID=%d\n", getppid());

	printf("etext=%8p\n", &etext); // text segment
	printf("edata=%8p\n", &edata); // initialized data segment
	printf("end=%8p\n", &end); // uninitialized data segment

	printf("$PS1=%s\n", env); // /bin/bash


	ret = setenv("AHNMH", "myunghoon ahn", 0); // do not overwrite
	if(ret == -1)
		errExit("setenv");

	// printout total enviornment variables
	for (ep = environ; *ep != NULL; ep++) {
		puts(*ep);
	}

	ret = unsetenv("AHNMH");
	if(ret == -1)
		errExit("unsetenv");


	return EXIT_SUCCESS;
}

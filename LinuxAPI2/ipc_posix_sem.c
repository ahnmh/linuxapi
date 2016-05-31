/*
 * ipc_posix_sem.c
 *
 *  Created on: May 31, 2016
 *      Author: ahnmh-vw
 */
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ipc_posix_func.h"
#include "tlpi_hdr.h"

void ipc_posix_sem_create()
{
	int flags;
	mode_t perms;
	unsigned int value;
	sem_t *sem;

	flags = O_CREAT | O_EXCL | O_RDWR;
	perms = S_IRUSR | S_IWUSR | S_IWGRP;

	value = 0;
	sem = sem_open(POSIX_SEM, flags, perms, value);
}

void ipc_posix_sem_acquire()
{
	sem_t *sem;
	sem = sem_open(POSIX_SEM, 0);
	if (sem == SEM_FAILED)
		errExit("sem_open()");

	if (sem_wait(sem) == -1)
		errExit("sem_wait()");

	printf("semaphore acquired.\n");

	int value;
	if (sem_getvalue(sem, &value) == -1)
		errExit("sem_getvalue()");
	printf("semaphore value = %d\n", value);
}

void ipc_posix_sem_release()
{
	sem_t *sem;
	sem = sem_open(POSIX_SEM, 0);
	if (sem == SEM_FAILED)
		errExit("sem_open()");

	if (sem_post(sem) == -1)
		errExit("sem_post()");

	printf("semaphore released.\n");

	int value;
	if (sem_getvalue(sem, &value) == -1)
		errExit("sem_getvalue()");
	printf("semaphore value = %d\n", value);

}

void ipc_posix_sem_destroy()
{
	if (sem_unlink(POSIX_SEM) == -1)
		errExit("sem_unlink()");
}


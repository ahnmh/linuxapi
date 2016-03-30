/*
 * file_io.c
 *
 *  Created on: Mar 30, 2016
 *      Author: root
 */


#include <fcntl.h>
#include <sys/stat.h>
#include "file_io.h"


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

int main()
{
	int input_fd, output_fd, open_flags;
	mode_t fileperms; // unsigned int
	ssize_t numread; // long int

	char buf[BUFFER_SIZE];

//	// generic file
//	const char* filename_r = "read";
//	const char* filename_w = "write";

	// to terminal
	const char* filename_r = "read";
	const char* filename_w = "/dev/tty1";

	// read file
	input_fd = open(filename_r, O_RDONLY); // fctrl.h
	if (input_fd == -1)
		errExit("opening file %s", filename_r);

	// Create | Write | make exist file length to zero and erase exist data.
	open_flags = O_CREAT | O_WRONLY | O_TRUNC;
	// rw-rw-rw-
	fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	// rw-r--r--
//	fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	// open file to write
	output_fd = open(filename_w, open_flags, fileperms);
	if (output_fd == -1)
		errExit("opening file %s", filename_w);

	// read from input_fd and write to output_fd
	// read() will return 0 if there is no more data to read.
	while ((numread = read(input_fd, buf, BUFFER_SIZE)) > 0) {
		if (write(output_fd, buf, numread) != numread)
			fatal("Couldn't write whole buffer");
	}

	// read failure
	if (numread == -1)
		errExit("read");

	if (close(input_fd) == -1)
		errExit("Close input");

	if (close(output_fd) == -1)
		errExit("Close output");

	return EXIT_SUCCESS;
}

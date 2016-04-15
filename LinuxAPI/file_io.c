/*
 * file_io.c
 *
 *  Created on: Mar 30, 2016
 *      Author: root
 */
#include "tlpi_hdr.h"
#include "file_io.h"
#include <fcntl.h>
#include <sys/uio.h> // struct iovec
#include <sys/syscall.h> // syscall
#include <pthread.h> // pthread_create

int file_open_read_write()
{
	int input_fd, output_fd, open_flags;
	mode_t fileperms; // unsigned int
	ssize_t numread; // long int

	char buf[BUFFER_SIZE];

	// generic file
	const char* filename_r = "read";
	const char* filename_w = "write";
	int offset = 10;

/*
	// to terminal
	const char* filename_r = "read";
	const char* filename_w = "/dev/tty1";
*/

	// read file
	input_fd = open(filename_r, O_RDONLY); // fctrl.h
	if (input_fd == -1)
		errExit("open(): %s", filename_r);

	// Create | Write | make exist file length to zero and erase exist data.
	open_flags = O_CREAT | O_WRONLY | O_TRUNC;
	// rw-rw-rw-
	fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	// rw-r--r--
//	fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
//	below also possible
//	fileperms = 0644;

	// open file to write
	output_fd = open(filename_w, open_flags, fileperms);
	if (output_fd == -1)
		errExit("open(): %s", filename_w);

/*
	// read from input_fd and write to output_fd
	// read() will return 0 if there is no more data to read.
	while ((numread = read(input_fd, buf, BUFFER_SIZE)) > 0) {
		if (write(output_fd, buf, numread) != numread)
			fatal("Couldn't write whole buffer");
	}
*/

	// move file pointer +10 from the beginning
	if (lseek(input_fd, offset, SEEK_SET) == -1)
			errExit("lseek");

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

int file_fcntl()
{
	int input_fd;
	// generic file
	const char* filename_r = "read";
	int flags, access_mode;

	input_fd = open(filename_r, O_RDONLY); // fctrl.h
	if (input_fd == -1)
		errExit("open(): %s", filename_r);

	// get current flags value
	flags = fcntl(input_fd, F_GETFL);
	if(flags == -1)
		errExit("fcntl()");

	if(flags & O_SYNC)
		printf("open are synchronized.\n");

	// to get access mode
	access_mode = flags & O_ACCMODE;
	if(access_mode == O_RDONLY)
		printf("read only.\n");


	// add mode
	flags |= O_APPEND;
	flags = fcntl(input_fd, F_SETFL, flags);
	if(flags == -1)
		errExit("fcntl()");



	return EXIT_SUCCESS;
}

// read string from file and save it to scatter buffer memory.
int file_scatter_gather()
{
	int read_fd, write_fd;
	ssize_t num_read, num_write, total_required;
	const char* filename_r = "read";
	const char* filename_w = "write";
	int open_flags = O_CREAT | O_WRONLY | O_TRUNC;
	mode_t fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;





	// read file
	read_fd = open(filename_r, O_RDONLY); // fctrl.h
	if (read_fd == -1)
		errExit("open(): %s", filename_r);

	struct iovec iov[3];
	char str1[10];
	char str2[5];
	char str3[3];

	iov[0].iov_base = str1;
	iov[0].iov_len = 10;
	total_required = 10;

	iov[1].iov_base = str2;
	iov[1].iov_len = 5;
	total_required += 5;

	iov[2].iov_base = str3;
	iov[2].iov_len = 3;
	total_required += 3;

	num_read = readv(read_fd, iov, 3);
	if (num_read == -1)
		errExit("num_read()");

	if(num_read < total_required)
		printf("read bytes are smaller than required.\n");

	write_fd = open(filename_w, open_flags, fileperms);
	if (write_fd == -1)
		errExit("open(): %s", filename_w);


	num_write = writev(write_fd, iov, 3);
	if(num_write < total_required)
		printf("written bytes are smaller than required.\n");


	return EXIT_SUCCESS;
}


int file_ftruncate()
{
	int input_fd;
	// generic file
	const char* filename = "read";

	input_fd = open(filename, O_WRONLY); // fctrl.h
	if (input_fd == -1)
		errExit("open(): %s", filename);

	int ret = ftruncate(input_fd, 10);
	if (ret == -1)
		errExit("ftruncate()");


	return EXIT_SUCCESS;
}

int file_opentempfile()
{
	int temp_fd;
	// file name will be changed to another name like as "temporaryfileAjDj86"
	char tempfile[] = "temporaryfileXXXXXX";
	temp_fd = mkstemp(tempfile);
	if (temp_fd == -1)
		errExit("mkstemp(): %s", tempfile);

	// can't see the file after unlink() but it's still alive so we can read and write it.
	unlink(tempfile);

	/*	do your job for temp file..*/
	char buf[] = "This is test.";
	int numwrite = write(temp_fd, buf, strlen(buf));
	if(numwrite == -1)
		errExit("write(): %d", temp_fd);

	// both read and write action handle same fd so,
	// before read it we have to reset file pointer to the start position.
	// because write will move file pointer to the end of file.
	if (lseek(temp_fd, 0, SEEK_SET) == -1)
			errExit("lseek");

	char readbuf[30];
	int numread = read(temp_fd, readbuf, numwrite);
	if(numread == -1)
		errExit("read(): %d", temp_fd);

	// temp file will be actually removed after calling close()
	if(close(temp_fd) == -1)
		errExit("close(): %d", temp_fd);


	return EXIT_SUCCESS;
}



void thread_func(void *user_data)
{
	int *fd = (int*)user_data;
	char buf[6];


	int i;
	int offset = 0;
	for (i = 0; i < 10; i++) {
//		int numread = read(*fd, buf, 5);

		int numread = pread(*fd, buf, 5, offset);
		if(numread == -1)
			errExit("read(): %d", *fd);

		buf[5] = '\0';

		printf("[%ld]%s ", syscall(SYS_gettid), buf);
	}
}

int file_pread_pwrite()
{
	int fd1, fd2;
	char *filename = "read";

	fd1 = open(filename, O_RDONLY); // fctrl.h
	if (fd1 == -1)
		errExit("open(): %s", fd1);

//	fd2 = open(filename, O_RDONLY); // fctrl.h
//	if (fd2 == -1)
//		errExit("open(): %s", fd2);

	pthread_t thread1, thread2;
	int tid1 = pthread_create(&thread1, NULL, (void*)&thread_func, (void*)&fd1);
	int tid2 = pthread_create(&thread2, NULL, (void*)&thread_func, (void*)&fd1);

	int status1, status2;
	pthread_join(thread1, (void**)&status1);
	pthread_join(thread2, (void**)&status2);

	if(close(fd1) == -1)
		errExit("close(): %d", fd1);

//	if(close(fd2) == -1)
//		errExit("close(): %d", fd2);

	return EXIT_SUCCESS;

}

/*
쓰레드와 파일 포인터

1. 쓰레드가 서로 다른 파일 디스크립터를 사용해서 같은 파일에 읽기 동작을 수행하는 경우:
 파일 디스크립터가 다르면, 파일이 같더라도 파일 포인터가 서로 다르므로, 서로 영향을 받지 않는다.

2. 쓰레드가 같은 파일 디스크립터를 사용해서 같은 파일에 읽기 동작을 수행하는 경우:
 파일 디스크립터가 같으면 파일 포인터도 동일하므로 스레드1의 읽기 동작을 통한 파일 포인터 이동은 스레드2에 영향을 준다.
 pread의 경우 파일에 접근해서 읽기 동작을 수행하고 파일 포인터를 원래 위치로 옮겨 놓는 동작까지 아토믹하게 수행하므로
 서로 영향을 주지 않을 수 있다.
*/

/*
 * file_buffering.c
 *
 *  Created on: Apr 12, 2016
 *      Author: ahnmh
 */

#define _GNU_SOURCE	// to use O_DIRECT flag
#include <stdio.h>
#include "tlpi_hdr.h"
#include "file_func.h"
#include <fcntl.h>
#include <malloc.h> // memalign


int file_buffering()
{
	int ret;

/*
	stdio 유저 버퍼 관련 함수: setvbuf, fflush
	커널 버퍼 함수: fsync, fdatasync, O_SYNC 플래그

	주의해야 할 것은 유저 버퍼와 커널 버퍼가 별개라는 것임.
	따라서 setvbuf의 설정이 커널 버퍼에 영향을 주는 것이 아님.
	fflush 해서 유저 버퍼 내용을 쓰기해도 커널 버퍼에서 디스크까지 써지는 것은 아님.
	마찬가지로 또한 fsync를 해도 유저 버퍼가 비워지는 것은 아님.
*/

/*
	[fprintf와 write의 차이점]

	fprintf, fwrite, fputs 등의 함수는 유저모드 라이브러리 함수로서 내부적으로 시스템 콜인 write를 호출한다.
	하지만 한번의 fprintf 호출이 반드시 write를 호출하는 것이 아니다.

	fprintf로 파일에 쓰는 경우 8192byte크기의 버퍼를 기본적으로 가지고 있으며 8192가 가득찬 이후에 write를 호출한다.
	터미널에 쓰는 경우에는 이와 다른데, 개행 문자를 만날때까지 버퍼에 채워두었다가 write를 한다.
*/


/*	sample for stdout

	// NULL = 버퍼 사이즈와 크기는 자동 할당됨.
	// _IOFBF = 완전히 버퍼링된 I/O를 사용
	// _IONBF = 줄바꿈 문자를 나타내는 글자가 출력되기 전까지 버퍼링. 터미널 디바이스에 대한 스트림에 기본값.
	// _IOLBF = I/O를 버퍼링하지 않고 즉각적으로 write, read 시스템 호출을 야기한다. stderr의 기본값.
	ret = setvbuf(stdout, NULL, _IOFBF, 0);
	printf("This is test.\n");

	// flush current stdout buffer.
	// "This is test." will be written at this time.
	ret = fflush(stdout);

	printf("This is test - after fflush\n");

*/


/* setvbuf 함수를 사용하기 전 */

	int fd;
	mode_t fileperms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	FILE *fstream;

	const char* filename = "write";
	// open file
	fd = open(filename, O_RDWR | O_CREAT, fileperms); // fctrl.h

	// fd로부터 파일스트림을 얻기 위한 함수
	fstream = fdopen(fd, "w+");
	// 또는 직접 열수 있다.
	// fstream = fopen(filename, "w+");

	// 파일에 쓰기 - 실제로 파일에 쓰이는 것이 아니라 stdio 버퍼(user mode)에 쓰여진다.
	fprintf(fstream, "This is test for normal file stream\n");

	// 파일 닫기. close함수를 중복으로 호출할 필요 없다.
	fclose(fstream);
	// 실제 파일에 쓰이는 시점은 fclose이후에 해당한다.



	/* setvbuf 함수를 사용 */

	fstream = fopen(filename, "w+");
	// 버퍼링 정책을 No buffering으로 변경한다.
	ret = setvbuf(fstream, NULL, _IONBF, 0);
	// 아래 내용은 stdio 버퍼(user mode)를 거치지 않고 바로 파일에 쓰여질 것이다.
	fprintf(fstream, "This is test for normal file stream - Set no buffering.\n");
	fclose(fstream);


	/* setvbuf 함수를 사용 */

	char *stdio_buf = malloc(10);
	fstream = fopen(filename, "w+");
	// 버퍼링 정책을 buffering으로 변경, stdio 버퍼를 명시적으로 동적할당하여 지정
	ret = setvbuf(fstream, stdio_buf, _IOFBF, 10);
	// It designated to use buffer but, 버퍼의 크기가 쓰기보다 작기 때문에 바로 flush 된다.
	fprintf(fstream, "This is test for normal file stream - Set buffer but small size.\n");
	fclose(fstream);


	/* flush kernel buffer by fsync */

	fstream = fopen(filename, "w+");
	char *strbuf = "This is test for normal file stream - fsync\n";
	write(fd, strbuf, strlen(strbuf));

/*
	 write를 호출해도 실제 물리 디스크에 바로 쓰는 것이 아니라 커널 버퍼에 일차적으로 저장되었다가
	 버퍼가 가득차거나 일정 시간이 되면 실제 물리 디스크에 써진다.

	 그러나 출력 파일에 대한 커널 버퍼의 flush를 강제할 경우가 있을 수 있다.
	 : 어플리케이션이 작업을 진행하기 전에 실제로 출력이 디스크에 기록되었는지 확인하기 위한 경우
*/

	// 커널 버퍼의 내용을 디스크로 flush하고 열린 파일 fd와 관련된 모든 메타데이터 또한 디스크로 쓴다.
	ret = fsync(fd);

	// 커널 버퍼의 내용만 디스크로 flush하고 수정되지 않은 메타데이터는 계속 버퍼 상태를 유지한다.
	// 따라서 디스크 동작의 횟수가 적기 때문에 속도에 유리함.
//	ret = fdatasync(fd);

	fclose(fstream);



/*
	O_SYNC flag
	파일을 열 때 O_SYNC 플래그를 사용하면 write 가 호출되면 바로 파일에 써진다.
	즉 매 쓰기마다 fsync를 호출하는 것과 같다.
*/
	fd = open(filename, O_RDWR | O_CREAT | O_SYNC, fileperms); // fctrl.h
	fstream = fopen(filename, "w+");
	write(fd, strbuf, strlen(strbuf));
	fclose(fstream);



/*
	DIRECT_IO:
	디스크 I/O를 실행할 때 버퍼 캐시를 우회하여 사용자 공간의 데이터를 디스크나 파일에 직접 전송함
	대부분의 리눅스 고유 파일 시스템은 O_DIRECT를 지원하지만 vfat과 같은 리눅스 이외의 파일 시스템은 지원하지 않는다.
	O_DIRECT를 사용할 때는 정렬 제한이 있다.
 		: 전송에 사용되는 데이터 버퍼는 블록 크기의 배수로 메모리 경계에 정렬되어야 함.
		: 데이터 전송이 시작된 파일이나 디바이스 옵셋은 블록 크기의 배수여야 한다.
		: 전송되는 데이터의 크기는 블록의 배수여야 함.
*/

	int numread;
	int blocksize = 512;
	fd = open("temp", O_RDONLY | O_DIRECT, fileperms);
	void *aligned_buf = (char*)memalign(blocksize, blocksize * 4);

	numread = read(fd, aligned_buf, blocksize * 2);
	if(numread == -1)
		errExit("read()");

	close(fd);

	return ret;


	// stdio buffer and kernel buffer: see the picture no. 13-1 of Basic Linux API
}

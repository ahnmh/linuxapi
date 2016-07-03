/*
 * fileio_stdio.c
 *
 *  Created on: Jul 3, 2016
 *      Author: ahnmh
 */

#define _GNU_SOURCE	//fputs_unblock
#include "fileio.h"

struct test {
	char name[20];
	int number;
};

/*
표준 입출력 라이브러리를 통해 문자열 쓰기
표준 입출력 라이브러리를 통해 바이너리 데이터 읽고 쓰기
*/
void fileio_stdio_write()
{
	/*
	const char *file = "test";
	int fd;

	fd = open(file, O_RDWR);
	if(fd == -1)
		errexit("open");

	FILE *stream;
	// 파일 디스크립터로 스트림 열기
	stream = fdopen(fd, "a+");
	if(!stream)
		errexit("fdopen");

	// 문자열 기록
	if(fputs("This is test\n", stream) == EOF)
		errexit("fputs");

	if(fclose(stream) == EOF)
		errexit("fclose");
*/

	const char *file_b = "test_b";

	FILE *stream_b;
	// 파일 이름으로 스트림 열기
	stream_b = fopen(file_b, "w+");
	if(!stream_b)
		errexit("fopen");

	// 바이너리 데이터 기록
	struct test mytest = {"Myunghoon Ahn", 12345};
	if(!fwrite(&mytest, sizeof(struct test), 1, stream_b))
		errexit("fwrite");

/*
	아래 읽기 시도에서 정상적으로 읽으려면 읽기 전에 쓰기를 flush해야 한다.
	실제 파일에 쓰기 전이기 때문에 정상적인 값이 읽히지 않음.
	따라서, fclose 과정이 필요함.
	또는 fflush(사용자 버퍼내용을 커널버퍼로 넘김) -> sync(커널 버퍼 내용을 실제 파일로 넘김) 함수 호출이 필요하다.
*/

//	if(fclose(stream_b) == EOF)
//		errexit("fclose");
//
	// 사용자 버퍼에 있는 데이터를 커널 버퍼로 쓰기. write를 직접 사용하는 것과 동일함.
	fflush(stream_b);
	// 커널 버퍼의 내용을 디스크에 쓰기. fsync 함수를 쓰기 위해 먼저 스트림으로부터 파일 디스크립터를 구한다.
	int fd_test_b = fileno(stream_b);
	fsync(fd_test_b);

	stream_b = fopen(file_b, "r");
	if(!stream_b)
		errexit("fopen");

	// 바이너리 데이터 읽기
	struct test myread;
	if(!fread(&myread, sizeof(struct test), 1, stream_b))
		errexit("fread");

	if(ferror(stream_b)) // 스트림에 에러가 발생했는지 검사
		printf("error on f\n");
	if(feof(stream_b)) // 스트림이 EOF에 도달했는지 검사
		printf("EOF on stream_b\n");
	clearerr(stream_b); // 에러와 EOF를 초기화

	printf("%s = %d\n", myread.name, myread.number);

	if(fclose(stream_b) == EOF)
		errexit("fclose");


}

// 표준 입출력 라이브러리를 thread-safe하게 사용하는 방법
void fileio_stdio_write_lock()
{
	const char *file = "test";
	FILE *stream;

	// 파일 디스크립터로 스트림 열기
	stream = fopen(file, "a+");
	if(!stream)
		errexit("fdopen");

	// stream에 락을 증가시키고 스레드가 stream을 소유하게 한다.
	flockfile(stream);

/*
	thread-safe 한 구간으로서 아래 2개의 파일 쓰기 명령 사이에 다른 스레드가 끼어들 수 없다.
	fputs은 기본적으로 함수 호출 수준의 thread-safe를 보장한다.
	그러나 flockfile을 통해 보호받을 때는 함수 호출 수준의 lock 검사가 필요없으므로
	lock 검사를 수행하지 않고 성능이 개선되는 fputs_unblocked를 사용할 수 있다.
	* _GNU_SOURCE 선언이 필요
*/
	if(fputs_unlocked("This is test1\n", stream) == EOF)
		errexit("fputs");
	if(fputs_unlocked("This is test2\n", stream) == EOF)
		errexit("fputs");

	// stream에 연관된 락 카운터를 감소시킨다.
	// 락 카운터가 0이되면 현재 스레드는 stream의 소유권을 포기하여 다른 스레드가 락을 얻게 한다.
	funlockfile(stream);

	if(fclose(stream) == EOF)
		errexit("fclose");

}

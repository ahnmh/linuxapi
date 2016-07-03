/*
 * fileio_stdio.c
 *
 *  Created on: Jul 3, 2016
 *      Author: ahnmh
 */

#define _GNU_SOURCE	//fputs_unblock
#include "fileio.h"

/*
표준 입출력 라이브러리(stdio)
 - 플래폼 독자적인 유저 모드 버퍼링을 제공함.
 - 유저모드 버퍼링
   데이터가 쓰여지면 프로그램 주소 공간 내 버퍼에 우선 저장된다.
   버퍼가 특정 크기에 도달하면 전체 버퍼는 한 번의 쓰기 연산을 통해 실제로 기록된다.
   읽기 역시 버퍼 크기에 맞춰 블록에 정렬된 데이터를 읽는다.
   어플리케이션이 이상한 크기로 읽기나 쓰기를 요청해도 데이터는 하나의 큰 버퍼에 저장되어 있다가 블록에 맞춰 파일 시스템으로 전달된다.
   따라서 데이터가 많더라고 모두 블록 크기에 맞춰 적은 횟수의 시스템 콜을 사용하므로 성능 향상을 가져올 수 있다.
   (시스템 콜을 통해 읽기/쓰기 수행시 디스크 블록 크기의 배수로 연산을 수행하지 않으면 성능 저하가 발생함. 이는 커널과 하드웨어가 블록 크기를 기준으로 대화하기 때문)
 - 유저 모드 버퍼링을 제공함으로써 1바이트씩 읽어도 성능 저하 우려가 없다.
*/

void fileio_stdio_read()
{
	FILE *stream; // 파일 스트림
//	int fd;

/*	모드
	r : 읽기
	r+ : 읽기/쓰기
	w : 쓰기, 파일이 존재하면 0으로 만듬, 파일이 없으면 생성
	w+ : 읽기/쓰기, 파일이 존재하면 0으로 만듬, 파일이 없으면 생성
	a : append 쓰기 모드, 파일이 없으면 생성, 쓰기는 파일 끝에서 진행됨
	a+ : append 읽기/쓰기 모드, 파일이 없으면 생성, 쓰기는 파일 끝에서 진행됨
*/
	// 파일 스트림을 여는 함수
	stream = fopen("test", "r");
	if(!stream)
		errexit("fdopen");

/*
	// 파일 디스크립터를 통해 파일 스트림을 여는 함수
	fd = open("test", O_RDONLY);
	if(fd == -1)
		errexit("open");

	stream = fdopen(fd, "r");
	if(!stream)
		errexit("fdopen");
*/

	char buf[BUF_SIZE];
/*
	// 한줄씩 읽기 - 개행문자 또는 EOF를 만날때까지 BUF_SIZE - 1만큼 읽은 후 마지막 위치에 '\0'을 추가함.
	// 개행 문자(\n)도 buf에 포함된다 -> 이 때문에 불편함.
	if(!fgets(buf, BUF_SIZE, stream))
		errexit("fgets");
*/


/*
	// 필요한 만큼(need) 문자열 읽기 - fgetc는 한 글자씩 읽는 함수인데, 이렇게 해도 사용자 버퍼를 사용하므로 성능 저하가 없다.
	char *idx;
	int ch;
	int need = 5;

	idx = buf;
	while(--need > 0 && (ch = fgetc(stream)) != EOF)
		*(idx++) = ch;
	*idx = '\0';
*/

	// 구분자를 정의하여 문자열 읽기(최대 길이 10)
	char *idx;
	int ch;
	int need = 10;
	int delimiter = '\n'; // 개행 문자를 구분자로 사용

	idx = buf;
	while(--need > 0 && (ch = fgetc(stream)) != EOF && (*(idx++) = ch) != delimiter);

	// 구분자에 의해 루프를 탈출한 경우
	if(ch == delimiter)
		*(--idx) = '\0';
	// 길이 제한에 의해 루프를 탈출한 경
	else
		*idx = '\0';

	printf("%s\n", buf);

	// 스트림 닫기
	if(fclose(stream) == EOF)
		errexit("fclose");

/*
	// 현재 프로세스의 모든 스트림 닫기
	fcloseall();
*/
}

/*
표준 입출력 라이브러리를 통해 문자열 쓰기
표준 입출력 라이브러리를 통해 바이너리 데이터 읽고 쓰기
*/

struct test {
	char name[20];
	int number;
};

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

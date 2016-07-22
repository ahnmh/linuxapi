/*
 * fileio_aio.c
 *
 *  Created on: Jul 7, 2016
 *      Author: ahnmh-vw
 */

#include "fileio.h"
#include <aio.h> // -lrt

/*
비동기 논블로킹 IO(Asynchronous non-blocking I/O(AIO))
- I/O request 이후, 즉시 리턴함.
- 어플리케이션은 다른 작업을 수행할 수 있고, I/O는 백그라운드에서 수행된다.
- I/O가 완료되면 시그널이나 쓰레드 기반 콜백으로 I/O 전달
*/

// aio 요청이 완료되면 실행되는 스레드
static void thread_func(sigval_t sv)
{
	struct aiocb *my_aiocb = sv.sival_ptr;
	ssize_t numwrite;

/*
	aio 요청 상태 확인한다. 다음 중 하나의 값을 리턴함.
	- EINPROGRESS: aio가 아직 완료되지 않음
	- ECANCELED: aio 요청이 취소됨.
	- 0: aio 요청이 성공적으로 완료됨.
*/
	if(aio_error(my_aiocb) != EINPROGRESS) {
/*
		aio 요청의 완료 상태를 확인한다.
		aio_error가 EINPROGRESS가 아닌 값을 리턴했을 때 확인해야 한다.
		write나 read 요청과 같은 값을 리턴한다.
*/
		if((numwrite = aio_return(my_aiocb)) == -1)
			errexit("aio_return");

		printf("write bytes = %ld, %s\n", numwrite, (char *)my_aiocb->aio_buf);
	}
}

#define MAX_LIST 5
void fileio_aio_write()
{
	int fd;
	fd = open("test", O_RDWR);
	if(fd == -1)
		errexit("open");

	// 비동기 I/O Request(aio)를 기술하는 구조체
	struct aiocb my_aiocb;
	bzero((void *)&my_aiocb, sizeof(struct aiocb)); //0으로 초기화하는 방법

	char buf[BUF_SIZE];
	strcpy(buf, "This is asynchronous call test!\n");

	my_aiocb.aio_fildes = fd;	// 파일 디스크립터
	my_aiocb.aio_nbytes = strlen(buf); // 버퍼 크기
	my_aiocb.aio_offset = 0;	// 파일 옵셋
	// aio 완료 통지는 스레드 방식으로 설정한다.
	my_aiocb.aio_sigevent.sigev_notify = SIGEV_THREAD;
	my_aiocb.aio_sigevent.sigev_notify_function = thread_func;
	my_aiocb.aio_sigevent.sigev_notify_attributes = NULL; // 스레드 속성
	my_aiocb.aio_sigevent.sigev_value.sival_ptr = &my_aiocb; // 스레드 생성시 전달할 파라미터. sigval_t sv의 sv.sival_ptr로 받을 수 있다.

	// 지정한 복수의 aio가 완료될 때까지 대기하는 aio_suspend 함수 호출시, 복수의 aio를 지정하는 aio 배열
	const struct aiocb *list_aiocb[MAX_LIST];
	bzero((void *)list_aiocb, sizeof(list_aiocb));
	list_aiocb[0] = &my_aiocb;

	// 쓰기 aio 수행. 성공하면 0을 리턴한다.
	if(aio_write(&my_aiocb) == -1)
		errexit("aio_write");

	// 리스트로 지정한 aio가 완료되거나 timeout이 발생할 때까지(여기서 무한) 스레드를 대기시킨다.
	// aio가 완료되면 0을 리턴한다.
	if(aio_suspend(list_aiocb, MAX_LIST, NULL) == -1)
		errexit("aio_suspend");

/*
	또는 aio_suspend를 호출하지 않고 다른 작업을 수행할 수 있다.
	aio가 완료되면 스레드가 호출된다.
*/

//	sleep(5);
}


/*
동기화(Synchronized)와 비동기화(Nonsynchronized)
동기식(Synchronous)과 비동기식(Asynchronous)

동기식 쓰기 연산:
쓰고자 하는 데이터가 커널 버퍼 캐시에 기록되기 전까지 리턴되지 않는다.
동기식 읽기 연산:
읽고자 하는 데이터가 어플리케이션에서 제공하는 사용자 영역의 버퍼에 저장되기 전까지 리턴되지 않는다.

비동기식 쓰기 연산: 데이터가 사용자 영역에 머무르고 있을지라도 리턴될 수 있다.
비동기식 읽기 연산: 읽으려는 데이터가 준비되기전에도 리턴될 수 있다.
비동기식 연산은 요청을 큐에 넣기만 할 뿐 실제로 요청된 작업을 수행하지 않는다.

동기화 연산은 동기식 연산보다 더 제약적이지만 안전하다.
동기화 쓰기 연산:
데이터를 디스크에 기록해서 커널 버퍼에 있던 데이터와 디스크에 기록된 데이터가 동기화되도록 보장한다.
동기화 읽기 연산:
항상 데이터의 최신 복사본을 디스크로부터 리턴한다.

쓰기 연산은 동기식 + 비동기화 연산임.
 - 쓰기 연산은 데이터가 커널 버퍼에 저장되기 전까지는 리턴되지 않는다.
읽기 연산은 동기식 + 동기화 연산임.
 - 읽기 연산은 최신 데이터를 버퍼로 읽어오기 전에는 리턴되지 않는다.
 */

/*
입출력 스케줄러
- 하드 디스크와 나머지 시스템 간의 성능 차이 때문에 입출력 요청을 순서대로 디스크로 보내는 것은 효율적이지 않다.
- 커널은 입출력 스케줄러를 구현해서 입출력 요청의 순서와 시간을 조작하는 방법으로 디스크 탐색 횟수를 최소화한다.

입출력 스케줄러 동작방식
- 입출력 스케줄러는 병합과 정렬이라는 2가지 기본 동작을 통해 디스크의 헤드 움직임을 최소화할 수 있다.
- 병합 : 둘 이상의 인접한 입출력 요청을 단일 요청으로 합침.(request 5 + request 6, 7 -> request 5, 6, 7)
- 정렬 : 대기 중인 입출력 요청을 블록 순서의 오름 차순으로 정렬.(블록 52, 109, 7 -> 블록 7, 52, 109)

deadline 입출력 스케줄러
- 리누스 엘리베이터 + 읽기 FIFO 큐 + 쓰기 FIFO 큐
- 리누스 엘리베이터 : 충분히 오래된 요청이 있으면 삽입, 정렬을 멈추고 먼저 처리함.
- 읽기 FIFO 큐는 읽기 요청만 담고 있고 쓰기 FIFO 큐는 쓰기 요청만 담고 있다.
- FIFO 큐에 들어 있는 요청은 만료 기간이 있다.(읽기 500ms, 쓰기 5000ms): 작업 특성상 쓰기는 읽기보다 지연시간이 길어도 됨.
- 새로운 입출력 요청이 들어오면 표준 큐에 삽입, 정렬되고 읽기 또는 쓰기 FIFO 큐의 끝 부분에 위치한다.
- 정렬된 표준 큐의 앞부분에 있는 입출력 요청을 디스크로 보낸다. 표준 큐는 블록 번호로 정렬되어 있으므로 탐색을 최소화하여 전체 처리량을 높인다.
- 만일, FIFO 큐 앞부분에 있는 아이템이 해당 큐의 만료기간보다 오래되면 입출력 스케줄러는 표준 큐에서 입출력을 처리하지 않고 해당 FIFO 큐에서 요청을 처리한다.

예측 입출력 스케줄러
- deadline 입출력 스케줄러 기반으로, 만약 입출력 스케줄러가 디스크의 동일 위치에 대한 읽기가 도착할 것을 알게 된다면 왔다 갔다 탐색하는 대신
  다음 읽기를 예측하고 기다릴 수 있게 된다.
- 예측 입출력 스케쥴러는 deadline 입출력 스케줄러로 시작하지만 예측 메커니즘을 추가하여 성능을 강화한다.
- 예측 입출력 스케줄러는 요청을 처리한 후 아무것도 하지 않고 6ms 을 더 기다린다.
- 6ms 은 어플리케이션이 파일 시스템의 동일한 부분에 대한 새로운 읽기를 요청할 충분한 시간이다.

CFQ(complete fair queueing) 입출력 스케줄러
- 프로세스마다 독자적인 큐를 할당하고, 각 큐는 시간을 할당받는다.
- 라운드 로빈 방식으로 큐를 순회하면서 큐에 허락된 시간이 남아있거나 요청을 다 처리할 때까지 요청을 처리한다.
- 시간은 남는데 요청이 남아 있지 않은 경우 짧은 시간(10ms)동안 큐에 들어오는 새 요청을 기다린다.
- 개별 큐 안에서 동기화된 요청(일반적으로 읽기)은 비동기 요청(일반적으로 쓰기)보다 더 높은 우선순위를 갖는다.
- 이런 식으로 CFQ는 읽기 요청을 배려하여 쓰기 요청이 읽기를 굶겨죽이는 문제를 회피한다.
- 대부분의 부하에 적합하며 가장 고려해볼만하다.

noop 스케줄려
- 가장 기본적인 스케줄러
- 정렬을 수행하지 않고 기본적인 병합만 수행한다.

SSD
- SSD에 대한 입출력은 정렬해봤자 큰 소득이 없으며 입출력 스케줄러의 기능이 큰 효과를 발휘하지 못한다.
- 따라서 SSD를 디스크로 사용하는 경우 noop 입출력 스케줄러를 사용한다.
- 또는 SSD라 할지라도 프로세스간 공정성을 위해 CFQ 입출력 스케줄러를 사용한다.

기본 입출력 스케줄러는 부팅 시 커널 명령행 인자인 iosched를 통해서 선택할 수 있다.
- cfq, deadline, noop이 가능함.

각 장치에 대해 /sys/block/<device. 예를 들면 sda>/queue/scheduler 값을 변경해서 입출력 스케줄러를 선택할 수 있다.
ahnmh-vw@ubuntu:~$ cat /sys/block/sda/queue/scheduler
noop [deadline] cfq

*/

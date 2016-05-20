/*
 * pthread_safe.c
 *
 *  Created on: May 20, 2016
 *      Author: ahnmh
 */

#include <pthread.h>
#include "pthread_func.h"
#include "tlpi_hdr.h"

#define BUFFER_LEN	256
static char buf[256];


/*
Thread-Unsafe 한 경우 출력은 아래와 같을 수 있다.
ahnmh@ahnmh-amor2:~/sample/LinuxAPI2/Debug$ ./LinuxAPI2
thread id = 0xa3a19700, Message: I'm an emproyee of samsung el
thread id = 0xa421a700, Message: I'm an emproyee of samsung el
ahnmh@ahnmh-amor2:~/sample/LinuxAPI2/Debug$ ./LinuxAPI2
thread id = 0x3074f700, Message: Hello, This is myunghoon Ahn.
thread id = 0x30f50700, Message: Hello, This is myunghoon Ahn.
ahnmh@ahnmh-amor2:~/sample/LinuxAPI2/Debug$ ./LinuxAPI2
thread id = 0x7fc700, Message: I'm an emproyee of samsung el
thread id = 0xffd700, Message: I'm an emproyee of samsung el
*/

static void *thread_func_unsafe(void *arg)
{
	char *str = (char*)arg;
	pthread_t tid = pthread_self();
	snprintf(buf, BUFFER_LEN, "Message: %s", str);

	printf("thread id = 0x%x, %s\n", (unsigned int)tid, buf);
	return NULL;
}

// 1회 초기화를 위해 pthread_once 전역 변수
static pthread_once_t once = PTHREAD_ONCE_INIT;
/*
TSD 데이터를 위한 키 - 스레드 별 자신의 TSD 데이터에 접근할 때 사용하는 키를 생성함. 구조는 아래와 같다.
Key[0] _______ Thread A TSD[0] -> TSD 버퍼
         |____ Thread B TSD[0] -> TSD 버퍼
         |____ Thread C TSD[0] -> TSD 버퍼
*/
static pthread_key_t message;

// TSD 소멸 함수
static void destructor(void *buf)
{
	free(buf);
}

// TSD 키 생성을 위한 1회 호출 함수
static void create_key()
{
	int s;
/*
	TSD 키를 생성하고 스레드가 종료될 때 자동 호출되는 소멸 함수를 지정한다.
	소멸 함수로는 TSD 버퍼의 주소가 넘어가고 소멸 함수 내부에서 할당 해제를 수행하면 된다.
*/
	s = pthread_key_create(&message, destructor);
	if(s != 0)
		errExitEN(s, "pthread_key_create");
}


/*
스레드별 데이터(Thread-Specific Data) API를 사용한 Thread-safe 구현
Thread-safe 한 경우 출력은 아래와 같을 수 있다.
ahnmh@ahnmh-amor2:~/sample/LinuxAPI2/Debug$ ./LinuxAPI2
thread id = 0xbbec5700, Message: I'm an emproyee of samsung electronics.
thread id = 0xbc6c6700, Message: Hello, This is myunghoon Ahn.
ahnmh@ahnmh-amor2:~/sample/LinuxAPI2/Debug$ ./LinuxAPI2
thread id = 0xd77c8700, Message: Hello, This is myunghoon Ahn.
thread id = 0xd6fc7700, Message: I'm an emproyee of samsung electronics.
*/

static void *thread_func_safe_with_tsd(void *arg)
{

	int s;
	char *tsd_buf;

/*
	스레드들을 위한 1회 초기화 함수를 호출한다.
	모든 스레드를 통틀어서(각 스레드별로 1회가 아닌) 단 한번만 호출이 수행된다.
*/
	s = pthread_once(&once, create_key);
	if(s != 0)
		errExitEN(s, "pthread_once");

	// TSD 버퍼의 포인터를 얻어온다.
	tsd_buf = pthread_getspecific(message);
	/*
		최초 호출시에는 저장된 TSD 버퍼가 없으므로 NULL이 리턴된다.
		이 경우, TSD를 신규로 생성한다.
	*/
	if(tsd_buf == NULL) {
		tsd_buf = malloc(BUFFER_LEN);
/*
		TSD 버퍼를 생성한 다음 키와 연관시킨다.
		이 키를 통해 각 스레드별 연관된 TSD 버퍼를 얻을 수 있다.(pthread_getspecific)
*/
		s = pthread_setspecific(message, tsd_buf);
		if(s != 0)
			errExitEN(s, "pthread_setspecific");
	}

	char *str = (char*)arg;
	pthread_t tid = pthread_self();
	snprintf(tsd_buf, 256, "Message: %s", str);

	printf("thread id = 0x%x, %s\n", (unsigned int)tid, tsd_buf);
	return NULL;
}


/*TLS를 이용한 Thread-safe 구현*/



void pthread_safe()
{
	pthread_t t1, t2;
	int s;

	char *str1 = "Hello, This is myunghoon Ahn.";
//	s = pthread_create(&t1, NULL, thread_func_unsafe, str1);
	s = pthread_create(&t1, NULL, thread_func_safe_with_tsd, str1);
	if(s != 0)
		errExitEN(s, "pthread_create");

	char *str2 = "I'm an emproyee of samsung electronics.";
//	s = pthread_create(&t2, NULL, thread_func_unsafe, str2);
	s = pthread_create(&t2, NULL, thread_func_safe_with_tsd, str2);
	if(s != 0)
		errExitEN(s, "pthread_create");

	s = pthread_join(t1, NULL);
	s = pthread_join(t2, NULL);
}


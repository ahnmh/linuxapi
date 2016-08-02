/*
 * gcc_extension.c
 *
 *  Created on: Jul 28, 2016
 *      Author: ahnmh-vw
 */

#include "gcc_extension.h"
#include <stdio.h>
#include <ctype.h> // toupper
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
/*
__attribute__(option)
: 컴파일러의 특정 옵션에 해당하는 기능을 enable 할 때 사용한다.
*/

/*
always_inline 키워드
- square1의 경우, 인라인 함수로 선언했지만 실제로는 컴파일러가 가능한 경우 인라인으로 선언한다.
- square2의 경우, always_inline 옵션을 enable하여 항상 인라인으로 선언한다.
*/
static inline int square1(int a)
{
	return a * a;
}

static inline __attribute__ ((always_inline)) int square2(int a)
{
	return a * a;
}

/*
pure 키워드
- 루프 최적화를 수행함.(아래 본문 설명 참조)
- 파라미터는 읽기로만 접근해야 함.
*/
static __attribute__ ((pure)) int string_length(const char *p)
{
	int i;
	for (i = 0; p[i] != '\0'; ++i);

	return i;
}

/*
const 키워드
- pure 보다 더 높은 수준의 최적화를 얻을 수 있다.(수학 함수들이 대표적인 상수 함수)
- 전역 변수에 접근하면 안되고 포인터를 인자로 받으면 안된다. 리턴값은 값으로 전달되어야 한다.
*/
static __attribute__ ((const)) int square3(int a)
{
	return a * a;
}

/*
noreturn 키워드
- 항상 exit()를 호출하여 리턴을 수행하지 않는 함수가 있는 경우 최적화를 수행함.
- 함수의 리턴은 항상 void이어야 한다.
*/
static __attribute__ ((noreturn)) void print_err(const char *p)
{
	fprintf(stderr, "[ERROR] %s\n", p);
	exit(EXIT_FAILURE);
}

/*
malloc 키워드
- 메모리를 새로 할당해서 존재하지 않는 유일한 포인터를 리턴하는 함수인 경우 최적화를 수행함.
*/
static __attribute__ ((malloc)) void * double_size(int size)
{
	return malloc(size);
}

/*
warn_unused_result 키워드
- 리턴을 하는 함수인데 해당 함수 호출 이후 리턴값을 사용하지 않는 경우, warning을 발생하도록 지정함.
- read와 같은 함수의 경우에 해당함.
- 최적화는 아니고 프로그래밍을 돕는다.
*/
static __attribute__ ((warn_unused_result)) int IsOdd(int chk)
{
	printf("check = %d\n", chk);
	return (chk % 2) ? 1 : 0;
}

/*
deprecated 키워드
- 해당 함수를 호출할 때 컴파일러가 경고를 내도록 한다.
- 권장하지 않거나 사라질 인터페이스임을 알려줄때 씀.
*/
static __attribute__ ((deprecated)) int old_version()
{
	printf("do not use this anynome!\n");
	return 0;
}


/*
unused 키워드
- 지정한 함수나 인자가 사용되지 않음을 알려주어 관련 경고를 출력하지 않는다.
*/
static __attribute__ ((unused)) int unused_func()
{
	printf("unused function does not show yellow bottom warning line\n");
	return 0;
}

/*
 packed 키워드
- 구조체를 패딩 없이 꽉 채우기
*/
struct test_unpacked {
	char a[10];
	int b;
};
struct __attribute__  ((packed)) test_packed {
	char a[10];
	int b;
};

/*
aligned (2의 배수) 키워드
- 특정 변수의 최소 정렬값을 지정한다.
*/
struct test_aligned {
	char a[10] __attribute__ ((aligned (32))); // 32바이트 크기로 정렬
	char b[10] __attribute__ ((aligned (32))); // 32바이트 크기로 정렬
};

// 아래와 같이 자주 사용해야 하는 키워드라면 미리 단축 표현을 사용할 수도 있다.
#if __GNUC__ >= 3
#undef inline
#define inline inline __attribute__ ((alwyas_inline))
#else
#define inline
#endif


/*
likely, unlikely 키워드
- 컴파일러에게 어디로 분기할지 힌트를 줘서 코드 재배치를 통해 성능을 향상시킨다.
- 거의 발생하지 않는 에러인 경우 unlikely를 사용하면 성능이 개선된다.
- 문법이 복잡하므로 아래와 같이 define해서 쓴다.
*/
#define likely(x)  __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)


/*
구조체 멤버의 옵셋
*/

#include <stddef.h> // offsetof 매크로가 정의된 헤더파일
struct offsettest {
	int a;
	int b;
	char *c;
	char d[10];
};

void gcc_extension()
{

/*	always_inline 키워드 테스트*/
	int i;
	int sum = 0;
	for (i = 0; i < 100; ++i) {
		sum += square1(i);
	}
	for (i = 0; i < 100; ++i) {
		sum += square2(i);
	}
	printf("sum = %d\n", sum);


/*	pure 키워드 테스트*/
	char *str = "this is test";
	int j;
	// 아래와 같이 호출했을 때 pure 함수가 아니라면 string_length는 동일한 값을 리턴하는데, 여러번 반복 호출된다.
	// pure 키워드가 적용되면 해당 함수를 한번만 호출하고 해당 리턴 값을 루프의 값을 사용하게 됨.
	// len = string_length(str); 한 후, len값으로 루프를 검사하는 것과 동일함.
	// 따라서 매번 다른 값을 리턴하는 함수는 pure 함수가 될 수 없다.
	for (j = 0; j < string_length(str); ++j) {
		printf("%c", toupper(str[j]));
	}
	printf("\n");


/*	const 키워드 테스트*/
	for (i = 0; i < 100; ++i) {
		sum += square3(i);
	}
	printf("sum = %d\n", sum);


/*	noreturn 키워드 테스트*/
//	print_err("what's wroing with you!");

/*	malloc 키워드 테스트*/
	void *p = double_size(5);
	strcpy(p, "0123456789");
	printf("p = %s\n", (char*)p);


/*	warn_unused_result 키워드 테스트*/
	// 리턴값을 쓰지 않는 경우, 아래와 같이 warning을 발생함.
	// warning: ignoring return value of ‘IsOdd’, declared with attribute warn_unused_result [-Wunused-result]
	int check = IsOdd(10);
	printf("result = %d\n", check);


/*	deprecated 키워드 테스트*/
	// 해당 함수를 호출하는 경우, 아래와 같은 warning을 발생함.
	// warning: ‘old_version’ is deprecated (declared at ../gcc_extension.c:93) [-Wdeprecated-declarations]
	int ret = old_version();


/*	unused 키워드 테스트*/
	// 아래와 같이 함수 뿐만 아니라 변수에도 쓸 수 있다.
	int unused __attribute__ ((unused))  = 10;

/*	packed 키워드 테스트*/
	// 일반 구조체의 경우 char 데이터 다음 int 데이터가 오면 3byte를 비워둔다. 따라서 구조체 크기는 1 + 3(padding) + 4 = 8
	printf("sizeof normal struct = %ld\n", sizeof(struct test_unpacked));
	// packed 구조체의 경우 char 데이터 다음 int 데이터가 오면 연달아 오게된다. 따라서 구조체 크기는 1 + 4 = 5
	printf("sizeof packed struct = %ld\n", sizeof(struct test_packed));
/*	aligned 키워드 테스트	*/
	printf("sizeof aligned struct = %ld\n", sizeof(struct test_aligned));

/*	unlikely 매크로 테스트 */
	ret = close(STDIN_FILENO);
	// unlikely: 분기문이 높은 확률로 거의 실행되지 않는 경우에 쓴다.
	if(unlikely(ret))
		perror("close");

/*	offsetof 매크로 테스트 */
	int start = offsetof(struct offsettest, d); // 4 + 4 + 8 = 16
	printf("start of member d = %d\n", start);

/*	case 범위 지정 테스트*/
	// GCC 정말 훌륭하다..
	// 생략 부호 ... 앞뒤로는 반드시 공백이 있어야 함.
	int val = 'Q';
	switch(val) {
	case 'A' ... 'F':
		printf("set 1\n");
		break;

	case 'G' ... 'T':
		printf("set 2\n");
		break;

	case 'U' ... 'Z':
		printf("set 3\n");
		break;
	}



}

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
- 리턴을 하는 함수인데 리턴값을 쓰지 않는 경우, warning을 발생하도록 지정함.
- read와 같은 함수의 경우
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
//	int ret = old_version();



}

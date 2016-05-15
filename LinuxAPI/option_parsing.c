/*
 * option_parsing.c
 *
 *  Created on: May 15, 2016
 *      Author: ahnmh
 */

#include <stdio.h>
#include <ctype.h>

#include "tlpi_hdr.h"

#define printable(ch) (isprint((unsigned char)ch)?ch:'#')

static void usage_error(char *progname, char *msg, int opt)
{
	if(msg != NULL && opt != 0)
		fprintf(stderr, "%s (-%c)\n", msg, printable(opt));
	fprintf(stderr, "Usage: %s [-p arg] [-s arg] [-y]\n", progname);

	exit(EXIT_FAILURE);
}

void option_parsing(int argc, char *argv[])
{
	int opt;
/*
 	getopt
 	 : 옵션에 해당하는 한 문자와 옵션의 인자를 파싱해서 옵션 문자 -> 리턴값, 전역변수 optarg에 전달인자에 해당하는 문자열을 저장함.
 	 : optstring에 지정되지 않은 사용할 수 없는 옵션값인 경우 '?'에 해당하는 문자를 리턴하고,
 	 : 옵션에 인자를 지정해야 한다고 되어 있는데 인자를 지정하지 않은 경우 ':'를 리턴함.
 	   단, 위의 2가지 오류 케이스를 구분하려면 optsting 맨 앞에 ':'를 추가해서 오류 메시지를 출력하지 않아야 한다.
 	optstring = 허용하는 옵션을 모두 기록.
	각 옵션 문자 뒤에는 콜론을 붙여서 해당 옵션에 인자가 필요함을 나타낸다.
	":p:x" -> 맨 앞의 ':'는 잘못된 옵션인 경우에 에러 메시지를 출력하지 않으며, 'p:'는 p 옵션은 반드시 인자가 필요하며, 'x'는 x옵션은 인자가 없음을 의미함.
	에러메시지를 출력하기 싫을 때는 ':'로 시작하는 optstring 뿐만 아니라 전역변수 opterr = 0으로 설정해도 된다.
*/
	while((opt = getopt(argc, argv, ":p:s:y")) != -1) {
		printf("option = %3d(%c), optind = %d\n", opt, printable(opt), optind); // getopt 함수가 argv를 처리할 때마다 전역변수 optind가 자동으로 증가함.

		switch(opt) {
		case 'p':
			printf("p option was specified with argument \"%s\".\n", optarg);
			break;
		case 's':
			printf("s option was specified with argument \"%s\".\n", optarg);
			break;
		case 'y':
			printf("y option was specified\n");
			break;
		case ':':
			usage_error(argv[0], "Missing argument", optopt); // 별도로 에러 메시지를 준비하므로 optsting 맨 앞에 ':'를 추가한 것임.
			break;
		case '?':
			usage_error(argv[0], "Unrecognized option", optopt); // 별도로 에러 메시지를 준비하므로 optsting 맨 앞에 ':'를 추가한 것임.
			break;
		}
	}

	printf("argc = %d, optind = %d\n", argc, optind);
	// optind보다 argc가 더 크면(getopt이후 아직 전달인자가 남아있다면), 옵션이 아닌 첫번째 전달인자이다.
	if(optind < argc)
		printf("first nonoption argument = %s\n", argv[optind]);

}

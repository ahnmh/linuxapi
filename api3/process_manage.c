/*
 * process_manage.c
 *
 *  Created on: Jul 10, 2016
 *      Author: ahnmh
 */
#include "process_manage.h"

void fork_exec_wait()
{
	pid_t child, pid;
	int status;

	// waitid함수에서 사용됨.
	// struct siginfo_t 가 아니다!!!! typedef로 선언되어 있음.
//	siginfo_t info;

/*
	fork :
	부모 프로세스와 동일한 자식 프로세스를 생성한다.
	fork의 리턴값은 자식 프로세스에서는 0, 부모 프로세스에서는 자식 프로세스의 pid가 됨.

	fork 이후 부모와 자식의 다른 점
	- 자식 프로세스의 pid는 부모 프로세스와 다른 값으로 새롭게 할당
	- 자식 프로세스의 ppid는 부모 프로세스의 pid
	- 리소스 통계는 0으로 초기화
	- 처리되지 않은 시그널은 사라지고 자식 프로세스로 상속되지 않음
	- 부모의 파일 락은 상속되지 않음.

	fork의 가장 흔한 사용 예는 새로운 프로세스를 생성하고 그 후에 새 프로세스에
	새로운 바이너리 이미지를 올리는 것(exec계열 함수)이다.(셸과 같이)


	copy-on-write :
	fork 이후 부모 프로세스 주소 공간을 모두 복사하지 않고 페이지에 대한 Copy-on-write를 수행한다.
	자식이나 부모가 리소스에 변경을 시도한다면 그 시점에 해당 리소스를 복사하고 복사본을 변경을 요청한 측에 전달한다.
	대부분의 fork는 후속 작업으로 exec가 이어지기 때문에 부모 프로세스의 주소 공간을 자식 프로세스의
	주소 공간으로 복사하는 일은 시간 낭비임.

*/
	switch(child = fork()) {
	case -1:
		errexit("fork");
		break;

	case 0:
		// child
		printf("pid = %d, ppid = %d\n", getpid(), getppid());

/*
		exec 계열 함수:
		새로운 프로그램을 실행한다.
		새로운 프로세스에서 새로운 프로그램을 실행하려면 fork -> exec 과정이 필요하다.
		호출이 성공하면 새로운 프로그램의 시작점으로 건너뛰므로 이전에 실행했던 코드는 프로세스의 주소 공간에 더 이상 존재하지 않게 됨.
		열린 파일은 exec 호출 과정에서 상속된다.
		새롭게 실행되는 프로그램이 파일 디스크립터 값을 알고 있다면 원래 프로세스에서 열었던 모든 파일에 접근이 가능하다.
		이를 사용하지 않을 때는 exec를 호출하지 전에 close를 사용해야 함.(fcntl을 통해 커널이 수행하도록 지시할 수 있음)
*/
/*
		execl:
		현재 프로세스를 첫번째 파라미터인 path가 가르키는 프로그램으로 대체함.
		l의 의미: 두번째 파라미터부터 가변인자를 사용해, 실행되는 프로세스로 전달되는 인자(args)로 사용된다.
		가변 인자의 목록은 항상 NULL로 끝나야 한다.
*/
//		if(execl("/usr/bin/vi", "vi", "test", NULL) == -1)
//			errexit("execl");

/*
		execv:
		v의 의미: 전달 인자를 배열(벡터)로 제공한다.
		배열을 사용하면 필요한 인자를 실행 시간에 결정할 수 있게 된다.
		execl이 파라미터 3개가 필요함에 반해 execv는 배열 원소 2개만을 사용하는 것에 유의.
*/
		char *args[] = {"/usr/bin/vi", "test", NULL};
//		if(execv(args[0], args) == -1)
//			errexit("execv");

/*
		execlp:
		p의 의미: path를 사용자의 실행 경로 환경 변수에서 찾는다.
		따라서 /usr/bin과 같이 전체 경로를 사용할 필요가 없다.
		p는 원래 셸에서 사용하기 위해 구현되었다.
		셸을 사용하므로 보안상의 이슈가 있다. 따라서 p가 들어가는 exec 함수는 사용하지 않는 것이 좋음.
*/
/*
		if(execlp("vi", "vi", "test", NULL) == -1)
			errexit("execlp");
*/
/*
		execve:
		e의 의미: 새로운 환경 변수를 전달할 수 있음을 의미함.
		실행 후 vim 에서 :sh 이후 아래와 같이 입력하여 환경변수가 전달됨을 확인할 수 있다.
		:sh(셸로 빠져나가기)
		$ echo $ENV1
		AHNMH
		또는 vim에서 :! echo $ENV1 도 가능.(!는 셸로 나가지 않고 명령 실행하는 방법)
*/
		// 생성되는 프로세스로 전달하는 환경변수 리스트.
		// 리스트는 반드시 NULL로 끝나야 한다.
		char *envp[] = {"ENV1=AHNMH", "ENV2=SAMSUNG", "ENV3=IOT", NULL};
		if(execve(args[0], args, envp) == -1)
			errexit("execve");

		exit(EXIT_FAILURE);

	default:
		// parent
/*
		wait - 부모가 자식의 종료(exit)를 대기
		일반적으로 부모가 fork이후 컨텍스트 스위칭되지 않는다면 부모가 먼저 수행됨.(성능 측면에서 더 우수함)
*/
//		if((pid = wait(&status)) == -1)
//			errexit("wait");

/*
		waitpid - 특정 자식만 기다릴 수 있게 함.
		pid 파라미터
		< -1: 프로세스 gid가 이 값의 절대값과 동일한 모든 프로세스에 대기함.
		-1: 모든 자식 프로세스
		0: 호출한 프로세스와 동일한 프로세스 그룹에 속한 모든 자식 프로세스
		> 1: 파라미터로 받은 pid와 일치하는 자식 프로세스를 기다림.
		option 파라미터
		- WNOHANG: 종료된 자식 프로세스가 없으면 블록하지 않고 바로 리턴된다.
*/
		if((pid = waitpid(child, &status, WNOHANG)) == -1)
			errexit("wait");

		if(pid == -1)
			errexit("waitpid");
		else if(pid == 0)
			printf("Not terminated yet\n");
		else {
			// wait의 status로 전달받은 exit 코드를 해석하기 위한 매크로
			// _exit를 통해 정상 종료된 경우 TRUE를 리턴
			if(WIFEXITED(status))
				printf("Normal termination with exit status = %d\n", WEXITSTATUS(status)); // _exit에 넘긴 하위 8bit를 구함.
			// 시그널에 의해 종료되면 TRUE를 리턴
			if(WIFSIGNALED(status))
				printf("Killed by signal = %d\n", WTERMSIG(status)); // 프로세스를 종료시킨 시그널 번호를 구함.
			// 프로세스가 멈추거나 다시 실행되는 경우 TRUE
			if(WIFSTOPPED(status))
				printf("Stopped by signal = %d\n", WSTOPSIG(status)); // 프로세스를 멈추게 한 시그널 번호를 구함.

			if(WIFCONTINUED(status))
				printf("Continued\n");
		}


/*
		waitid - 자식 프로세스 대기를 위해 다양한 기능을 제공함.
		- idtype
		  P_PID: pid가 id와 일치하는 자식 프로세스를 기다림.
		  P_GID: gid가 id와 일치하는 자식 프로세스를 기다림.
		  P_ALL: 모든 자식 프로세스를 기다리며 id값은 무시된다.
		- options
		  WEXITED: id와 idtype으로 지정된 자식 프로세스가 종료되기를 기다림
		  WSTOPPED: 시그널을 받고 실행을 멈춘 자식 프로세스를 기다림
		  WNOHANG: 종료된 자식 프로세스가 없어도 블록되지 않고 바로 리턴됨
		  WNOWAIT: 좀비 상태에 있는 프로세스를 제거하지 않는다. 나중에 다시 이 프로세스를 다시 기다릴 수 있다.
*/
//		if((pid = waitid(
//				P_PID, 	// idtyp
//				child, 	// id
//				&info, 	// infop: 종료된 프로세스와 관련된 정보를 얻을 수 있다.
//				WEXITED	// options
//				)) == -1) // 성공하면 0을 리턴함.
//			errexit("waitid");
//
//		printf("pid = %d, uid = %d, code = %d, status = %d\n", info.si_pid, info.si_uid, info.si_code, info.si_status);
/*
		info.si_code: 자식 프로세스가 종료되거나, 시그널에 의해 종료되거나, 시그널에 의해 멈추거나, 시그널에 의해 재개되는 경우 각각 설정된다.
		- CLD_EXITED(=1)
		- CLD_KILLED(=2)
		- CLD_STOPPED
		- CLD_CONTINUED
*/

/*
		프로세스 종료 단계
		- atexit에 등록한 함수를 호출한다.
		- 열려있는 표준 입출력 스트림의 버퍼를 비운다.
		- tmpfile 함수를 통해 생성한 임시 파일을 삭제한다.
		- exit는 _exit 시스템 콜을 호출하여 프로세스 종료의 나머지 단계를 커널이 진행하게 한다.
		- 커널은 프로세스가 생성한 모든 리소스를 정리한다(할당 메모리, 열린 파일, 시스템 V 세마포어 등)
		- 커널은 프로세스를 종료하고 자식 프로세스가 종료됨을 부모 프로세스에 알린다.
*/
		exit(EXIT_SUCCESS);
	}

}

void system_sample()
{
	int status;
/*
	system 함수
	- 새로운 프로세스를 생성하고 종료를 기다리는 동작을 하나의 함수로 처리한다.(동기식 프로세스 생성)
	- 프로세스가 자식 프로세스를 생성한 다음 바로 종료를 기다리는 경우에 적합하다.
	- 간단한 유틸리티나 셸 스크립트를 실행할 목적으로 사용한다.
	- command 인자는 자동으로 /bin/sh -c 뒤에 연결된다. 즉, command 인자는 셸에 그대로 전달된다.
	- 호출에 성공하면 wait와 마찬가지로 status 값을 리턴한다.
	- 실행한 명령의 종료코드는 WEXITSTATUS로 얻을 수 있다.
	- 명령을 실행하는 동안 SIGCHLD는 블록되고, SIGINT, SIGQUIT는 무시된다.
	- exec 함수 계열의 'p'를 사용하는 것과 동일하게 셸을 직접 호출하므로 보안에 위험 요소가 있다.
*/
	if((status = system("vi test")) == -1)
		errexit("system");

	if(WIFEXITED(status))
		printf("status = %d\n", WEXITSTATUS(status));
}

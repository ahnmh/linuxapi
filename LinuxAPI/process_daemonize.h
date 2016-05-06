/*
 * process_daemonize.h
 *
 *  Created on: May 6, 2016
 *      Author: ahnmh
 */

#ifndef PROCESS_DAEMONIZE_H_
#define PROCESS_DAEMONIZE_H_

#include <syslog.h>

#define BD_NO_CHDIR				01 /* CWD를 root 디렉토리로 변경하지 않음 */
#define BD_NO_CLOSE_FILES		02 /* 모든 열린 파일을 종료하지 않음 */
#define BD_NO_REOPEN_STD_FDS	04 /* stdin, stdout, stderr를 /dev/null로 다시 열지 않음 */
#define BD_NO_UMASK0			010 /* umask(0)을 하지 않음 */

#define BD_MAX_CLOSE 			8192 /* 종료할 최대 파일 디스크립터 */


void process_daemonize();


#endif /* PROCESS_DAEMONIZE_H_ */

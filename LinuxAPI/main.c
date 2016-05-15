/*
 * main.c
 *
 *  Created on: Mar 31, 2016
 *      Author: root
 */

//#include "file_func.h"
//#include "process_env.h"
//#include "memory_alloc.h"
//#include "time_func.h"
//#include "user_func.h"
//#include "get_limit.h"
//#include "directory_link.h"
//#include "signal_func.h"
//#include "timer_func.h"
//#include "process_func.h"
//#include "process_daemonize.h"
//#include "syslog_test.h"
//#include "capability_test.h"
#include "dltest.h"

int main(int argc, char *argv[])
{
	int ret = 0;

/*
	ret = file_io();
	ret = file_fcntl();
	ret = file_scatter_gather();
	ret = file_ftruncate();
	ret = file_opentempfile();
	ret = file_pread_pwrite();
*/

//	ret = get_enviorn();
//	memory_alloc();
//	user_group();
//	time_related_func();
//	get_limit();
//	file_buffering();
//	file_mount();
//	file_stat();
//	directory_link();
//	file_event();
//	signal_basic();
//	signal_longjmp();
//	signal_restart();
//	signal_realtime();
//	signal_suspend();
//	signal_suspend_sync();
//	timer_func();
//	timer_sync_expire();
//	timer_sleep();
//	timer_clock_gettime();
//	timer_clock_nanosleep();
//	timer_advanced_signal();
//	timer_advanced_thread();
//	timer_advanced_fd();
//	process_func();
//	process_sync_by_signal();
//	process_exit();
//	process_wait();
//	process_waitpid();
//	process_waitid();
//	process_wait_by_signal();
//	process_exec();
//	process_clone();
//	process_setpgid();
//	process_priority();
//	process_scheduler();
//	process_resources();
//	process_daemonize();
//	syslog_test();
//	user_getuserid();
//	user_setuserid();
//	set_capability();
//	dltest();
	option_parsing(argc, argv);


	return 0;
}

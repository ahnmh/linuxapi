/*
 * ipc_func.h
 *
 *  Created on: May 22, 2016
 *      Author: ahnmh
 */

#ifndef IPC_FUNC_H_
#define IPC_FUNC_H_

void ipc_pipe(int argc, char *argv[]);
void ipc_pipe_stdinout();
void ipc_pipe_popen();

void ipc_fifo_server();

#endif /* IPC_FUNC_H_ */

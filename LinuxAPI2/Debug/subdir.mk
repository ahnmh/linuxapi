################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../alt_io_sample.c \
../file_lock.c \
../ipc_fifo_server.c \
../ipc_pipe.c \
../ipc_posix_mq.c \
../ipc_posix_sem.c \
../ipc_posix_shm.c \
../ipc_posix_unnamed_sem.c \
../ipc_systemv_msg_server.c \
../ipc_systemv_sem.c \
../ipc_systemv_shm_writer.c \
../main.c \
../mmap_func.c \
../pthread_cancellation.c \
../pthread_condition_variable.c \
../pthread_introduce.c \
../pthread_safe.c \
../pthread_signal.c \
../pthread_sync.c \
../socket_echo_client_example.c \
../socket_echo_server_example.c \
../socket_inet_domain.c \
../socket_unix_domain.c 

OBJS += \
./alt_io_sample.o \
./file_lock.o \
./ipc_fifo_server.o \
./ipc_pipe.o \
./ipc_posix_mq.o \
./ipc_posix_sem.o \
./ipc_posix_shm.o \
./ipc_posix_unnamed_sem.o \
./ipc_systemv_msg_server.o \
./ipc_systemv_sem.o \
./ipc_systemv_shm_writer.o \
./main.o \
./mmap_func.o \
./pthread_cancellation.o \
./pthread_condition_variable.o \
./pthread_introduce.o \
./pthread_safe.o \
./pthread_signal.o \
./pthread_sync.o \
./socket_echo_client_example.o \
./socket_echo_server_example.o \
./socket_inet_domain.o \
./socket_unix_domain.o 

C_DEPS += \
./alt_io_sample.d \
./file_lock.d \
./ipc_fifo_server.d \
./ipc_pipe.d \
./ipc_posix_mq.d \
./ipc_posix_sem.d \
./ipc_posix_shm.d \
./ipc_posix_unnamed_sem.d \
./ipc_systemv_msg_server.d \
./ipc_systemv_sem.d \
./ipc_systemv_shm_writer.d \
./main.d \
./mmap_func.d \
./pthread_cancellation.d \
./pthread_condition_variable.d \
./pthread_introduce.d \
./pthread_safe.d \
./pthread_signal.d \
./pthread_sync.d \
./socket_echo_client_example.d \
./socket_echo_server_example.d \
./socket_inet_domain.d \
./socket_unix_domain.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../error -O0 -g3 -Wall -c -pthread -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../capability_test.c \
../directory_link.c \
../dltest.c \
../file_buffering.c \
../file_event.c \
../file_io.c \
../file_mount.c \
../file_stat.c \
../get_limit.c \
../main.c \
../memory_alloc.c \
../option_parsing.c \
../process_clone.c \
../process_daemonize.c \
../process_env.c \
../process_exec.c \
../process_exit.c \
../process_func.c \
../process_priority.c \
../process_resources.c \
../process_setpgid.c \
../process_sync_by_signal.c \
../process_wait.c \
../process_wait_by_signal.c \
../signal_basic.c \
../signal_handler.c \
../signal_handler2.c \
../signal_realtime_handler.c \
../signal_suspend.c \
../signal_suspend_sync.c \
../syslog_test.c \
../time_func.c \
../timer_clock.c \
../timer_func.c \
../timer_sleep.c \
../timer_sync_expire.c \
../user_group.c \
../user_setuserid.c 

OBJS += \
./capability_test.o \
./directory_link.o \
./dltest.o \
./file_buffering.o \
./file_event.o \
./file_io.o \
./file_mount.o \
./file_stat.o \
./get_limit.o \
./main.o \
./memory_alloc.o \
./option_parsing.o \
./process_clone.o \
./process_daemonize.o \
./process_env.o \
./process_exec.o \
./process_exit.o \
./process_func.o \
./process_priority.o \
./process_resources.o \
./process_setpgid.o \
./process_sync_by_signal.o \
./process_wait.o \
./process_wait_by_signal.o \
./signal_basic.o \
./signal_handler.o \
./signal_handler2.o \
./signal_realtime_handler.o \
./signal_suspend.o \
./signal_suspend_sync.o \
./syslog_test.o \
./time_func.o \
./timer_clock.o \
./timer_func.o \
./timer_sleep.o \
./timer_sync_expire.o \
./user_group.o \
./user_setuserid.o 

C_DEPS += \
./capability_test.d \
./directory_link.d \
./dltest.d \
./file_buffering.d \
./file_event.d \
./file_io.d \
./file_mount.d \
./file_stat.d \
./get_limit.d \
./main.d \
./memory_alloc.d \
./option_parsing.d \
./process_clone.d \
./process_daemonize.d \
./process_env.d \
./process_exec.d \
./process_exit.d \
./process_func.d \
./process_priority.d \
./process_resources.d \
./process_setpgid.d \
./process_sync_by_signal.d \
./process_wait.d \
./process_wait_by_signal.d \
./signal_basic.d \
./signal_handler.d \
./signal_handler2.d \
./signal_realtime_handler.d \
./signal_suspend.d \
./signal_suspend_sync.d \
./syslog_test.d \
./time_func.d \
./timer_clock.d \
./timer_func.d \
./timer_sleep.d \
./timer_sync_expire.d \
./user_group.d \
./user_setuserid.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DBUFFER_SIZE=1000 -I../error -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



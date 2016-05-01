################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../directory_link.c \
../file_buffering.c \
../file_event.c \
../file_io.c \
../file_mount.c \
../file_stat.c \
../get_limit.c \
../main.c \
../memory_alloc.c \
../process_clone.c \
../process_env.c \
../process_exec.c \
../process_exit.c \
../process_func.c \
../process_sync_by_signal.c \
../process_wait.c \
../process_wait_by_signal.c \
../signal_basic.c \
../signal_handler.c \
../signal_handler2.c \
../signal_realtime_handler.c \
../signal_suspend.c \
../signal_suspend_sync.c \
../time_func.c \
../timer_clock.c \
../timer_func.c \
../timer_sleep.c \
../timer_sync_expire.c \
../user_group.c 

OBJS += \
./directory_link.o \
./file_buffering.o \
./file_event.o \
./file_io.o \
./file_mount.o \
./file_stat.o \
./get_limit.o \
./main.o \
./memory_alloc.o \
./process_clone.o \
./process_env.o \
./process_exec.o \
./process_exit.o \
./process_func.o \
./process_sync_by_signal.o \
./process_wait.o \
./process_wait_by_signal.o \
./signal_basic.o \
./signal_handler.o \
./signal_handler2.o \
./signal_realtime_handler.o \
./signal_suspend.o \
./signal_suspend_sync.o \
./time_func.o \
./timer_clock.o \
./timer_func.o \
./timer_sleep.o \
./timer_sync_expire.o \
./user_group.o 

C_DEPS += \
./directory_link.d \
./file_buffering.d \
./file_event.d \
./file_io.d \
./file_mount.d \
./file_stat.d \
./get_limit.d \
./main.d \
./memory_alloc.d \
./process_clone.d \
./process_env.d \
./process_exec.d \
./process_exit.d \
./process_func.d \
./process_sync_by_signal.d \
./process_wait.d \
./process_wait_by_signal.d \
./signal_basic.d \
./signal_handler.d \
./signal_handler2.d \
./signal_realtime_handler.d \
./signal_suspend.d \
./signal_suspend_sync.d \
./time_func.d \
./timer_clock.d \
./timer_func.d \
./timer_sleep.d \
./timer_sync_expire.d \
./user_group.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DBUFFER_SIZE=1000 -I../error -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



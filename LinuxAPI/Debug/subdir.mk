################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../directory_link.c \
../file_buffering.c \
../file_io.c \
../file_mount.c \
../file_stat.c \
../get_limit.c \
../main.c \
../memory_alloc.c \
../process_env.c \
../signal_basic.c \
../signal_handler.c \
../signal_handler2.c \
../signal_realtime_handler.c \
../signal_suspend.c \
../signal_suspend_sync.c \
../time_func.c \
../user_group.c 

OBJS += \
./directory_link.o \
./file_buffering.o \
./file_io.o \
./file_mount.o \
./file_stat.o \
./get_limit.o \
./main.o \
./memory_alloc.o \
./process_env.o \
./signal_basic.o \
./signal_handler.o \
./signal_handler2.o \
./signal_realtime_handler.o \
./signal_suspend.o \
./signal_suspend_sync.o \
./time_func.o \
./user_group.o 

C_DEPS += \
./directory_link.d \
./file_buffering.d \
./file_io.d \
./file_mount.d \
./file_stat.d \
./get_limit.d \
./main.d \
./memory_alloc.d \
./process_env.d \
./signal_basic.d \
./signal_handler.d \
./signal_handler2.d \
./signal_realtime_handler.d \
./signal_suspend.d \
./signal_suspend_sync.d \
./time_func.d \
./user_group.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DBUFFER_SIZE=1000 -I../error -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



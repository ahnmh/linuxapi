################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ipc_pipe.c \
../main.c \
../pthread_cancellation.c \
../pthread_condition_variable.c \
../pthread_introduce.c \
../pthread_safe.c \
../pthread_signal.c \
../pthread_sync.c 

OBJS += \
./ipc_pipe.o \
./main.o \
./pthread_cancellation.o \
./pthread_condition_variable.o \
./pthread_introduce.o \
./pthread_safe.o \
./pthread_signal.o \
./pthread_sync.o 

C_DEPS += \
./ipc_pipe.d \
./main.d \
./pthread_cancellation.d \
./pthread_condition_variable.d \
./pthread_introduce.d \
./pthread_safe.d \
./pthread_signal.d \
./pthread_sync.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../error -O0 -g3 -Wall -c -pthread -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



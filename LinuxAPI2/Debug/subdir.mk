################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../main.c \
../ptherad_introduce.c \
../pthread_condition_variable.c \
../pthread_sync.c 

OBJS += \
./main.o \
./ptherad_introduce.o \
./pthread_condition_variable.o \
./pthread_sync.o 

C_DEPS += \
./main.d \
./ptherad_introduce.d \
./pthread_condition_variable.d \
./pthread_sync.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../error -O0 -g3 -Wall -c -pthread -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



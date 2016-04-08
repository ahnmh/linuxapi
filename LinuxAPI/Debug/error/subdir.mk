################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../error/error_functions.c \
../error/get_num.c 

OBJS += \
./error/error_functions.o \
./error/get_num.o 

C_DEPS += \
./error/error_functions.d \
./error/get_num.d 


# Each subdirectory must supply rules for building sources it contributes
error/%.o: ../error/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DBUFFER_SIZE=1000 -I../error -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



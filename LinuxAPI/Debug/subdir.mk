################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../file_io.c \
../main.c \
../memory_alloc.c \
../process_env.c \
../time_func.c \
../user_group.c 

OBJS += \
./file_io.o \
./main.o \
./memory_alloc.o \
./process_env.o \
./time_func.o \
./user_group.o 

C_DEPS += \
./file_io.d \
./main.d \
./memory_alloc.d \
./process_env.d \
./time_func.d \
./user_group.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DBUFFER_SIZE=1000 -I../error -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



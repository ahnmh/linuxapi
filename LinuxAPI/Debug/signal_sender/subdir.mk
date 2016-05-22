################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../signal_sender/signal_sender.o 

C_SRCS += \
../signal_sender/signal_realtime_sender.c \
../signal_sender/signal_sender.c 

OBJS += \
./signal_sender/signal_realtime_sender.o \
./signal_sender/signal_sender.o 

C_DEPS += \
./signal_sender/signal_realtime_sender.d \
./signal_sender/signal_sender.d 


# Each subdirectory must supply rules for building sources it contributes
signal_sender/%.o: ../signal_sender/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DBUFFER_SIZE=1000 -I../error -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



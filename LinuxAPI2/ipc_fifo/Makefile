C_SRCS += \
		ipc_fifo_client.c \
		../error/error_functions.c \
		../error/get_num.c 
OBJS += \
		ipc_fifo_client.o \
		../error/error_functions.o \
		../error/get_num.o 
C_DEPS += \
		ipc_fifo_client.d \
		../error/error_functions.d \
		../error/get_num.d 

DEFINITIONS := -DDEBUG
INCLUDES := -I../ -I../error
LIBS :=
CFLAGS += -O0 -g3 -Wall -fmessage-length=0 -MMD -MP

EXECUTABLES := 

RM := rm -rf

../error/%.o: ../error/%.c
	@echo 'Building file: $<'
		@echo 'Invoking: GCC C Compiler'
			gcc -I../error -O0 -g3 -Wall -c -pthread -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
				@echo 'Finished building: $<'
					@echo ' '
%.o: %.c
		@echo 'Building file: $<'
		@echo 'Invoking: GCC C Compiler'
		gcc $(INCLUDES) $(DEFINITIONS) -c $(CFLAGS) -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
		@echo 'Finished building: $<'

all: ipc_fifo_client

ipc_fifo_client: $(OBJS)
		@echo 'Building target: $@'
		@echo 'Invoking: GCC C Linker'
		gcc -o $@ $^ $(LIBS)
		@echo 'Finished building target: $@'
		@echo ' '

clean:
		-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) ipc_fifo_client
		@echo ' '

.PHONY: all clean

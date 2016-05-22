C_SRCS += \
		ipc_fifo_server.c \

OBJS += \
		ipc_fifo_server.o \

C_DEPS += \
		ipc_fifo_server.d \

DEFINITIONS := -DDEBUG
INCLUDES := -I../
LIBS :=
CFLAGS += -O0 -g3 -Wall -fmessage-length=0 -MMD -MP

EXECUTABLES := 

RM := rm -rf

%.o: %.c
		@echo 'Building file: $<'
		@echo 'Invoking: GCC C Compiler'
		gcc $(INCLUDES) $(DEFINITIONS) -c $(CFLAGS) -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
		@echo 'Finished building: $<'

all: ipc_fifo_server

ipc_fifo_server: $(OBJS)
		@echo 'Building target: $@'
		@echo 'Invoking: GCC C Linker'
		gcc -o $@ $^ $(LIBS)
		@echo 'Finished building target: $@'
		@echo ' '

clean:
		-$(MAKE) -C ./src clean
		-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) ipc_fifo_server
		@echo ' '

.PHONY: all clean

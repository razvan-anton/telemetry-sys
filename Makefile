# Toolchain setup
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# Project configuration
TARGET  = build/app
MCU     = -mcpu=cortex-m3 -mthumb

# Include paths
INCLUDES = -Iinclude

# Compiler flags
CFLAGS  = $(MCU) $(INCLUDES) -Wall -Wextra -O0 -g3 \
          -ffunction-sections -fdata-sections -MMD -MP -Iinclude -DSTM32F103xB

# Linker flags
LDFLAGS = $(MCU) -Tstm32f103rb.ld -Wl,--gc-sections \
          --specs=nano.specs -Wl,-Map=build/app.map

# Sources
SRCS    = $(wildcard src/*.c) startup_stm32f103xb.s
OBJS    = $(patsubst %.c, build/%.o, $(patsubst %.s, build/%.o, $(SRCS)))
DEPS    = $(OBJS:.o=.d)

# Rules
all: $(TARGET).bin

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.s | build
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

build:
	mkdir -p build/src

clean:
	rm -rf build

-include $(DEPS)

.PHONY: all clean
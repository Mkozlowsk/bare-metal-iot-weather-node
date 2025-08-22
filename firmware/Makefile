# firmware/Makefile

# Toolchain definitions
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Target and sources
TARGET = main# Output file base name
SRCS = \
	core/main.c \
	lib/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.s \
    lib/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c
INCS = \
	lib/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Include/.\
	-Ilib/STM32CubeL4/Drivers/CMSIS/Include/.
LINKER = lib/STM32CubeL4/Projects/NUCLEO-L476RG/Templates/STM32CubeIDE/STM32L476RGTX_FLASH.ld

# Microcontroller and flags
MCU = cortex-m4
THUMB = -mthumb                 # Use Thumb instruction set
CFLAGS = -mcpu=$(MCU) $(THUMB) -Og -g -Wall -std=c99 -I$(INCS) -DSTM32L476xx# Compiler flags: CPU, Thumb, debug, warnings, C99
LDFLAGS = -T$(LINKER) -nostdlib -Wl,-Map=$(TARGET).map # Linker flags: script, no stdlib, map file

# Generate list of object files
OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

# Default target: build everything
all: $(TARGET).bin

# Rule to create .bin file from .elf
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $<

# Rule to create .elf file from object files
$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Generic rule to compile .c files to .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).map

# Flash the board using OpenOCD (odkomentuj później)
# flash: $(TARGET).bin
# 	openocd -f interface/stlink.cfg -f target/stm32l4x.cfg -c "program $(TARGET).bin verify reset exit 0x08000000"

.PHONY: all clean #flash
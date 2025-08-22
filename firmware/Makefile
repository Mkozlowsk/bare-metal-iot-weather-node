# firmware/Makefile

# Toolchain definitions
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Target and sources
TARGET = main                   # Output file base name
SRCS = \
	core/main.c \
    #core/system_stm32l4xx.c  # List of source files (.c)
# Add your .c files above

# Microcontroller and flags
MCU = cortex-m4
THUMB = -mthumb                 # Use Thumb instruction set
CFLAGS = -mcpu=$(MCU) $(THUMB) -Og -g -Wall -std=c99 # Compiler flags: CPU, Thumb, debug, warnings, C99
LDFLAGS = -Tlinker_script.ld -nostdlib -Wl,-Map=$(TARGET).map # Linker flags: script, no stdlib, map file

# Generate list of object files
OBJS = $(SRCS:.c=.o)

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

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).map

# Flash the board using OpenOCD (odkomentuj później)
# flash: $(TARGET).bin
# 	openocd -f interface/stlink.cfg -f target/stm32l4x.cfg -c "program $(TARGET).bin verify reset exit 0x08000000"

#.PHONY: all clean flash
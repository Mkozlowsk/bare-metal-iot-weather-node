# firmware/Makefile

# Toolchain definitions
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Target and build directory
TARGET = main# Output file base name
BUILD_DIR = build

# Source files
SRCS = \
	core/main.c \
    lib/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c

# Startup assembly file
ASM_SRC = lib/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.s

# Include directories
CMSIS_DEVICE_INC = lib/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Include
CMSIS_INC = lib/STM32CubeL4/Drivers/CMSIS/Include

# Linker script
LINKER = lib/STM32CubeL4/Projects/NUCLEO-L476RG/Templates/STM32CubeIDE/STM32L476RGTX_FLASH.ld

# Microcontroller and flags
MCU = cortex-m4
MCU_MODEL = STM32L476xx
THUMB = -mthumb                 # Use Thumb instruction set
CFLAGS = -mcpu=$(MCU) $(THUMB) -Og -g -Wall -std=c99 -I$(CMSIS_DEVICE_INC) -I$(CMSIS_INC) -D$(MCU_MODEL)# Compiler flags: CPU, Thumb, debug, warnings, C99
LDFLAGS = -T$(LINKER) -nostdlib -Wl,-Map=$(BUILD_DIR)/$(TARGET).map # Linker flags: script, no stdlib, map file

# Generate list of object files in build directory
OBJS = $(SRCS:.c=.o)
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))
OBJS += $(BUILD_DIR)/startup_stm32l476xx.o

OBJS := $(OBJS:.s=.o)

# Default target: build everything
all: $(BUILD_DIR)/$(TARGET).bin

# Rule to create .bin file from .elf
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $<

# Rule to create .elf file from object files
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@


# Rule to compile .c files to .o object files in build directory
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to assemble .s files to .o object files in build directory
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Special rule for the startup file (it's in a different location)
$(BUILD_DIR)/startup_stm32l476xx.o: $(ASM_SRC)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Flash the board using OpenOCD
flash: $(BUILD_DIR)/$(TARGET).bin
	openocd -f interface/stlink.cfg -f target/stm32l4x.cfg -c "program $< verify reset exit 0x08000000"

.PHONY: all clean flash